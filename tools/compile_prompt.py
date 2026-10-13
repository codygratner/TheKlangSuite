#!/usr/bin/env python3
"""
tools/compile_prompt.py — N'kai to High Reasoning Prompt Compiler (Flight 2)
=============================================================================
Compiles raw research cards, user notes, dialectical theses, and adversarial
friction metrics into a dense, high-signal master system prompt payload for
flagship high-reasoning models (Claude 3.7 Sonnet Thinking, Gemini 3.1 Pro High).

Supports:
- OS system clipboard injection (Win32 ctypes, macOS pbcopy, Linux xclip/wl-copy)
- Direct vault handoff drops (`TheKlangVault/Dispatches/reasoning/`)
- Terminal shell stdout piping
- Integration with N'kai sidecar action chips and mobile walkie-talkie
"""

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from typing import Any, Dict, Optional, Tuple

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from tools.models import (
    AntithesisBlock,
    PromptHandoffPayload,
    SynthesisTargetConfig,
    ThesisBlock,
    detect_lens_profile,
    load_lens_profiles,
)


def copy_to_clipboard(text: str) -> bool:
    """
    Copies text to the system clipboard across Windows, macOS, and Linux.
    Returns True on success, False on error.
    """
    try:
        if sys.platform in ["win32", "cygwin"]:
            # Try Win32 API via ctypes first
            try:
                import ctypes

                text_bytes = text.encode("utf-16le")
                # GMEM_MOVEABLE = 0x0002
                h_mem = ctypes.windll.kernel32.GlobalAlloc(0x0002, len(text_bytes) + 2)
                p_mem = ctypes.windll.kernel32.GlobalLock(h_mem)
                ctypes.memmove(p_mem, text_bytes, len(text_bytes))
                ctypes.windll.kernel32.GlobalUnlock(h_mem)

                if ctypes.windll.user32.OpenClipboard(None):
                    ctypes.windll.user32.EmptyClipboard()
                    # CF_UNICODETEXT = 13
                    ctypes.windll.user32.SetClipboardData(13, h_mem)
                    ctypes.windll.user32.CloseClipboard()
                    return True
            except Exception:
                pass

            # Fallback to Windows clip command
            proc = subprocess.run(
                ["clip"],
                input=text.encode("utf-16le"),
                check=False,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            return proc.returncode == 0

        elif sys.platform == "darwin":
            proc = subprocess.run(
                ["pbcopy"],
                input=text.encode("utf-8"),
                check=False,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            return proc.returncode == 0

        else:
            # Linux: try wl-copy then xclip
            for cmd in [["wl-copy"], ["xclip", "-selection", "clipboard"]]:
                try:
                    proc = subprocess.run(
                        cmd,
                        input=text.encode("utf-8"),
                        check=False,
                        stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL,
                    )
                    if proc.returncode == 0:
                        return True
                except FileNotFoundError:
                    continue
            return False

    except Exception as e:
        sys.stderr.write(f"Clipboard transport error: {e}\n")
        return False


def find_card_data(topic_id: str) -> Optional[Dict[str, Any]]:
    """
    Searches for card data across rejigger.json and mail_queue.json.
    Matches by exact ID, card index (e.g. 5 or #5), or substring in title/id.
    """
    clean_id = topic_id.strip().lower().lstrip("#")
    
    # 1. Search rejigger.json
    rejigger_path = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "rejigger.json"
    if rejigger_path.exists():
        try:
            cards = json.loads(rejigger_path.read_text(encoding="utf-8"))
            if isinstance(cards, list):
                # Try exact ID
                for c in cards:
                    if str(c.get("id", "")).lower() == clean_id:
                        return c
                # Try rank/index
                if clean_id.isdigit():
                    idx = int(clean_id)
                    for c in cards:
                        if c.get("rank") == idx:
                            return c
                # Try title match
                for c in cards:
                    if clean_id in str(c.get("title", "")).lower():
                        return c
        except Exception:
            pass

    # 2. Search mail_queue.json
    queue_path = REPO_ROOT / ".agents" / "pipeline" / "mailbag" / "mail_queue.json"
    if queue_path.exists():
        try:
            queue_data = json.loads(queue_path.read_text(encoding="utf-8"))
            items = queue_data.get("items", []) if isinstance(queue_data, dict) else queue_data
            if isinstance(items, list):
                for item in items:
                    if str(item.get("id", "")).lower() == clean_id:
                        return item
                    if clean_id in str(item.get("title", "")).lower():
                        return item
        except Exception:
            pass

    return None


def find_vault_note(topic_query: str) -> Optional[Path]:
    """
    Searches TheKlangVault for a matching markdown note.
    """
    vault_root = Path("C:/Dev/TheKlangVault")
    if not vault_root.exists():
        # Check relative or fallback
        vault_root = REPO_ROOT.parent / "TheKlangVault"
    if not vault_root.exists():
        return None

    clean = topic_query.strip().lower()
    
    # Check Archive and Inbox
    search_dirs = [
        vault_root / "Archive",
        vault_root / "Inbox",
        vault_root / "Docs",
    ]

    for sdir in search_dirs:
        if not sdir.exists():
            continue
        for f in sdir.glob("*.md"):
            if clean in f.name.lower() or clean == f.stem.lower():
                return f

    return None


def parse_markdown_note(note_path: Path) -> Tuple[Dict[str, Any], str]:
    """
    Extracts YAML-like frontmatter and body from a markdown note.
    """
    text = note_path.read_text(encoding="utf-8", errors="replace")
    frontmatter = {}
    body = text

    if text.startswith("---"):
        parts = text.split("---", 2)
        if len(parts) >= 3:
            fm_text = parts[1]
            body = parts[2].strip()
            for line in fm_text.splitlines():
                if ":" in line:
                    k, v = line.split(":", 1)
                    frontmatter[k.strip().lower()] = v.strip().strip('"\'[]')

    return frontmatter, body


def compile_payload_from_card(card: Dict[str, Any], profile_override: Optional[str] = None) -> PromptHandoffPayload:
    """
    Transforms a card dictionary into a PromptHandoffPayload.
    """
    thread_id = str(card.get("id", "TOPIC_UNKNOWN"))
    title = card.get("title", "Untitled Topic")
    intent = card.get("synthesis_intent", card.get("summary", title))
    mechanism = card.get("synthesis_mechanism", "")
    route = card.get("synthesis_route", card.get("action_route", ""))
    source_file = card.get("source_file", card.get("source_note", None))
    pillar = card.get("pillar", "NK")

    goals = []
    if mechanism:
        goals.append(f"Proposed Mechanism: {mechanism}")
    if route:
        goals.append(f"Action Route: {route}")
    if not goals and "body" in card:
        # Extract first 3 non-empty lines
        lines = [line.strip().lstrip("-*# ") for line in card["body"].splitlines() if line.strip()]
        goals = lines[:3]

    thesis = ThesisBlock(
        core_intent=f"**{title}**\n\n{intent}",
        user_goals=goals,
        source_note=str(source_file) if source_file else None,
    )

    # Antithesis
    vulns = []
    vectors = []
    persona = "The Dissolution / Adversarial Auditor"
    shatter = "Confrontation with real-world implementation traps and cognitive friction"

    # Check for PCDA / dialectic fields
    dialectic = card.get("dialectic", {})
    if isinstance(dialectic, dict):
        if "cons" in dialectic and isinstance(dialectic["cons"], list):
            vulns.extend([str(c) for c in dialectic["cons"]])
        if "devils_advocate" in dialectic and dialectic["devils_advocate"]:
            vulns.append(str(dialectic["devils_advocate"]))
        if "resolution" in dialectic and dialectic["resolution"]:
            vectors.append(str(dialectic["resolution"]))

    if not vulns:
        vulns = [
            f"Premature implementation trap: Over-engineering {title} before testing ergonomics.",
            "Token/computational churn: Infinite loops or cascading failure modes in unattended execution.",
            "Split-brain state drift: Concurrent modification of shared files without atomic locking.",
        ]

    if not vectors:
        vectors = [
            f"Sleepy Architecture Resolution: Deliver 100% of {title} with sub-80 lines of zero-token code.",
            "Strict One-Way Boundary: Keep all generated artifacts strictly decoupled from core engine runtime.",
        ]

    antithesis = AntithesisBlock(
        reviewer_persona=persona,
        echo_chamber_shatter=shatter,
        structural_vulnerabilities=vulns,
        orthogonal_vectors=vectors,
    )

    # Target constraints
    pillar_map = {
        "TKS": "Native C++20 / JUCE 9.0.3 Audio Plugin DSP & APVTS Controls",
        "TT": "ToadTracker Embedded Engine & M8 / Tracktion Scaffold",
        "NK": "N'kai Sidecar UI Extension & Local Automation Harness",
        "TKR": "TheKlangResearch RFC Specification & DSP Math Proof",
        "AGY": "Google Antigravity Customization, Skill, or Rule System",
        "TBD-16": "ESP32-P4 / Microcontroller Firmware",
    }
    target_mat = pillar_map.get(pillar, "Python Tooling / Antigravity Sidecar Component")

    targets = SynthesisTargetConfig(
        materialization=target_mat,
        the_blueprint=f"Definitive drop-in implementation plan for {title}",
        effort_level="MINIMAL",
        impact_level="MAXIMUM",
        status="PENDING_REASONING",
    )

    # Resolve lens profile (Flight 4)
    cfg = load_lens_profiles()
    tags = card.get("tags", [])
    if isinstance(tags, str):
        tags = [t.strip() for t in tags.split(",") if t.strip()]
    active_profile_id = (
        profile_override
        if (profile_override and profile_override in cfg.profiles)
        else detect_lens_profile(tags, f"{title}\n{card.get('body', '')}", cfg)
    )
    active_profile = cfg.profiles.get(active_profile_id) or cfg.profiles[cfg.default_profile]

    lenses_summary = "\n".join([f"- **{lens.name}**: {lens.prompt}" for lens in active_profile.lenses])

    mandate = (
        f"Analyze the structural tension between my core intent for '{title}' (Thesis) "
        "and the adversarial vulnerabilities/traps (Antithesis). "
        f"Using your extended deep-thinking reasoning mode, evaluate through the 4 specialized lenses of the '{active_profile.display_name}' profile:\n"
        f"{lenses_summary}\n\n"
        "Reconcile these perspectives into a definitive software architecture spec. "
        "Ensure the final blueprint strictly optimizes for Sleepy Architecture principles: achieve MAXIMUM feature leverage with MINIMAL coding execution overhead."
    )

    return PromptHandoffPayload(
        thread_id=thread_id,
        reconciliation_protocol="Method B+ Constructive Adversarial Dialectic",
        thesis=thesis,
        antithesis=antithesis,
        targets=targets,
        prompt_mandate=mandate,
    )


def compile_payload_from_note(note_path: Path, profile_override: Optional[str] = None) -> PromptHandoffPayload:
    """
    Transforms a markdown note file into a PromptHandoffPayload.
    """
    fm, body = parse_markdown_note(note_path)
    title = fm.get("title", note_path.stem)
    pillar = fm.get("pillar", "NK")

    # Extract intent from body
    lines = [l.strip() for l in body.splitlines() if l.strip()]
    first_para = "\n".join(lines[:4]) if lines else "No description provided."

    card = {
        "id": note_path.stem.lower().replace(" ", "_"),
        "title": title,
        "synthesis_intent": first_para,
        "synthesis_mechanism": "Parse and implement as described in source note.",
        "synthesis_route": "High-Reasoning Synthesis",
        "source_file": str(note_path),
        "pillar": pillar,
        "body": body,
    }
    return compile_payload_from_card(card, profile_override=profile_override)


def compile_prompt(
    topic: Optional[str] = None,
    file_path: Optional[str] = None,
    clipboard: bool = False,
    out_file: Optional[str] = None,
    to_vault: bool = False,
    profile: Optional[str] = None,
) -> PromptHandoffPayload:
    """
    Core compiler entrypoint. Resolves topic or file, formats payload,
    and dispatches to destination.
    """
    payload = None

    if file_path:
        p = Path(file_path).resolve()
        if not p.exists():
            raise FileNotFoundError(f"Source file not found: {file_path}")
        if p.suffix == ".json":
            data = json.loads(p.read_text(encoding="utf-8"))
            if "thesis" in data and "thread_id" in data:
                payload = PromptHandoffPayload.model_validate(data)
            else:
                payload = compile_payload_from_card(data, profile_override=profile)
        else:
            payload = compile_payload_from_note(p, profile_override=profile)

    elif topic:
        # Check card data in pipeline
        card = find_card_data(topic)
        if card:
            payload = compile_payload_from_card(card, profile_override=profile)
        else:
            # Check vault note
            note = find_vault_note(topic)
            if note:
                payload = compile_payload_from_note(note, profile_override=profile)
            else:
                # Ad-hoc topic
                card = {
                    "id": topic.lower().replace(" ", "_"),
                    "title": topic,
                    "synthesis_intent": f"Architect and synthesize solution for: {topic}",
                    "synthesis_mechanism": "Standard engineering execution",
                    "pillar": "NK",
                }
                payload = compile_payload_from_card(card, profile_override=profile)
    else:
        raise ValueError("Must provide either --topic or --file.")

    md_output = payload.to_markdown()

    # Destination handling
    if clipboard:
        ok = copy_to_clipboard(md_output)
        if ok:
            print("📋 Successfully copied compiled reasoning prompt to system clipboard!")
        else:
            print("⚠️ Failed to copy to clipboard; displaying on stdout.")

    if out_file:
        out_p = Path(out_file).resolve()
        out_p.parent.mkdir(parents=True, exist_ok=True)
        out_p.write_text(md_output, encoding="utf-8")
        print(f"💾 Saved compiled prompt to: {out_p}")

    if to_vault:
        vault_dispatch = Path("C:/Dev/TheKlangVault/Dispatches/reasoning")
        if not vault_dispatch.parent.parent.exists():
            vault_dispatch = REPO_ROOT.parent / "TheKlangVault" / "Dispatches" / "reasoning"
        vault_dispatch.mkdir(parents=True, exist_ok=True)
        vault_target = vault_dispatch / f"{payload.thread_id}_reasoning_prompt.md"
        payload.to_vault_file(vault_target)
        print(f"📦 Staged reasoning prompt in vault: {vault_target}")

    return payload


def main():
    parser = argparse.ArgumentParser(
        description="N'kai to High Reasoning Prompt Compiler (Flight 2)"
    )
    parser.add_argument(
        "--topic",
        "-t",
        help="Card ID, rank (#N), or search query across mailbag / vault",
    )
    parser.add_argument(
        "--file",
        "-f",
        help="Path to markdown note or JSON card file",
    )
    parser.add_argument(
        "--profile",
        "-p",
        choices=["audio_dev", "studio_hardware", "creative_general"],
        help="Explicit lens profile override (audio_dev, studio_hardware, creative_general)",
    )
    parser.add_argument(
        "--clipboard",
        "-c",
        action="store_true",
        help="Automatically copy compiled markdown to OS system clipboard",
    )
    parser.add_argument(
        "--out",
        "-o",
        help="Write compiled markdown output to a specific file",
    )
    parser.add_argument(
        "--vault",
        action="store_true",
        help="Stage compiled prompt in TheKlangVault/Dispatches/reasoning/",
    )
    parser.add_argument(
        "--stdout",
        action="store_true",
        help="Print compiled markdown payload directly to terminal stdout",
    )

    args = parser.parse_args()

    if not args.topic and not args.file:
        parser.print_help()
        sys.exit(1)

    try:
        # Default to stdout if no destinations were requested
        if not args.clipboard and not args.out and not args.vault:
            args.stdout = True

        payload = compile_prompt(
            topic=args.topic,
            file_path=args.file,
            clipboard=args.clipboard,
            out_file=args.out,
            to_vault=args.vault,
            profile=args.profile,
        )

        if args.stdout:
            print(payload.to_markdown())

    except Exception as e:
        sys.stderr.write(f"❌ Error compiling reasoning prompt: {e}\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
