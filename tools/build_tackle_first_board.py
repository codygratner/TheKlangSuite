#!/usr/bin/env python3
"""
tools/build_tackle_first_board.py — Cosmic Codex Pilot Board Builder
====================================================================
Generates the interactive "What Would You Like to Tackle First?" decision
and priority board for N'kai mobile and desktop sidecar panels.
Universal UTF-8 compliance guaranteed.
"""

import argparse
from datetime import datetime
import os
from pathlib import Path
import sys

# Universal UTF-8 Console Streams
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.models import (
    BoardMetadata,
    BoardSchema,
    EcosystemPillar,
    MailItem,
    PriorityLevel,
    TriageStatus,
)
from tools.board_renderer import render_board
from tools.board_registry import load_registry, save_registry

DEFAULT_OUTPUT_PATH = REPO_ROOT / ".agents" / "sidecar" / "tackle_first_board.html"


def create_tackle_first_cards() -> list[MailItem]:
    """Builds the 6 prime initiative decision cards."""
    now_str = datetime.now().strftime("%Y-%m-%d")

    cards = [
        MailItem(
            id="tackle_01_gdrive",
            title="🧹 Google Drive & Cloud Ingestion Cleanup",
            date=now_str,
            priority=PriorityLevel.P1,
            pillars=[EcosystemPillar.NK, EcosystemPillar.TKR],
            status=TriageStatus.PENDING,
            scope="TheKlangVault / Google Drive",
            raw_body=(
                "### Objective & Scope\n"
                "De-duplicate and ingest legacy Google Drive folders, voice memos, and scattered notes into canonical folders inside `TheKlangVault`.\n\n"
                "### Proposed Action Route\n"
                "1. Connect via Google Takeout / Drive export or cloud sync script.\n"
                "2. Run de-duplication hash filter against existing `TheKlangVault/` assets.\n"
                "3. Ingest unindexed voice memos into `C:\\_\\Vault\\Inbox\\` for rapid mobile triage.\n"
                "4. Move completed archives to historical provenance cold storage."
            ),
            suggested_action="approve",
            split_group="cloud_ingestion",
        ),
        MailItem(
            id="tackle_02_inbox_zero",
            title="📥 Morning Inbox Zero Sprint",
            date=now_str,
            priority=PriorityLevel.P1,
            pillars=[EcosystemPillar.NK],
            status=TriageStatus.PENDING,
            scope="TheKlangVault/Inbox",
            raw_body=(
                "### Objective & Scope\n"
                "Triage remaining unindexed voice memos and raw thoughts directly from the couch using `N'kai (Home)`.\n\n"
                "### Proposed Action Route\n"
                "1. Open Morning Mail Inbox on your iPhone XR (`http://192.168.1.3:3030`).\n"
                "2. 1-tap Approve, Backlog, or Research on each docket card.\n"
                "3. Automatically append approved tickets to `docs/BACKLOG.md`."
            ),
            suggested_action="approve",
            split_group="daily_intake",
        ),
        MailItem(
            id="tackle_03_simplenote",
            title="📝 Simplenote Import & Line-Ending Normalization",
            date=now_str,
            priority=PriorityLevel.P2,
            pillars=[EcosystemPillar.TKS],
            status=TriageStatus.PENDING,
            scope="TheKlangVault/Inbox/Simplenote",
            raw_body=(
                "### Objective & Scope\n"
                "Ingest the pending dispatch packet from the Inter-Project Data Bus (`2026-10-12_cosmos_to_tks_simplenote_import_line_endings`).\n\n"
                "### Proposed Action Route\n"
                "1. Run `python tools/sync_simplenote.py` to convert CRLF to clean UTF-8 LF.\n"
                "2. Acknowledge and resolve bus dispatch.\n"
                "3. Verify zero Windows encoding warnings."
            ),
            suggested_action="backlog",
            split_group="inter_project_bus",
        ),
        MailItem(
            id="tackle_04_vault_tags",
            title="🏷️ Vault Tag Normalization & Frontmatter Invariant",
            date=now_str,
            priority=PriorityLevel.P2,
            pillars=[EcosystemPillar.TKR],
            status=TriageStatus.PENDING,
            scope="TheKlangVault/Docs",
            raw_body=(
                "### Objective & Scope\n"
                "Acknowledge pending bus dispatch `2026-10-12_cosmos_to_tks_vault_tag_normalization` and enforce the Universal Frontmatter Invariant.\n\n"
                "### Proposed Action Route\n"
                "1. Run `python tools/lint_frontmatter.py` across `C:\\_\\Vault`.\n"
                "2. Normalize `#the-vault` and `#new-klang-city` tags into clean YAML frontmatter arrays.\n"
                "3. Ensure 100% Obsidian tag-wrangler parity."
            ),
            suggested_action="backlog",
            split_group="vault_hygiene",
        ),
        MailItem(
            id="tackle_05_forge_cyborg",
            title="🎲 The Forge: Launch Solo RPG 'STARVE' Cyberpunk Docket",
            date=now_str,
            priority=PriorityLevel.P2,
            pillars=[EcosystemPillar.TKR],
            status=TriageStatus.PENDING,
            scope="The Forge/Solo TTRPG",
            raw_body=(
                "### Objective & Scope\n"
                "Bring The Forge into active play! Manifest an interactive mission & oracle generator for the CY_BORG solo campaign.\n\n"
                "### Proposed Action Route\n"
                "1. Generate new job using `ProcGenJobs` and `! Generators/Generate Mission.md`.\n"
                "2. Track thread state and debt pay-off in `Threads/Paying Off Debt.md`.\n"
                "3. Roll dice and log scene moods directly in Obsidian using Cosmos Monochrome Icons."
            ),
            suggested_action="research",
            split_group="creative_worldbuilding",
        ),
        MailItem(
            id="tackle_06_dsp_saturator",
            title="🎹 The Klang Farmer: Flight 1 Non-Linear Saturation Core",
            date=now_str,
            priority=PriorityLevel.P1,
            pillars=[EcosystemPillar.TKS],
            status=TriageStatus.PENDING,
            scope="TheKlangSuite/Source/dsp",
            raw_body=(
                "### Objective & Scope\n"
                "Kick off Phase 1 of The Klang Farmer: C++20 non-linear tape saturation DSP and JUCE 9.0.3 audio engine.\n\n"
                "### Proposed Action Route\n"
                "1. Formulate exact differential equation equations in `PLAN.md`.\n"
                "2. Execute Targeted Opus DSP Swap Protocol for real-time safety.\n"
                "3. Audit zero allocations, zero locks, and FastMath SIMD compliance via `audiothread-guard`."
            ),
            suggested_action="approve",
            split_group="factory_floor",
        ),
    ]
    return cards


def build_tackle_first_board(out_path: Optional[Path] = None, skip_registry: bool = False) -> Path:
    """Builds and mirrors the interactive 'What Would You Like to Tackle First?' board."""
    target_path = out_path or DEFAULT_OUTPUT_PATH
    cards = create_tackle_first_cards()

    schema = BoardSchema(
        board_id="tackle_first",
        title="🌌 What Would You Like to Tackle First?",
        source_path="The Cosmic Codex // Nyarlathotep Priority Radar",
        cards=cards,
    )

    print(f"[*] Rendering Cosmic Codex Decision Board:")
    print(f"    Target:  {target_path}")
    print(f"    Cards:   {len(cards)} candidate initiatives")

    rendered = render_board(schema, target_path)

    # Mirror to nkai directories
    nkai_sidecar = Path(r"c:\Dev\nkai\sidecars\panel\tackle_first_board.html")
    if nkai_sidecar.parent.exists():
        try:
            nkai_sidecar.write_text(rendered.read_text(encoding="utf-8"), encoding="utf-8")
            print(f"    [+] Mirrored to: {nkai_sidecar}")
        except Exception as e:
            print(f"    [-] Mirror warning: {e}")

    plugin_panel = Path(r"C:\Users\codyg\.gemini\config\plugins\nkai\sidecars\panel\tackle_first_board.html")
    if plugin_panel.parent.exists():
        try:
            plugin_panel.write_text(rendered.read_text(encoding="utf-8"), encoding="utf-8")
            print(f"    [+] Mirrored to: {plugin_panel}")
        except Exception as e:
            print(f"    [-] Mirror warning: {e}")

    # Register in BoardRegistry
    if not skip_registry:
        try:
            reg = load_registry()
            existing = reg.get_board("tackle_first")
            if existing:
                existing.title = "🌌 What Would You Like to Tackle First?"
                existing.description = "Cosmic Codex Priority Radar: Interactive triage for Google Drive, Vault Ingestion & Active Flights"
                existing.card_count = len(cards)
                existing.category = "active"
                existing.board_type = "custom"
                existing.file_path = ".agents/sidecar/tackle_first_board.html"
                existing.updated_at = datetime.now().isoformat()
            else:
                reg.boards.append(
                    BoardMetadata(
                        id="tackle_first",
                        title="🌌 What Would You Like to Tackle First?",
                        description="Cosmic Codex Priority Radar: Interactive triage for Google Drive, Vault Ingestion & Active Flights",
                        category="active",
                        board_type="custom",
                        file_path=".agents/sidecar/tackle_first_board.html",
                        environment="PRODUCTION",
                        card_count=len(cards),
                        updated_at=datetime.now().isoformat(),
                    )
                )
            save_registry(reg)
            print("    [+] Registered 'tackle_first' in BoardRegistry manifest")
        except Exception as e:
            print(f"    [-] Registry warning: {e}")

    print("✅ Cosmic Codex Board build complete!")
    return target_path


def main():
    parser = argparse.ArgumentParser(description="Cosmic Codex Board Builder")
    parser.add_argument("--out", type=str, default=str(DEFAULT_OUTPUT_PATH), help="Target output HTML path")
    parser.add_argument("--no-registry", action="store_true", help="Skip updating registry.json")
    args = parser.parse_args()

    build_tackle_first_board(out_path=Path(args.out), skip_registry=args.no_registry)


if __name__ == "__main__":
    main()
