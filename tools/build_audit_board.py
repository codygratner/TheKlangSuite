#!/usr/bin/env python3
"""
tools/build_audit_board.py — Systemic Audit & Triage Board Generator
===================================================================
Constructs a declarative BoardSchema containing the 12 key systemic audit
decisions from the 4-Cluster Method B teardown and renders a brand-new
interactive N'kai triage board.
Universal UTF-8 compliance guaranteed.
"""

import html
import json
import sys
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.models import (
    BoardSchema,
    MailItem,
    PriorityLevel,
    TriageStatus,
    BoardActionDefinition,
    FilterRail,
    FilterRailItem,
)
from tools.board_renderer import render_board
from tools.board_registry import load_registry, save_registry, BoardMetadata


def build_audit_items():
    items = [
        MailItem(
            id="audit-msg-boundary",
            title="Air-Gap Boundary Law: Data Bus (Diplomacy) vs. Communiqué (Local Execution)",
            date="2026-10-12",
            priority=PriorityLevel.P0,
            status=TriageStatus.APPROVED,
            target_pillars=["AGY", "TKS"],
            tags=["messaging", "concurrency", "safety", "boundary-law"],
            preview="Keep strict separation: Data Bus is asynchronous WAN mail; Communiqué is synchronous local compiler mutex.",
            body="""### The Core Dilemma
Should our internal factory execution files (`factory_dispatch.md` ↔ `factory_receipt.md`) be merged into the cross-project Data Bus (`Dispatches/`)?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Unifying all communications under a single event bus (`tools/bus.py`) eliminates duplicate code, unifies YAML schemas, and provides global visibility in `The Cosmos/Agenda.md`.
- **🔴 Devil's Advocate Antithesis**: Merging them creates the **Distributed Monolith Antipattern**. The Data Bus is an *inter-domain asynchronous postal service* (loss-tolerant, eventually consistent). The Communiqué is an *intra-domain synchronous execution latch* guarding a hardware singleton (the MSBuild compiler lock and Git working tree). If Cosmic Codex could trigger factory builds automatically, a stray thought in the vault could lock MSBuild for 10 minutes or violate audio safety.
- **🟣 80/20 Resolution**: **STRICT SEPARATION (Approved & Locked)**. Data Bus is for diplomacy, specs, and status; Communiqués are strictly local execution semaphores requiring human confirmation.""",
            idx=1,
            estimated_tokens=4500,
            char_count=1250,
            word_count=180,
            tier="tier2",
        ),
        MailItem(
            id="audit-msg-intent-locks",
            title="Intent Locks & Blast-Radius Concurrency Telemetry (Flight 1B)",
            date="2026-10-12",
            priority=PriorityLevel.P0,
            status=TriageStatus.COMPLETED,
            target_pillars=["AGY", "COSMOS"],
            tags=["concurrency", "intent-locks", "bus", "telemetry"],
            preview="Implemented ephemeral intent locks with 30m dead-man's TTL and state manager collision radar.",
            body="""### The Core Dilemma
How do we prevent concurrent agent sessions (Cosmic Codex in The Cosmos and New Klang City in The Klang Suite) from colliding on shared directory structures like `TheKlangVault`?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Distributed file locks are standard in distributed systems to guarantee exclusive write access.
- **🔴 Devil's Advocate Antithesis**: Hard blocking distributed locks are catastrophic in LLM agent workflows. If an agent crashes, hits context truncation, or clearing occurs, permanent deadlock ensues.
- **🟣 80/20 Resolution**: **EPHEMERAL INTENT LOCKS WITH DEAD-MAN'S TTL (Implemented)**.
  - Implemented in `tools/bus.py`: `lock`, `unlock`, `locks`, and `check-scope`.
  - Non-blocking advisory telemetry: broadcasts intention across workspaces.
  - 30-minute auto-expiration prevents orphan deadlocks.
  - Wired directly into `tools/state_manager.py` radar.""",
            idx=2,
            estimated_tokens=4200,
            char_count=1180,
            word_count=165,
            tier="tier2",
        ),
        MailItem(
            id="audit-exec-tooling-ratchet",
            title="The Zero-Token Tooling Ratchet Invariant (Topic #6)",
            date="2026-10-12",
            priority=PriorityLevel.P0,
            status=TriageStatus.APPROVED,
            target_pillars=["TKS", "AGY"],
            tags=["tooling-ratchet", "zero-token", "invariants", "testing"],
            preview="Encode all discovered bugs and manual chores into Python CLI tools + unit tests to permanently eliminate token churn.",
            body="""### The Core Dilemma
Should we spend engineering turns writing bespoke Python CLI tools and unit tests for regressions, or let the LLM handle them conversationally in chat?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Chat-based fixes are fast and immediate; writing Python scripts feels like extra friction.
- **🔴 Devil's Advocate Antithesis**: Chat-based fixes have zero retention across `/clear` or fresh sessions. Regressions recur constantly, burning tens of thousands of expensive LLM tokens re-explaining and re-debugging the same issues.
- **🟣 80/20 Resolution**: **THE TOOLING RATCHET INVARIANT (Codified in GEMINI.md)**.
  - Every discovered invariant violation (audiothread, frontmatter, UTF-8, template drift) MUST be codified into `tools/` with a unit test in `tools/tests/`.
  - Future checks execute in sub-10ms for 0 LLM tokens, permanently immunizing the codebase.""",
            idx=3,
            estimated_tokens=3900,
            char_count=1100,
            word_count=155,
            tier="tier2",
        ),
        MailItem(
            id="audit-exec-hard-safety",
            title="Hard Audio Thread Safety Gauntlet (audiothread-guard & build-validate)",
            date="2026-10-12",
            priority=PriorityLevel.P0,
            status=TriageStatus.COMPLETED,
            target_pillars=["TKS"],
            tags=["audiothread", "dsp", "safety", "build-validate"],
            preview="Non-negotiable AST/regex scanning for real-time safety. Token-shield terse mode added in Flight 6.",
            body="""### The Core Dilemma
Does strict automated AST/regex scanning of DSP code slow down iterative feature velocity?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Compilers check syntax; unit tests check logic. AST scanning of audio loops is extra overhead.
- **🔴 Devil's Advocate Antithesis**: In real-time audio DSP, a single hidden `malloc`, lock acquisition, or CRT transcendental in `processBlock()` causes catastrophic audio glitching and DAW plugin blacklist rejection. LLMs frequently hallucinate dynamic allocations.
- **🟣 80/20 Resolution**: **UNTOUCHABLE INVARIANT (Hardened in Flight 6)**.
  - `audiothread-guard` remains the non-negotiable gatekeeper before any code commit.
  - Flight 6 added `--terse` / `--token-shield` mode: emits <10 tokens on clean runs and <100 tokens on failures, protecting context allowance.""",
            idx=4,
            estimated_tokens=3800,
            char_count=1050,
            word_count=150,
            tier="tier1",
        ),
        MailItem(
            id="audit-triage-overlay",
            title="Retire Static rejigger.json Overlay in Favor of Dynamic Multi-Key Sorting",
            date="2026-10-12",
            priority=PriorityLevel.P1,
            status=TriageStatus.APPROVED,
            target_pillars=["NK"],
            tags=["triage", "rejigger", "sorting", "data-model"],
            preview="Eliminate brittle 37-card static JSON overlay in favor of dynamic sorting by (priority, status, date) with contiguous index.",
            body="""### The Core Dilemma
Is maintaining a 37-card static `rejigger.json` file sustainable as new human voice memos and notes arrive daily?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: The curated 37-card overlay provided an ordered, high-conviction view of early priorities.
- **🔴 Devil's Advocate Antithesis**: Static integer ranks (1..37) require $O(N)$ cascading array re-indexing on every insertion. The JSON overlay drifts from raw Markdown truth, creating two competing sources of reality.
- **🟣 80/20 Resolution**: **DYNAMIC MULTI-KEY TRIAGE (Flight 5 Completed)**.
  - Built into `tools/check_mail.py` and `tools/models.py`.
  - Automatically sorts by `(priority_weight, status_weight, date_desc)` and generates contiguous `idx: 1..N` at render time.
  - Safe to retire static `rejigger.json` overlay.""",
            idx=5,
            estimated_tokens=4100,
            char_count=1150,
            word_count=160,
            tier="tier2",
        ),
        MailItem(
            id="audit-mem-blackbox",
            title="Deterministic Black Box Context Management (tools/state_manager.py)",
            date="2026-10-12",
            priority=PriorityLevel.P1,
            status=TriageStatus.COMPLETED,
            target_pillars=["AGY"],
            tags=["memory", "blackbox", "context-clues", "state-manager"],
            preview="Replaced 23KB manual context clues with 5ms deterministic Python state sniffer generating <1KB ACTIVE_CONTEXT.md.",
            body="""### The Core Dilemma
Are our manual context clues files (`context_clues_build.md`, `context_clues_plan.md`, etc.) protecting against amnesia, or are they an expensive compliance tax?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Detailed Markdown files give booting LLMs comprehensive historical orientation.
- **🔴 Devil's Advocate Antithesis**: 4 manual files (23.2 KB) consume 30,000 tokens on cold boot for static boilerplate that is 80% redundant. High-tier models spend valuable time hand-formatting diaries.
- **🟣 80/20 Resolution**: **AUTOMATED BLACK BOX SYSTEM (Stage 2 Completed)**.
  - `tools/state_manager.py` sniffs git, plan progress, communiqués, and peer locks in <5ms.
  - Generates `ACTIVE_CONTEXT.md` (<1KB vs 23KB, 97.3% token reduction).
  - Enables true 0-turn cold boot with process runtime detection.""",
            idx=6,
            estimated_tokens=4400,
            char_count=1220,
            word_count=170,
            tier="tier2",
        ),
        MailItem(
            id="audit-ui-template-sync",
            title="Multi-Repo Template Synchronization Engine (Flight 4)",
            date="2026-10-12",
            priority=PriorityLevel.P1,
            status=TriageStatus.COMPLETED,
            target_pillars=["NK"],
            tags=["ui", "templates", "sync", "parity"],
            preview="Automated SHA-256 parity auditor preventing template drift across nkai core, sidecars, and bundle templates.",
            body="""### The Core Dilemma
How do we prevent subtle UI template drift across `nkai/core`, `TheKlangSuite/.agents/sidecar`, and `nkai/bundle/templates`?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Developers can manually copy modified HTML files when making sidecar changes.
- **🔴 Devil's Advocate Antithesis**: Manual syncing fails predictably. Templates drift apart, leading to elusive bugs where features work in one workspace but fail in another.
- **🟣 80/20 Resolution**: **ZERO-TOKEN TEMPLATE SYNCHRONIZER (Flight 4 Completed)**.
  - Implemented `tools/sync_templates.py` (`check`, `sync`, `--force`).
  - Asserts 100% SHA-256 hash parity across all template targets.
  - Backed by unit tests in `tools/tests/test_sync_templates.py`.""",
            idx=7,
            estimated_tokens=3600,
            char_count=1020,
            word_count=145,
            tier="tier2",
        ),
        MailItem(
            id="audit-capture-simplenote",
            title="Frictionless Edge Capture: sync_simplenote.py & Mobile Couch Triage",
            date="2026-10-12",
            priority=PriorityLevel.P1,
            status=TriageStatus.COMPLETED,
            target_pillars=["COSMOS", "NK"],
            tags=["capture", "simplenote", "mobile", "couch-triage"],
            preview="Retain Simplenote sync and mobile LAN server with QR code for frictionless couch triage.",
            body="""### The Core Dilemma
Should human ideation be captured directly in Git repositories, or through frictionless external note apps?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Capturing everything in Git enforces typed discipline and version control from moment zero.
- **🔴 Devil's Advocate Antithesis**: Git is too high friction for middle-of-the-night voice memos and phone rambles. If capturing an idea requires git commits or opening an IDE, ideas are lost.
- **🟣 80/20 Resolution**: **TWO-SPEED CAPTURE ARCHITECTURE (Keep & Retain)**.
  - `tools/sync_simplenote.py` syncs mobile memos to `TheKlangVault/Inbox/`.
  - `tools/nkai_serve.py --lan` serves mobile companion board with QR code for couch triage.
  - Triage transforms raw thoughts into typed backlog items with 1 click.""",
            idx=8,
            estimated_tokens=3700,
            char_count=1080,
            word_count=150,
            tier="tier2",
        ),
        MailItem(
            id="audit-triage-overflow",
            title="Deprecate overflow.json & tools/mail_add.py (Touch-It-Once Invariant)",
            date="2026-10-12",
            priority=PriorityLevel.P2,
            status=TriageStatus.PENDING,
            target_pillars=["NK"],
            tags=["triage", "overflow", "mail-add", "gtd"],
            preview="Retire overflow purgatory queue and mail_add.py in favor of direct Backlog or Archive routing.",
            body="""### The Core Dilemma
Should decomposed multi-idea rambles maintain an `overflow.json` secondary queue, and should `mail_add.py` splice cards at specific integer ranks?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Overflow queues prevent long voice dumps from overwhelming the primary triage board.
- **🔴 Devil's Advocate Antithesis**: GTD methodology demands processing an inbox, not building secondary waiting rooms. `overflow.json` is idea-hoarding purgatory. `mail_add.py` only existed to support brittle contiguous integer ranking in `rejigger.json`.
- **🟣 80/20 Resolution**: **PRUNE SECONDARY QUEUES (Proposed Stage 3)**.
  - Long rambles decomposed by `ramble_ingest.py` should route top 3 ideas to active cards and remainder directly to `docs/BACKLOG.md` (low priority) or `TheKlangVault/Inbox/Archive/`.
  - Delete `mail_add.py` once dynamic sorting replaces static ranking.""",
            idx=9,
            estimated_tokens=3900,
            char_count=1120,
            word_count=160,
            tier="tier2",
        ),
        MailItem(
            id="audit-ui-universal-shell",
            title="Consolidate Bespoke HTML Boards into Universal Dynamic Shell (shell.html)",
            date="2026-10-12",
            priority=PriorityLevel.P2,
            status=TriageStatus.PENDING,
            target_pillars=["NK"],
            tags=["ui", "sidecar", "shell", "consolidation"],
            preview="Merge morning_mail, cli_board, and roadmap_sidecar into a single canonical shell rendering JSON payloads.",
            body="""### The Core Dilemma
Does maintaining 5 distinct bespoke HTML boards across repositories create rendering drag and template bloat?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: Dedicated HTML files are completely standalone and have zero cross-view dependencies.
- **🔴 Devil's Advocate Antithesis**: 5 bespoke boards duplicate 90% of their CSS, JS bridges, haptic listeners, and theme variables. Updating a button style requires touching 5 files across 3 repos.
- **🟣 80/20 Resolution**: **UNIVERSAL DYNAMIC SHELL (Proposed Stage 4)**.
  - Build `nkai/sidecars/panel/shell.html`.
  - Shell aggressively caches static CSS/JS and dynamically renders different views (Triage, Roadmap, Research, CLI) from JSON payloads.
  - Eliminates template sprawl and cache-busting hacks.""",
            idx=10,
            estimated_tokens=4000,
            char_count=1140,
            word_count=160,
            tier="tier2",
        ),
        MailItem(
            id="audit-ui-sandbox-mock",
            title="Replace Duplicate sandbox_mail_inbox.html with --mock Data Flag",
            date="2026-10-12",
            priority=PriorityLevel.P2,
            status=TriageStatus.PENDING,
            target_pillars=["NK"],
            tags=["ui", "sandbox", "testing", "refactor"],
            preview="Retire duplicate sandbox HTML file; supply synthetic fixtures directly to canonical shell via --mock.",
            body="""### The Core Dilemma
Should QA sandbox testing use a separate HTML template (`sandbox_mail_inbox.html`) or feed test data to the production template?

### Hegelian Dialectic Teardown
- **🟢 Thesis**: A separate sandbox file guarantees zero chance of accidental mutations to production boards.
- **🔴 Devil's Advocate Antithesis**: Copying HTML to create a sandbox is a testing antipattern. The sandbox template inevitably drifts from production, meaning tests pass on stale UI while production is broken.
- **🟣 80/20 Resolution**: **DATA-DRIVEN MOCK MODE (Proposed Stage 4)**.
  - Keep deterministic test fixtures (`assets/fixtures/sandbox_mail_queue.json`).
  - Feed fixtures directly into the canonical template using `--mock`, rendering with the amber sandbox banner.
  - Delete `sandbox_mail_inbox.html`.""",
            idx=11,
            estimated_tokens=3500,
            char_count=1000,
            word_count=140,
            tier="tier2",
        ),
        MailItem(
            id="audit-roadmap-pivot",
            title="Strategic Sequence: TKS v0.5.0 (Sound & Chaos) Before ToadTracker PoC",
            date="2026-10-12",
            priority=PriorityLevel.P1,
            status=TriageStatus.APPROVED,
            target_pillars=["TKS", "TT"],
            tags=["roadmap", "sequencing", "dsp", "renoise"],
            preview="Stress-tested 7-stage flight path: push TKS FM/Bass DSP first so ToadTracker Renoise export loads working sounds.",
            body="""### The Core Dilemma
Should we pivot to ToadTracker immediately after TKS v0.4.1 (UI shells) or push TKS through v0.5.0 (synthesis DSP) first?

### Hegelian Dialectic Teardown
- **🟢 Thesis (Early Pivot)**: Switching to TT early provides a change of scenery and validates the SteamOS gamepad tracker.
- **🔴 Devil's Advocate Antithesis**: Pivoting to TT with hollow UI shells means TT's Renoise `.xrns` export loads silent, placeholder plugins. Switching back and forth between low-level Linux gamepad C++ and JUCE plugin DSP creates massive cognitive thrashing.
- **🟣 80/20 Resolution**: **OPTION BETA (Locked in Roadmap)**.
  - Push TKS v0.5.0 (\"Sound & Chaos\": 2-op FM, Bass Mode, Phase Smear) first.
  - When TT exports Renoise projects, The Klang Farmer and Planter already produce rich sound.
  - High creative momentum and zero false-start pivots.""",
            idx=12,
            estimated_tokens=4200,
            char_count=1190,
            word_count=165,
            tier="tier1",
        ),
    ]
    return items


def main():
    items = build_audit_items()
    
    schema = BoardSchema(
        schema_version="1.0.0",
        board_id="system_sanity_audit",
        title="System Sanity Audit & Triage Lab",
        subtitle="Forensic Method B Dialectics across Messaging, Capture, Sidecars & Memory",
        environment="PRODUCTION",
        allowance_budget=200000,
        cards=items,
    )
    
    output_path = REPO_ROOT / ".agents" / "sidecar" / "system_sanity_audit.html"
    output_path.parent.mkdir(parents=True, exist_ok=True)
    print(f"[*] Rendering System Sanity Audit Board HTML to {output_path}...")
    render_board(schema, output_path)
    print(f"[+] Wrote board to: {output_path}")
    
    # Mirror to nkai panel
    nkai_panel_path = Path(r"c:\Dev\nkai\sidecars\panel\system_sanity_audit.html")
    if nkai_panel_path.parent.exists():
        render_board(schema, nkai_panel_path)
        print(f"[+] Mirrored board to: {nkai_panel_path}")

    # Register in registry.json as active board
    print("[*] Updating Board Registry...")
    registry = load_registry()
    
    # Find or update system_sanity_audit
    found = False
    for b in registry.boards:
        if b.id == "system_sanity_audit":
            b.title = "System Sanity Audit & Triage Lab"
            b.description = "Forensic Method B Dialectics across Messaging, Capture, Sidecars & Memory"
            b.category = "active"
            b.file_path = ".agents/sidecar/system_sanity_audit.html"
            b.environment = "PRODUCTION"
            b.card_count = len(items)
            found = True
            break
            
    if not found:
        registry.boards.append(
            BoardMetadata(
                id="system_sanity_audit",
                title="System Sanity Audit & Triage Lab",
                description="Forensic Method B Dialectics across Messaging, Capture, Sidecars & Memory",
                category="active",
                file_path=".agents/sidecar/system_sanity_audit.html",
                environment="PRODUCTION",
                card_count=len(items),
            )
        )
        
    save_registry(registry)
    print(f"[+] Registry updated: 'system_sanity_audit' is now ACTIVE ({len(items)} cards).")
    print("✅ Audit Board Generation Complete!")


if __name__ == "__main__":
    main()
