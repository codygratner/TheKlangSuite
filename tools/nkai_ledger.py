#!/usr/bin/env python3
"""
N'kai Cumulative Allowance Ledger & Token Estimator (tools/nkai_ledger.py)
Part of the N'kai Asymmetric Sidecar Framework (Topics 6.3 & 6.19).

Zero-dependency CLI utility to audit living HTML research boards, extract triage verdicts,
tally model tier token commitments, and verify cumulative allowance thresholds.

Usage:
    python tools/nkai_ledger.py --audit <board.html>
    python tools/nkai_ledger.py --audit <board.html> --chat
    python tools/nkai_ledger.py --audit <board.html> --summary
    python tools/nkai_ledger.py --audit <board.html> --threshold 250000
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

# Reconfigure stdout/stderr for UTF-8 on Windows
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

TIER_RATES = {
    "tier1": 40000,
    "tier2": 10000,
    "tier3": 2000
}

TIER_LABELS = {
    "tier1": "Tier 1 Pro High (~40k)",
    "tier2": "Tier 2 Flash High (~10k)",
    "tier3": "Tier 3 Flash Low (~2k)"
}


class TriageItem:
    def __init__(self, item_id: str, title: str, verdict: Optional[str] = None, note: str = "", tier: str = "tier2"):
        self.item_id = item_id
        self.title = title
        self.verdict = (verdict.upper() if verdict else None) if verdict else None
        self.note = note
        self.tier = tier.lower() if tier else "tier2"
        if self.tier not in TIER_RATES:
            self.tier = "tier2"

    @property
    def token_cost(self) -> int:
        return TIER_RATES.get(self.tier, 10000)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.item_id,
            "title": self.title,
            "verdict": self.verdict,
            "note": self.note,
            "tier": self.tier,
            "tier_label": TIER_LABELS.get(self.tier, "Tier 2 Flash"),
            "token_cost": self.token_cost
        }


class NkaiBoardAuditor:
    def __init__(self, file_path: Path, max_threshold: int = 200000, safe_threshold: int = 100000):
        self.file_path = file_path
        self.max_threshold = max_threshold
        self.safe_threshold = safe_threshold
        self.items: Dict[str, TriageItem] = {}
        if not self.file_path.exists():
            raise FileNotFoundError(f"Board HTML file not found: {self.file_path}")
        self.content = self.file_path.read_text(encoding="utf-8", errors="replace")
        self._parse()

    def _infer_tier(self, title: str, raw_tier: Optional[str] = None) -> str:
        if raw_tier and raw_tier.lower() in TIER_RATES:
            return raw_tier.lower()
        t = title.lower()
        if re.search(r'\b(dsp|differential|saturation|concurrency|threading|polyphon|filter|fader|math|equation)\b', t):
            return "tier1"
        if re.search(r'\b(glyph|schema|doc|chore|catalog|json|tweak|clean|typo)\b', t):
            return "tier3"
        return "tier2"

    def _parse(self):
        # 1. Parse triageData JS object if present
        triage_match = re.search(r'const\s+triageData\s*=\s*\{([^\}]+(?:\{[^\}]+\}[^\}]*)*)\};', self.content, re.DOTALL)
        if triage_match:
            block = triage_match.group(1)
            # Find item entries: '1.1': { title: '...', verdict: ..., note: '...', tier: '...' }
            pattern = re.compile(
                r"['\"]([0-9A-Za-z\.\-_]+)['\"]\s*:\s*\{\s*title:\s*['\"]([^'\"]*)['\"]"
                r"(?:,\s*verdict:\s*(['\"][A-Za-z]+['\"]|null))?"
                r"(?:,\s*note:\s*['\"]([^'\"]*)['\"])?"
                r"(?:,\s*tier:\s*['\"]([^'\"]*)['\"])?",
                re.DOTALL
            )
            for m in pattern.finditer(block):
                i_id = m.group(1)
                title = m.group(2)
                raw_v = m.group(3)
                verdict = raw_v.strip("'\"") if raw_v and raw_v != "null" else None
                note = m.group(4) or ""
                raw_tier = m.group(5) or None
                tier = self._infer_tier(title, raw_tier)
                self.items[i_id] = TriageItem(i_id, title, verdict, note, tier)

        # 2. Extract DOM cards if triageData did not find all items
        card_matches = re.finditer(
            r'id=["\'](?:card|hero-stage|autopsy-item)-([0-9A-Za-z\.\-_]+)["\'][^>]*(?:data-tier=["\']([^"\']+)["\'])?',
            self.content
        )
        for m in card_matches:
            c_id = m.group(1)
            dom_tier = m.group(2)
            if c_id not in self.items:
                self.items[c_id] = TriageItem(c_id, f"Topic {c_id}", None, "", dom_tier or "tier2")
            elif dom_tier:
                self.items[c_id].tier = dom_tier.lower()

    def calculate(self) -> Dict[str, Any]:
        total_count = len(self.items)
        approved_items = [it for it in self.items.values() if it.verdict == "APPROVE"]
        tabled_items = [it for it in self.items.values() if it.verdict == "TABLE"]
        killed_items = [it for it in self.items.values() if it.verdict == "KILL"]
        pending_items = [it for it in self.items.values() if it.verdict not in ("APPROVE", "TABLE", "KILL")]

        tokens_t1 = sum(it.token_cost for it in approved_items if it.tier == "tier1")
        tokens_t2 = sum(it.token_cost for it in approved_items if it.tier == "tier2")
        tokens_t3 = sum(it.token_cost for it in approved_items if it.tier == "tier3")
        tokens_total = tokens_t1 + tokens_t2 + tokens_t3

        count_t1 = sum(1 for it in approved_items if it.tier == "tier1")
        count_t2 = sum(1 for it in approved_items if it.tier == "tier2")
        count_t3 = sum(1 for it in approved_items if it.tier == "tier3")

        # Health Band
        if tokens_total > self.max_threshold:
            band = "critical"
            band_label = "CRITICAL"
            band_icon = "🔴"
            band_desc = "Exceeds quota ceiling! Decompose or shift tasks to Tier 2/3 before dispatching."
        elif tokens_total > self.safe_threshold:
            band = "warning"
            band_label = "WARNING"
            band_icon = "🟡"
            band_desc = "Multi-sprint flight budget alert. Recommend staged execution."
        else:
            band = "safe"
            band_label = "SAFE"
            band_icon = "🟢"
            band_desc = "Clear to proceed. Sustainable within standard allowance."

        pct_budget = (tokens_total / self.max_threshold * 100) if self.max_threshold > 0 else 0.0

        return {
            "file": str(self.file_path),
            "tokens_total": tokens_total,
            "tokens_t1": tokens_t1,
            "tokens_t2": tokens_t2,
            "tokens_t3": tokens_t3,
            "counts": {
                "total": total_count,
                "approved": len(approved_items),
                "tabled": len(tabled_items),
                "killed": len(killed_items),
                "pending": len(pending_items),
                "approved_t1": count_t1,
                "approved_t2": count_t2,
                "approved_t3": count_t3,
            },
            "thresholds": {
                "safe": self.safe_threshold,
                "max": self.max_threshold,
                "pct_consumed": round(pct_budget, 1)
            },
            "band": band,
            "band_label": band_label,
            "band_icon": band_icon,
            "band_description": band_desc,
            "approved_items": [it.to_dict() for it in approved_items],
            "all_items": [it.to_dict() for it in self.items.values()]
        }


def render_ascii_meter(pct: float, width: int = 32) -> str:
    filled = min(width, max(0, int(round((pct / 100.0) * width))))
    empty = width - filled
    return "[" + ("█" * filled) + ("░" * empty) + "]"


def format_chat_output(res: Dict[str, Any]) -> str:
    c = res["counts"]
    th = res["thresholds"]
    lines = [
        f"### 🔋 N'kai Cumulative Allowance Ledger: `{Path(res['file']).name}`",
        f"> **Cumulative Token Commitment:** {res['tokens_total']:,} / {th['max']:,} tokens  ",
        f"> **Allowance Status:** {res['band_label']} {res['band_icon']} ({res['band_description']})  ",
        f"> **Triage Scoreboard:** {c['approved']} Approved | {c['tabled']} Tabled | {c['killed']} Killed | {c['pending']} Pending",
        "",
        "| Model Tier | Approved Tasks | Token Rate | Cumulative Tokens | Quota Impact |",
        "| :--- | :---: | :---: | :---: | :--- |",
        f"| **Tier 1 (Pro High)** | {c['approved_t1']} | ~40,000 | {res['tokens_t1']:,} tokens | ⚠️ High Impact (DSP/Math) |",
        f"| **Tier 2 (Flash High)** | {c['approved_t2']} | ~10,000 | {res['tokens_t2']:,} tokens | 🟢 Sustainable (Default) |",
        f"| **Tier 3 (Flash Low)** | {c['approved_t3']} | ~2,000 | {res['tokens_t3']:,} tokens | ⚡ Minimal (Chores/Schemas) |",
        f"| **Total Approved Commitment** | **{c['approved']}** | — | **{res['tokens_total']:,} tokens** | **{res['band_label']} {res['band_icon']} ({th['pct_consumed']}%)** |",
        ""
    ]

    if res["approved_items"]:
        lines.append("**Itemized Approved Flight Queue:**")
        running = 0
        for it in res["approved_items"]:
            running += it["token_cost"]
            lines.append(f"- **Item {it['id']} ({it['title']}):** {it['tier_label']} — `{it['token_cost']:,}` tokens (running: `{running:,}`)")
    else:
        lines.append("_No topics marked APPROVE yet. Tally reflects baseline active queue._")

    return "\n".join(lines)


def format_cli_output(res: Dict[str, Any]) -> str:
    c = res["counts"]
    th = res["thresholds"]
    meter = render_ascii_meter(th["pct_consumed"], 30)

    lines = [
        "======================================================================",
        f"  N'kai Cumulative Allowance Ledger (Topic 6.19)",
        "======================================================================",
        f"  * Target Board:    {res['file']}",
        f"  * Active Status:   {res['band_label']} {res['band_icon']} ({th['pct_consumed']}% of {th['max']:,} ceiling)",
        f"  * Cumulative Burn: {res['tokens_total']:,} tokens",
        f"  * Progress Meter:  {meter} {th['pct_consumed']}%",
        "----------------------------------------------------------------------",
        f"  SCOREBOARD:  {c['total']} Total | {c['approved']} Approved | {c['tabled']} Tabled | {c['killed']} Killed | {c['pending']} Pending",
        "----------------------------------------------------------------------",
        "  MODEL TIER BREAKDOWN:",
        f"    • Tier 1 Pro High   (~40k):  {c['approved_t1']:2d} approved tasks -> {res['tokens_t1']:7,d} tokens (⚠️ HIGH)",
        f"    • Tier 2 Flash High (~10k):  {c['approved_t2']:2d} approved tasks -> {res['tokens_t2']:7,d} tokens (🟢 SUSTAINABLE)",
        f"    • Tier 3 Flash Low   (~2k):  {c['approved_t3']:2d} approved tasks -> {res['tokens_t3']:7,d} tokens (⚡ MINIMAL)",
        "----------------------------------------------------------------------",
        f"  GOVERNANCE VERDICT: {res['band_description']}"
    ]

    if res["approved_items"]:
        lines.append("\n  APPROVED QUEUE:")
        for it in res["approved_items"]:
            lines.append(f"    [+] {it['id']:5s} | {it['tier'].upper():5s} | {it['token_cost']:6,d} tokens | {it['title']}")

    lines.append("======================================================================")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="N'kai Cumulative Allowance Ledger & Token Estimator",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--audit", required=True, help="Path to N'kai HTML research board or sidecar template.")
    parser.add_argument("--chat", action="store_true", help="Print concise, markdown-formatted allowance scorecard.")
    parser.add_argument("--summary", action="store_true", help="Output machine-readable JSON summary.")
    parser.add_argument("--threshold", type=int, default=200000, help="Custom critical threshold ceiling in tokens (default: 200,000).")
    parser.add_argument("--safe-threshold", type=int, default=100000, help="Custom safe threshold band in tokens (default: 100,000).")

    args = parser.parse_args()

    board_path = Path(args.audit)
    if not board_path.exists():
        print(f"[ERROR] Board file not found: {board_path}", file=sys.stderr)
        sys.exit(1)

    auditor = NkaiBoardAuditor(board_path, max_threshold=args.threshold, safe_threshold=args.safe_threshold)
    result = auditor.calculate()

    if args.summary:
        print(json.dumps(result, indent=2))
    elif args.chat:
        print(format_chat_output(result))
    else:
        print(format_cli_output(result))

    # Exit code: 0 if safe/warning, 2 if critical
    if result["band"] == "critical":
        sys.exit(2)
    sys.exit(0)


if __name__ == "__main__":
    main()
