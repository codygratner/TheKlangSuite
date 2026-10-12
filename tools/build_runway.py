#!/usr/bin/env python3
"""
tools/build_runway.py — Ecosystem Delivery Runway Generator (Flight 20 / Act III)
================================================================================
Parses docs/BACKLOG.md Act III flights and ecosystem milestones, building a
declarative BoardSchema for runway_board.html.
Universal UTF-8 compliance guaranteed.
"""

import argparse
from datetime import datetime
import html
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from typing import Any, Dict, List, Optional, Tuple

# Universal UTF-8 Environment Invariant (Flight 18)
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

try:
    from tools.models import (
        BoardMetadata,
        BoardRegistry,
        BoardSchema,
        EcosystemPillar,
        MailItem,
        PriorityLevel,
        TriageStatus,
        infer_pillars,
    )
except ImportError:
    try:
        from models import (
            BoardMetadata,
            BoardRegistry,
            BoardSchema,
            EcosystemPillar,
            MailItem,
            PriorityLevel,
            TriageStatus,
            infer_pillars,
        )
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from models import (
            BoardMetadata,
            BoardRegistry,
            BoardSchema,
            EcosystemPillar,
            MailItem,
            PriorityLevel,
            TriageStatus,
            infer_pillars,
        )

try:
    from tools.board_renderer import render_board
except ImportError:
    try:
        from board_renderer import render_board
    except ImportError:
        _script_dir = Path(__file__).resolve().parent
        if str(_script_dir) not in sys.path:
            sys.path.insert(0, str(_script_dir))
        from board_renderer import render_board

try:
    from tools.board_registry import load_registry, save_registry
except ImportError:
    try:
        from board_registry import load_registry, save_registry
    except ImportError:
        pass


DEFAULT_BACKLOG_PATH = REPO_ROOT / "docs" / "BACKLOG.md"
DEFAULT_OUTPUT_PATH = REPO_ROOT / ".agents" / "sidecar" / "runway_board.html"


def get_git_tags() -> List[str]:
    """Retrieves list of git tags in repository."""
    try:
        res = subprocess.run(
            ["git", "tag", "-l"],
            cwd=str(REPO_ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace"
        )
        if res.returncode == 0:
            return [t.strip() for t in res.stdout.splitlines() if t.strip()]
    except Exception:
        pass
    return []


def parse_act3_flights(backlog_text: str) -> List[MailItem]:
    """Parses Act III flights and cards from BACKLOG.md markdown text."""
    cards: List[MailItem] = []
    
    # 1. Locate Act III Section (Section 15)
    sec15_match = re.search(r"### 15\.\s+N'kai Sidecar Architecture Roadmap \(The Rejiggered Runway — Act III\)(.*?)(?=### 16\.|\Z)", backlog_text, re.DOTALL)
    if not sec15_match:
        return cards
    
    act3_content = sec15_match.group(1)
    
    # Also collect full detailed flight sections (### 16. Flight 2, ### 17. Flight 3, etc.)
    detail_sections: Dict[str, str] = {}
    detail_matches = re.findall(r"### (\d+)\.\s+(Flight \d+[^:\n]*):?\s*(.*?)(?=\n### |\Z)", backlog_text, re.DOTALL)
    for num, title_prefix, content in detail_matches:
        f_match = re.search(r"Flight (\d+)", title_prefix, re.IGNORECASE)
        if f_match:
            detail_sections[f"flight_{f_match.group(1)}"] = content.strip()

    # Parse each checklist bullet item in Act III
    bullet_pattern = re.compile(
        r"^\s*-\s+\[([ xX])\]\s+\*\*(Flight\s+[\d\+]+|Card\s+`[^`]+`|Spike:[^*]+|\b[^*]+?\b):?\s*(.*?)\*\*\s*—\s*([^\n\r]+)",
        re.MULTILINE
    )

    now_date = datetime.now().strftime("%Y-%m-%d")

    for idx, match in enumerate(bullet_pattern.finditer(act3_content), 1):
        is_checked = (match.group(1).lower() == 'x')
        raw_prefix = match.group(2).strip()
        raw_title = match.group(3).strip()
        raw_desc = match.group(4).strip()

        full_title = f"{raw_prefix}: {raw_title}".strip(": ")
        
        # ID generation
        f_num = re.search(r"Flight\s+([\d\+]+)", raw_prefix, re.IGNORECASE)
        card_id = re.search(r"`([^`]+)`", raw_prefix)
        if f_num:
            item_id = f"flight_{f_num.group(1).replace('+', '_')}"
        elif card_id:
            item_id = card_id.group(1).replace("-", "_")
        else:
            item_id = f"runway_item_{idx}"

        # Status determination
        status = TriageStatus.BACKLOG
        desc_upper = raw_desc.upper()
        if "COMPLETED" in desc_upper or "RESOLVED" in desc_upper or is_checked:
            status = TriageStatus.COMPLETED
        elif "IN PROGRESS" in desc_upper or "ACTIVE" in desc_upper or "APPROVED" in desc_upper:
            status = TriageStatus.APPROVED
        elif "GRAVEYARD" in desc_upper or "REJECTED" in desc_upper:
            status = TriageStatus.GRAVEYARD
        elif "RESEARCH" in desc_upper or "PCDA" in desc_upper:
            status = TriageStatus.RESEARCH

        # Priority determination
        priority = PriorityLevel.P1
        if any(p in (full_title + " " + raw_desc).upper() for p in ["(P0", "P0 —", "TOP PRIORITY", "TOP ACTIVE"]):
            priority = PriorityLevel.P0
        elif "(P2" in full_title or "P2" in raw_desc:
            priority = PriorityLevel.P2
        elif "(P3" in full_title or "P3" in raw_desc:
            priority = PriorityLevel.P3

        # Pillar determination
        pillars = infer_pillars(full_title, body=raw_desc)
        # N'kai flights default to NK if not set
        if "NK" not in pillars and ("Flight" in raw_prefix or "rj-" in raw_prefix or "sidecar" in raw_desc.lower()):
            pillars.insert(0, "NK")

        # Excerpt
        clean_desc = re.sub(r"^[✅⏳🔨🪦\s*—]+", "", raw_desc).strip()
        clean_desc = re.sub(r"^(COMPLETED|QUEUED|ACTIVE|IN PROGRESS|APPROVED|RESOLVED|GRAVEYARD):?\s*", "", clean_desc, flags=re.IGNORECASE).strip()
        excerpt = clean_desc[:260] + ("..." if len(clean_desc) > 260 else "")

        # Body (merge detailed section if exists)
        full_body = clean_desc
        if item_id in detail_sections:
            full_body = f"{clean_desc}\n\n### Detailed Specification\n{detail_sections[item_id]}"

        cards.append(
            MailItem(
                id=item_id,
                title=full_title,
                source_note="docs/BACKLOG.md",
                date=now_date,
                excerpt=excerpt or full_title,
                body=full_body,
                priority=priority,
                tags=["act3", "runway", "flight"],
                pillars=pillars,
                status=status,
                model_tier="tier2",
                token_impact=15000,
                verdict_notes=raw_desc[:80] if len(raw_desc) > 0 else None,
            )
        )

    return cards


def build_ecosystem_milestones() -> List[MailItem]:
    """Constructs cards for active and historical ecosystem milestones."""
    now_date = datetime.now().strftime("%Y-%m-%d")
    milestones = [
        # TKS Milestones
        MailItem(
            id="ms_tks_v030",
            title="Milestone: The Klang Farmer v0.3.0 (Architecture Update)",
            source_note="docs/BACKLOG.md",
            date="2026-10-06",
            excerpt="C++ GUI Test Harness, The Klang Editor (TKE), macOS .pkg pipeline, Gatekeeper quarantine stripper, and v0.2.0 parity audit.",
            body="Comprehensive architecture update: single-binary GUI testing harness (`gui_tests`), The Klang Editor with APVTS parameter sync, zero-cost macOS installer, and 100% legacy parity.",
            priority=PriorityLevel.P0,
            tags=["milestone", "architecture", "v0.3.0"],
            pillars=["TKS"],
            status=TriageStatus.COMPLETED,
            model_tier="tier1",
            token_impact=45000,
            verdict_notes="Shipped in commit 4001296",
        ),
        MailItem(
            id="ms_tks_v031",
            title="Milestone: The Klang Farmer v0.3.1 (Editor Quality & Data Schema)",
            source_note="docs/BACKLOG.md",
            date="2026-10-08",
            excerpt="Standalone Tree UX, Limiter Callout, Two-Line Status Bar, pluginval Runner integration, and snap points schema.",
            body="Hardened editor data schemas, standalone audio preview engine, dynamic popover limiter callouts, and automated pluginval validation runner.",
            priority=PriorityLevel.P1,
            tags=["milestone", "editor", "v0.3.1"],
            pillars=["TKS"],
            status=TriageStatus.COMPLETED,
            model_tier="tier2",
            token_impact=30000,
            verdict_notes="Shipped & verified",
        ),
        MailItem(
            id="ms_tks_v032",
            title="Milestone: The Klang Farmer v0.3.2 (Agent Infrastructure & Logging)",
            source_note="docs/BACKLOG.md",
            date="2026-10-10",
            excerpt="Guardrails audit, unified filterable master tree, dedicated text schema, and developer logging (TKS_LOG).",
            body="Complete agent infrastructure overhaul: strict real-time audiothread guardrails, master parameter tree reflection, strings.json text schema, and atomic dev logging.",
            priority=PriorityLevel.P0,
            tags=["milestone", "tooling", "v0.3.2"],
            pillars=["TKS", "AGY"],
            status=TriageStatus.COMPLETED,
            model_tier="tier2",
            token_impact=25000,
            verdict_notes="Shipped in 0.4.0-dev",
        ),
        MailItem(
            id="ms_tks_v040",
            title="Milestone: The Klang Farmer v0.4.0 (Neo-Slate Vector UI & Theming)",
            source_note="docs/BACKLOG.md",
            date="2026-10-12",
            excerpt="Complete clean-slate UX overhaul, Neo-Slate Vector UI (Kilohearts/Vital aesthetic), 4-Controls-Per-Card, Header Nav & Stereo Scope, Popover Callouts, 7-Theme Engine.",
            body="Active flagship milestone: Complete interface rewrite to Neo-Slate vector styling, 4 controls per card, integrated stereo scope, popovers, dynamic themes, and desktop sanity testing.",
            priority=PriorityLevel.P0,
            tags=["milestone", "ui", "neo-slate", "v0.4.0"],
            pillars=["TKS"],
            status=TriageStatus.APPROVED,
            model_tier="tier2",
            token_impact=50000,
            verdict_notes="Active Engineering",
        ),
        MailItem(
            id="ms_tks_v045",
            title="Milestone: The Klang Planter v0.4.5 (Planter Refresh & Parity)",
            source_note="docs/BACKLOG.md",
            date=now_date,
            excerpt="Rebuilding The Klang Planter on TKF's Neo-Slate Foundation — 4-Controls-Per-Card, Header Nav, Vector Dice Buttons, Popover Callouts, Universal Theming.",
            body="Planned refresh bringing full architectural parity to The Klang Planter: Neo-Slate styling, vector dice randomizer, popover callouts, and 7 themes.",
            priority=PriorityLevel.P1,
            tags=["milestone", "planter", "v0.4.5"],
            pillars=["TKS"],
            status=TriageStatus.BACKLOG,
            model_tier="tier2",
            token_impact=35000,
            verdict_notes="Planned after v0.4.0 release",
        ),
        MailItem(
            id="ms_tks_v050",
            title="Milestone: The Klang Farmer v0.5.0 (Sound & Chaos + Mega FX Modules)",
            source_note="docs/BACKLOG.md",
            date=now_date,
            excerpt="Unified Mega FX Modules (Drive w/ Tanh/Wavefolder/Faturator, Thickener w/ Chorus/Ensemble), FX Picker, Alpha Juno oscillators, Mixer Cross-mod.",
            body="Massive DSP expansion milestone: Non-linear drive models, Alpha Juno PWM oscillators, ER-1 Mk2 cross-modulation, preset morphing, and dual sample players.",
            priority=PriorityLevel.P0,
            tags=["milestone", "dsp", "v0.5.0"],
            pillars=["TKS", "TKR"],
            status=TriageStatus.BACKLOG,
            model_tier="tier1",
            token_impact=75000,
            verdict_notes="Planned next major feature",
        ),

        # N'kai Milestones
        MailItem(
            id="ms_nk_v020",
            title="Milestone: N'kai v0.2.0 (UI Extension & Native Bridge)",
            source_note="docs/BACKLOG.md",
            date="2026-10-10",
            excerpt="Antigravity UI extension integration, native IPC bridge (window.sidecar.agent.sendMessage), and morning mail triage lab.",
            body="First official release of N'kai as an Antigravity UI extension with native agent messaging, two-tier header, allowance meter, and procedural audio haptics.",
            priority=PriorityLevel.P0,
            tags=["milestone", "sidecar", "v0.2.0"],
            pillars=["NK"],
            status=TriageStatus.COMPLETED,
            model_tier="tier2",
            token_impact=20000,
            verdict_notes="Codified in registry.json",
        ),
        MailItem(
            id="ms_nk_v030",
            title="Milestone: N'kai v0.3.0 (Mobile Companion & Touch Ergonomics)",
            source_note="docs/BACKLOG.md",
            date="2026-10-11",
            excerpt="Mobile companion board with LAN QR code, thumb navigation bar with 44px tap targets, haptic feedback, and viewport-fit layout.",
            body="Mobile ergonomic expansion enabling couch triage via mobile phone browser: sticky thumb bar, bottom sheet drawers, and zero-latency local websocket/http sync.",
            priority=PriorityLevel.P1,
            tags=["milestone", "mobile", "v0.3.0"],
            pillars=["NK"],
            status=TriageStatus.COMPLETED,
            model_tier="tier2",
            token_impact=25000,
            verdict_notes="Shipped in tools/nkai_serve.py",
        ),
        MailItem(
            id="ms_nk_v035",
            title="Milestone: N'kai v0.3.5 (Ecosystem Hardening & 3-Surface System)",
            source_note="docs/BACKLOG.md",
            date="2026-10-12",
            excerpt="The 3-Surface System (Scratchpad, Mailbag, Runway), in-board quick capture, Flight Runway board, and lab conclusion lifecycle.",
            body="Active flight milestone: Codifies the 3-surface workflow, in-board quick note capture into daily catchall, automatic runway board generation, and triumphant session lifecycle.",
            priority=PriorityLevel.P0,
            tags=["milestone", "3surfaces", "v0.3.5"],
            pillars=["NK", "AGY"],
            status=TriageStatus.APPROVED,
            model_tier="tier2",
            token_impact=30000,
            verdict_notes="Executing Flight 20",
        ),

        # ToadTracker Milestone
        MailItem(
            id="ms_tt_v010",
            title="Milestone: ToadTracker v0.1.0 (Embedded Tracker Engine & PoC)",
            source_note="docs/BACKLOG.md",
            date=now_date,
            excerpt="Core migration of Dirtywave M8 tracker engine, pattern memory layouts, gamepad input debouncing, and ESP32-P4 hardware bridge.",
            body="Active embedded hardware tracker milestone: Headless tracker playback engine, pattern sequencer, M8 display protocol emulation, and TBD-16 bridge.",
            priority=PriorityLevel.P0,
            tags=["milestone", "tracker", "embedded", "v0.1.0"],
            pillars=["TT", "TBD-16"],
            status=TriageStatus.APPROVED,
            model_tier="tier1",
            token_impact=60000,
            verdict_notes="Active PoC in c:/Dev/ToadTracker",
        ),
    ]

    return milestones


def build_runway(
    backlog_path: Optional[Path] = None,
    output_path: Optional[Path] = None,
    register_board: bool = True
) -> Path:
    """Builds and renders the Ecosystem Delivery Runway board."""
    b_path = backlog_path or DEFAULT_BACKLOG_PATH
    out_path = output_path or DEFAULT_OUTPUT_PATH

    print(f"[*] Building Ecosystem Delivery Runway Board:")
    print(f"    Source Backlog: {b_path}")
    print(f"    Target HTML:    {out_path}")

    if not b_path.exists():
        raise FileNotFoundError(f"docs/BACKLOG.md not found at: {b_path}")

    backlog_text = b_path.read_text(encoding="utf-8")
    
    # 1. Parse Act III flights
    flight_cards = parse_act3_flights(backlog_text)
    print(f"    [+] Parsed {len(flight_cards)} flight cards from Act III")

    # 2. Build core ecosystem milestones
    milestone_cards = build_ecosystem_milestones()
    print(f"    [+] Created {len(milestone_cards)} ecosystem milestone cards")

    # Combine cards: Active milestones & flights first, then planned, then completed
    all_cards = milestone_cards + flight_cards

    # Build BoardSchema
    schema = BoardSchema(
        schema_version="1.0.0",
        board_id="runway",
        title="Ecosystem Delivery Runway",
        subtitle="Cross-Project Strategic Delivery Horizon (Act III)",
        environment="PRODUCTION",
        board_type="evergreen",
        allowance_budget=350000,
        cards=all_cards,
    )

    # 3. Render HTML board
    render_board(schema, out_path=out_path)
    print(f"    [+] Rendered runway board HTML: {out_path} ({out_path.stat().st_size:,} bytes)")

    # 4. Canonical Mirroring
    canonical_mirrors = [
        Path(r"c:\Dev\nkai\sidecars\panel\runway_board.html"),
        Path(r"c:\Dev\nkai\plugin\sidecars\panel\runway_board.html"),
    ]
    for mirror in canonical_mirrors:
        try:
            mirror.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(out_path, mirror)
            print(f"    [+] Mirrored runway board to: {mirror}")
        except Exception as e:
            print(f"    [-] Mirror warning for {mirror}: {e}")

    # 5. Register in BoardRegistry
    if register_board:
        try:
            reg = load_registry()
            # Check if runway already registered
            existing = reg.get_board("runway")
            if existing:
                existing.title = "Ecosystem Delivery Runway"
                existing.description = "Cross-project strategic delivery horizon mapped from docs/BACKLOG.md Act III"
                existing.card_count = len(all_cards)
                existing.category = "active"
                existing.board_type = "evergreen"
                existing.file_path = ".agents/sidecar/runway_board.html"
                existing.updated_at = datetime.now().isoformat()
            else:
                reg.boards.append(
                    BoardMetadata(
                        id="runway",
                        title="Ecosystem Delivery Runway",
                        description="Cross-project strategic delivery horizon mapped from docs/BACKLOG.md Act III",
                        category="active",
                        board_type="evergreen",
                        file_path=".agents/sidecar/runway_board.html",
                        environment="PRODUCTION",
                        card_count=len(all_cards),
                        updated_at=datetime.now().isoformat(),
                    )
                )
            save_registry(reg)
            print("    [+] Registered 'runway' board in BoardRegistry manifest")
        except Exception as e:
            print(f"    [-] Registry warning: {e}")

    print("✅ Runway board build complete!")
    return out_path


def main():
    parser = argparse.ArgumentParser(description="Ecosystem Delivery Runway Board Builder (Flight 20)")
    parser.add_argument("--backlog", type=str, default=str(DEFAULT_BACKLOG_PATH), help="Path to docs/BACKLOG.md")
    parser.add_argument("--out", type=str, default=str(DEFAULT_OUTPUT_PATH), help="Target output HTML path")
    parser.add_argument("--no-registry", action="store_true", help="Skip updating registry.json")

    args = parser.parse_args()
    build_runway(
        backlog_path=Path(args.backlog),
        output_path=Path(args.out),
        register_board=not args.no_registry
    )


if __name__ == "__main__":
    main()
