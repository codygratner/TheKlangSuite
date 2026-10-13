#!/usr/bin/env python3
"""
Pydantic Data Models for Mailbag, Triage Ledger, and Vault Studio Hub (Topic 6.11 / Flight 5)
Provides schema validation, priority queue sorting, and atomic transaction receipts.
"""

from datetime import datetime
from enum import Enum
import json
from pathlib import Path
import re
from typing import Any, Dict, List, Optional
from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator


class EcosystemPillar(str, Enum):
    TKS = "TKS"        # The Klang Suite (Audio Plugins, DSP, JUCE, VST3/AU)
    TT = "TT"          # ToadTracker (Tracker Engine & M8 Bridge)
    NK = "NK"          # N'kai (Sidecar, Webviews & Triage Lab)
    TKR = "TKR"        # TheKlangResearch (RFCs, Papers & DSP Math)
    AGY = "AGY"        # Antigravity (Skills, Harness, Rules & Workflows)
    TBD16 = "TBD-16"    # Hardware (ESP32-P4 / Microcontroller Firmware)


PILLAR_METADATA = {
    "TKS": {
        "name": "The Klang Suite",
        "emoji": "🎹",
        "desc": "Audio Plugins & Native C++ DSP",
        "color": "#0284c7",
        "bg": "rgba(2, 132, 199, 0.15)",
        "border": "rgba(2, 132, 199, 0.35)",
    },
    "TT": {
        "name": "ToadTracker",
        "emoji": "🐸",
        "desc": "Tracker Engine & M8 Bridge",
        "color": "#10b981",
        "bg": "rgba(16, 185, 129, 0.15)",
        "border": "rgba(16, 185, 129, 0.35)",
    },
    "NK": {
        "name": "N'kai",
        "emoji": "👁️",
        "desc": "Sidecar & Triage Lab",
        "color": "#8b5cf6",
        "bg": "rgba(139, 92, 246, 0.15)",
        "border": "rgba(139, 92, 246, 0.35)",
    },
    "TKR": {
        "name": "TheKlangResearch",
        "emoji": "🔬",
        "desc": "RFCs & DSP Math",
        "color": "#f59e0b",
        "bg": "rgba(245, 158, 11, 0.15)",
        "border": "rgba(245, 158, 11, 0.35)",
    },
    "AGY": {
        "name": "Antigravity",
        "emoji": "🧠",
        "desc": "Skills, Harness & Rules",
        "color": "#f43f5e",
        "bg": "rgba(244, 63, 94, 0.15)",
        "border": "rgba(244, 63, 94, 0.35)",
    },
    "TBD-16": {
        "name": "TBD-16 Hardware",
        "emoji": "🎛️",
        "desc": "Microcontroller Firmware & Embedded Hardware",
        "color": "#ea580c",
        "bg": "rgba(234, 88, 12, 0.15)",
        "border": "rgba(234, 88, 12, 0.35)",
    },
}


def infer_pillars(title: str, tags: Optional[List[str]] = None, body: str = "") -> List[str]:
    """
    Infers target ecosystem pillars (TKS, TT, NK, TKR, AGY, TBD-16) from note title,
    tags, and body keywords using heuristic matching.
    """
    text = f"{title} {' '.join(tags or [])} {body}".lower()
    matched: List[str] = []

    # 1. Direct tag & title bracket checks (e.g. "[TKS]", "#tt", "[NK]")
    tag_set = {t.lower().lstrip("#") for t in (tags or [])}

    bracket_m = re.findall(r"\[([a-zA-Z0-9_\-]+)\]", title)
    for b in bracket_m:
        b_upper = b.upper()
        if b_upper in PILLAR_METADATA and b_upper not in matched:
            matched.append(b_upper)

    if any(t in tag_set for t in ["tks", "theklangsuite", "klang"]) and "TKS" not in matched:
        matched.append("TKS")
    if any(t in tag_set for t in ["tt", "toadtracker", "tracker"]) and "TT" not in matched:
        matched.append("TT")
    if any(t in tag_set for t in ["nk", "nkai", "sidecar"]) and "NK" not in matched:
        matched.append("NK")
    if any(t in tag_set for t in ["tkr", "theklangresearch", "research"]) and "TKR" not in matched:
        matched.append("TKR")
    if any(t in tag_set for t in ["agy", "antigravity"]) and "AGY" not in matched:
        matched.append("AGY")
    if any(t in tag_set for t in ["tbd-16", "tbd16", "hardware", "firmware"]) and "TBD-16" not in matched:
        matched.append("TBD-16")

    # 2. Targeted Keyword rules
    keywords = {
        "TKS": [
            "vst", "vst3", "juce", "synth", "synthesizer", "filter", "oscillator",
            "preset", "patch", "farmer", "planter", "modulator", "apvts", "drum machine",
            "distortion", "chorus", "delay", "reverb", "audio thread", "fastmath"
        ],
        "TT": [
            "toadtracker", "toadcore", "m8", "dirtywave", "tracker", "sequencer",
            "pattern", "song mode", "sample playback", "headless"
        ],
        "NK": [
            "nkai", "sidecar", "triage", "canvas", "webview", "dossier", "mailbag",
            "action-chip", "two-tier header", "scoreboard", "haptic", "inbox", "quick note", "quick-note"
        ],
        "TKR": [
            "rfc", "dsp paper", "math proof", "autopsy", "benchmark", "tradeoff",
            "differential equation", "transfer function", "saturation model", "audit"
        ],
        "AGY": [
            "antigravity", "subagent", "gemini.md", "prompt compiler", "allowance",
            "token budget", "slash command", "skill", "orchestrator", "opus", "pro 3.1",
            "guardrail", "guardrails", "frontmatter", "front matter", "template"
        ],
        "TBD-16": [
            "esp32", "p4", "i2s", "codec", "rotary encoder", "teensy", "firmware",
            "gpio", "eurorack", "pcb", "tbd-16"
        ],
    }

    for pillar, kws in keywords.items():
        if pillar not in matched:
            for kw in kws:
                if re.search(r"\b" + re.escape(kw) + r"\b", text):
                    matched.append(pillar)
                    break

    # If nothing matched, default to TKS
    if not matched:
        matched.append("TKS")

    return matched


class PriorityLevel(str, Enum):
    P0 = "P0"
    P1 = "P1"
    P2 = "P2"
    P3 = "P3"


class TriageStatus(str, Enum):
    PENDING = "PENDING"
    APPROVED = "APPROVED"
    TABLED = "TABLED"
    KILLED = "KILLED"
    BACKLOG = "BACKLOG"
    COMPLETED = "COMPLETED"
    RESEARCH = "RESEARCH"
    GRAVEYARD = "GRAVEYARD"
    PCDA = "PCDA"


class TriageActionType(str, Enum):
    BACKLOG = "BACKLOG"
    RESEARCH = "RESEARCH"
    GRAVEYARD = "GRAVEYARD"
    PCDA = "PCDA"
    APPROVE = "APPROVE"
    TABLE = "TABLE"
    KILL = "KILL"


class DossierDrawer(BaseModel):
    model_config = ConfigDict(extra="allow")

    source_note: Optional[str] = Field(default=None, description="Path or filename of the source note")
    source_markdown: Optional[str] = Field(default="", description="Original raw markdown content from the note")
    pcda_dialectic: Optional[Dict[str, Any]] = Field(default_factory=dict, description="PCDA 4-part triad & 4-lens metrics")
    research_dossier: Optional[Dict[str, Any]] = Field(default_factory=dict, description="Citations, benchmarks, and precedents")
    pass_count: int = Field(default=2, description="Number of iterative research passes executed (strictly <= 2)")
    researched_at: Optional[str] = Field(default=None, description="ISO timestamp of research completion")


class MailItem(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str
    title: str
    created_at: str = Field(default_factory=lambda: datetime.now().strftime("%Y-%m-%d"))
    priority: PriorityLevel = PriorityLevel.P2
    status: TriageStatus = TriageStatus.PENDING
    tags: List[str] = Field(default_factory=list)
    target_pillars: List[str] = Field(default_factory=list)
    source_file: str = ""
    preview: str = ""
    dependencies: List[str] = Field(default_factory=list)
    verdict: Optional[str] = None
    verdict_notes: Optional[str] = None
    estimated_tokens: int = 10000

    # Additional attributes for rich triage metadata
    char_count: int = 0
    word_count: int = 0
    tier: str = "tier2"
    body: str = ""
    excerpt: str = ""
    file_name: str = ""
    rel_path: str = ""
    source_note: str = ""
    idx: int = 0
    merged_count: int = 0
    source_ids: List[str] = Field(default_factory=list)
    split_group: str = ""
    frontmatter: Dict[str, Any] = Field(default_factory=dict)

    # Per-Card Response Drawer (Flight 19 / rj-08)
    agent_reply: Optional[str] = None
    agent_reply_at: Optional[str] = None
    has_reply: bool = False

    # Multi-Drawer Dossier (Flight 3 / Topic 17 / rj-25)
    dossier: Optional[DossierDrawer] = None

    @property
    def pillars(self) -> List[str]:
        return self.target_pillars

    @pillars.setter
    def pillars(self, val: List[str]):
        self.target_pillars = val

    @model_validator(mode="before")
    @classmethod
    def sync_pillars_input(cls, data: Any) -> Any:
        if isinstance(data, dict):
            if "pillars" in data and not data.get("target_pillars"):
                data["target_pillars"] = data["pillars"]
            elif "target_pillars" in data and "pillars" not in data:
                data["pillars"] = data["target_pillars"]
        return data

    @model_validator(mode="after")
    def sync_has_reply(self) -> "MailItem":
        if self.agent_reply and self.agent_reply.strip():
            self.has_reply = True
        elif not self.agent_reply:
            self.has_reply = False
        return self

    @field_validator("priority", mode="before")
    @classmethod
    def normalize_priority(cls, v: Any) -> PriorityLevel:
        if isinstance(v, PriorityLevel):
            return v
        if isinstance(v, str):
            v_upper = v.strip().upper()
            if v_upper in PriorityLevel._value2member_map_:
                return PriorityLevel(v_upper)
        return PriorityLevel.P2

    @field_validator("status", mode="before")
    @classmethod
    def normalize_status(cls, v: Any) -> TriageStatus:
        if isinstance(v, TriageStatus):
            return v
        if isinstance(v, str):
            v_upper = v.strip().upper()
            if v_upper in TriageStatus._value2member_map_:
                return TriageStatus(v_upper)
        return TriageStatus.PENDING

    @field_validator("target_pillars", mode="before")
    @classmethod
    def normalize_target_pillars(cls, v: Any) -> List[str]:
        if isinstance(v, str):
            return [v.strip().upper()] if v.strip() else []
        if isinstance(v, (list, tuple, set)):
            res = []
            for p in v:
                p_str = str(p).strip().upper()
                if p_str and p_str not in res:
                    res.append(p_str)
            return res
        return []


class MailQueue(BaseModel):
    model_config = ConfigDict(extra="allow")

    schema_version: str = "1.0.0"
    last_scanned: str = Field(default_factory=lambda: datetime.now().isoformat())
    total_items: int = 0
    p0_count: int = 0
    items: List[MailItem] = Field(default_factory=list)

    def sort_by_priority(self) -> "MailQueue":
        """
        Sorts self.items dynamically by (priority_weight, status_weight, date_desc).
        All P0 items are placed first with untriaged/pending items at the top of each priority band.
        Dynamically assigns contiguous 1-based index (idx = 1..N).
        Updates self.total_items and self.p0_count.
        """
        priority_rank = {
            PriorityLevel.P0: 0,
            PriorityLevel.P1: 1,
            PriorityLevel.P2: 2,
            PriorityLevel.P3: 3,
        }
        status_rank = {
            TriageStatus.PENDING: 0,
            TriageStatus.RESEARCH: 1,
            TriageStatus.BACKLOG: 2,
            TriageStatus.COMPLETED: 3,
            TriageStatus.GRAVEYARD: 4,
        }

        # Python's timsort is stable:
        # 1. Date descending:
        self.items.sort(key=lambda x: str(x.created_at or ""), reverse=True)
        # 2. Status rank ascending (PENDING before COMPLETED):
        self.items.sort(key=lambda x: status_rank.get(x.status, 0))
        # 3. Priority rank ascending (P0 before P1 before P2):
        self.items.sort(key=lambda x: priority_rank.get(x.priority, 2))

        # Dynamically assign contiguous 1-based display index
        for i, it in enumerate(self.items, 1):
            it.idx = i

        self.total_items = len(self.items)
        self.p0_count = sum(1 for it in self.items if it.priority == PriorityLevel.P0)
        return self


class TransactionReceipt(BaseModel):
    model_config = ConfigDict(extra="allow")

    tx_id: str
    timestamp: str = Field(default_factory=lambda: datetime.now().isoformat())
    action: str
    note_id: str
    prev_status: TriageStatus
    new_status: TriageStatus
    ledger_snapshot_path: str = ""
    can_rollback: bool = True


# ==============================================================================
# TKPL: The Klang Preset Linter Models (Flight 6)
# ==============================================================================


class PresetMetadata(BaseModel):
    model_config = ConfigDict(extra="allow")

    name: str = Field(min_length=1, max_length=64, description="Human-readable name of the preset")
    author: str = Field(default="Cody Gratner", description="Creator or author of the preset")
    category: str = Field(description="Sonic classification (e.g. Bass, Lead, Pad, Kick, Percussion, FX)")
    tags: List[str] = Field(default_factory=list, description="Searchable descriptive tags")
    theme_hint: Optional[str] = Field(default=None, description="Optional UI theme identifier to pair with preset")


class PresetPayload(BaseModel):
    model_config = ConfigDict(extra="allow")

    schema_version: str = Field(default="1.0.0", description="SemVer version of the preset schema")
    target_plugin: str = Field(pattern="^(TKF|TKP|TKM|TKR)$", description="Target plugin identifier (TKF=Farmer, TKP=Planter, TKM=Modulator, TKR=Research)")
    metadata: PresetMetadata = Field(description="Preset metadata block")
    parameters: Dict[str, float] = Field(description="Normalized APVTS parameter float values [0.0, 1.0]")

    @field_validator("parameters")
    @classmethod
    def validate_parameter_ranges(cls, v: Dict[str, float]) -> Dict[str, float]:
        for param_id, val in v.items():
            if not isinstance(val, (int, float)):
                raise ValueError(f"Parameter '{param_id}' value must be a number, got {type(val).__name__}")
            if val < 0.0 or val > 1.0:
                raise ValueError(f"Parameter '{param_id}' value {val} is outside normalized range [0.0, 1.0]")
        return v


# ==============================================================================
# Ramble Ingestion Engine Models (Flight 1 / Topic 2)
# ==============================================================================


class RambleCard(BaseModel):
    model_config = ConfigDict(extra="allow")

    source_note: str = Field(description="Originating note path or filename")
    idea_index: int = Field(default=1, description="1-indexed idea index within note")
    title: str = Field(max_length=64, description="Crisp synthesized title (max 64 chars)")
    core_intent: str = Field(description="1-sentence summary of the creative goal")
    proposed_mechanism: str = Field(description="Affected repo, module, or hardware target")
    action_route: TriageActionType = Field(default=TriageActionType.BACKLOG, description="Suggested triage routing")
    target_pillar: str = Field(default="TKS", description="Target ecosystem pillar (TKS, TT, NK, TKR, TBD-16, AGY)")
    tags: List[str] = Field(default_factory=list, description="Descriptive or domain tags")
    raw_snippet: str = Field(default="", description="Supporting quote or excerpt from source note")
    confidence_score: float = Field(default=0.85, description="Heuristic parsing confidence [0.0, 1.0]")
    split_group: str = Field(default="", description="Parent note identifier or stem for overflow grouping")
    agent_reply: Optional[str] = Field(default=None, description="Optional markdown body of agent response")
    agent_reply_at: Optional[str] = Field(default=None, description="ISO timestamp of reply")

    def to_mail_item(self) -> MailItem:
        import re
        from pathlib import Path

        # Sanitize source identifier for ID
        stem = Path(self.source_note).stem if self.source_note else "note"
        clean_stem = re.sub(r"[^a-zA-Z0-9_\-]", "", stem.lower().replace(" ", "_")) or "ramble"
        item_id = f"ramble-{clean_stem}-{self.idea_index}"

        # Clean tags
        clean_tags = ["ramble", self.target_pillar.lower()]
        for t in self.tags:
            t_clean = t.lstrip("#").strip().lower()
            if t_clean and t_clean not in clean_tags:
                clean_tags.append(t_clean)

        # Detect priority from tags or title
        priority = PriorityLevel.P2
        if any(p in clean_tags for p in ["p0", "highest-priority", "top-priority"]):
            priority = PriorityLevel.P0
        elif any(p in clean_tags for p in ["p1", "high-priority"]):
            priority = PriorityLevel.P1
        elif any(p in clean_tags for p in ["p3", "low-priority"]):
            priority = PriorityLevel.P3

        # Model tier heuristic
        if self.target_pillar.upper() in ["TKS", "TT"] and any(k in self.title.lower() for k in ["dsp", "filter", "polyphon", "thread", "hal"]):
            tier = "tier1"
        elif any(k in self.title.lower() for k in ["doc", "catalog", "lore"]):
            tier = "tier3"
        else:
            tier = "tier2"

        route_emoji = {
            TriageActionType.BACKLOG: "📥 Backlog",
            TriageActionType.RESEARCH: "🔬 Research",
            TriageActionType.GRAVEYARD: "🪦 Graveyard",
            TriageActionType.PCDA: "⚖️ PCDA",
        }.get(self.action_route, "📥 Backlog")

        body_lines = [
            f"### 🎙️ [{self.target_pillar}] {self.title}",
            f"- 🎯 **Core Intent**: {self.core_intent}",
            f"- 🛠️ **Proposed Mechanism**: {self.proposed_mechanism}",
            f"- 🚦 **Action Route**: `[ {route_emoji} ]` (Confidence: {self.confidence_score:.2f})",
        ]
        if self.raw_snippet:
            snippet_cleaned = self.raw_snippet.strip()
            body_lines.append("")
            body_lines.append("> **Source Excerpt:**")
            for s_line in snippet_cleaned.splitlines():
                body_lines.append(f"> {s_line}")

        body_text = "\n".join(body_lines)

        return MailItem(
            id=item_id,
            title=f"🎙️ [{self.target_pillar}] {self.title}",
            created_at=datetime.now().strftime("%Y-%m-%d"),
            priority=priority,
            status=TriageStatus.PENDING,
            tags=clean_tags,
            target_pillars=[self.target_pillar],
            source_file=self.source_note,
            source_note=self.source_note,
            file_name=Path(self.source_note).name if self.source_note else "",
            rel_path=self.source_note,
            preview=self.core_intent,
            excerpt=self.core_intent,
            body=body_text,
            char_count=len(body_text),
            word_count=len(body_text.split()),
            tier=tier,
            idx=self.idea_index,
            split_group=self.split_group,
            agent_reply=self.agent_reply,
            agent_reply_at=self.agent_reply_at,
            has_reply=bool(self.agent_reply and self.agent_reply.strip()),
        )


class RambleExtractionResult(BaseModel):
    model_config = ConfigDict(extra="allow")

    source_file: str = Field(description="Path to processed file")
    extracted_at: str = Field(default_factory=lambda: datetime.now().isoformat(), description="ISO timestamp")
    total_ideas: int = Field(default=0, description="Total extracted RambleCards")
    cards: List[RambleCard] = Field(default_factory=list, description="Extracted cards")


# ==============================================================================
# Declarative Board Models (Flight 15 / Topic 6.11 / rj-04)
# ==============================================================================


class HeroStageItem(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str = Field(description="Unique DOM ID for hero stage item")
    title: str = Field(description="Hero card title")
    subtitle: str = Field(default="", description="Secondary subtitle or context")
    badge: str = Field(default="", description="Badge text or emoji")
    content_html: str = Field(default="", description="Rendered HTML content")
    body_html: str = Field(default="", description="Alias for content_html")
    context_target: Optional[str] = Field(default=None, description="Jump target card ID or section anchor")

    def model_post_init(self, __context: Any) -> None:
        if not self.content_html and self.body_html:
            self.content_html = self.body_html
        elif not self.body_html and self.content_html:
            self.body_html = self.content_html


class FilterRailItem(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str = Field(description="Deterministic element ID e.g. pill-pillar-tks or pill-status-p0")
    label: str = Field(description="Display label e.g. TKS or Top Priority")
    icon: str = Field(default="🏷️", description="Emoji or SVG icon")
    count: int = Field(default=0, description="Item counter")
    filter_key: str = Field(description="Filter value matching card attribute")
    filter_type: str = Field(default="status", description="Type of rail filter (pillar | status | custom)")
    active: bool = Field(default=False, description="Whether currently active by default")


class FilterRail(BaseModel):
    model_config = ConfigDict(extra="allow")

    rail_id: str = Field(description="Identifier for rail (e.g. rail-pillars, rail-status)")
    label: str = Field(description="Human-readable rail label")
    items: List[FilterRailItem] = Field(default_factory=list, description="Filter items inside this rail")


class BoardActionDefinition(BaseModel):
    model_config = ConfigDict(extra="allow")

    action_id: str = Field(description="Action identifier (e.g. approve, backlog, research, graveyard, pcda)")
    label: str = Field(description="Button display label")
    icon: str = Field(description="Button emoji or icon")
    color_var: str = Field(default="var(--accent-blue)", description="Theme color variable or CSS hex")
    prompt_template: str = Field(default="", description="Prompt template dispatched to subagent or agent")
    shortcut: str = Field(default="", description="Keyboard shortcut hint")


class BoardSchema(BaseModel):
    model_config = ConfigDict(extra="allow")

    schema_version: str = Field(default="1.0.0", description="Board schema SemVer")
    board_id: str = Field(default="morning_mail", description="Board slug e.g. morning_mail, sandbox_mail, deep_research")
    title: str = Field(description="Board title in header")
    subtitle: str = Field(default="", description="Optional header subtitle or scope")
    environment: str = Field(default="PRODUCTION", description="Environment mode (PRODUCTION or SANDBOX)")
    board_type: str = Field(default="custom", description="Board surface type ('evergreen' | 'lab' | 'custom')")
    has_hero_stage: bool = Field(default=False, description="Whether hero showcase panel is enabled")
    hero_items: List[HeroStageItem] = Field(default_factory=list, description="Hero stage items")
    filter_rails: List[FilterRail] = Field(default_factory=list, description="Filter rails (Rail 1, Rail 2, etc.)")
    available_actions: List[BoardActionDefinition] = Field(default_factory=list, description="Card action button definitions")
    cards: List[MailItem] = Field(default_factory=list, description="Card item stream")
    allowance_budget: int = Field(default=200000, description="Token allowance budget ceiling for meter")

    @classmethod
    def export_schema_to_file(cls, path: Path) -> Path:
        import json
        path.parent.mkdir(parents=True, exist_ok=True)
        schema = cls.model_json_schema()
        path.write_text(json.dumps(schema, indent=2) + "\n", encoding="utf-8")
        return path


# ==============================================================================
# Multi-Board Hub & Registry Models (Flight 14 / Topic 6.11 / rj-05)
# ==============================================================================


class BoardMetadata(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str = Field(description="Deterministic board identifier e.g. morning_mail, sandbox_mail")
    title: str = Field(description="Human-readable title of the board")
    description: str = Field(default="", description="Summary of board purpose")
    category: str = Field(default="active", description="Board category ('active' | 'staging' | 'archived' | 'sandbox')")
    board_type: str = Field(default="custom", description="Board surface type ('evergreen' | 'lab' | 'custom')")
    file_path: str = Field(description="Relative or canonical path to HTML file e.g. .agents/sidecar/morning_mail_inbox.html")
    environment: str = Field(default="PRODUCTION", description="Runtime environment ('PRODUCTION' | 'SANDBOX')")
    card_count: int = Field(default=0, description="Total cards or items on the board")
    updated_at: str = Field(default_factory=lambda: datetime.now().isoformat(), description="ISO timestamp of last update")


class BoardRegistry(BaseModel):
    model_config = ConfigDict(extra="allow")

    version: str = Field(default="1.0.0", description="Board registry schema SemVer")
    default_board_id: str = Field(default="morning_mail", description="Default board ID to load")
    active_lab_id: Optional[str] = Field(default=None, description="Currently active ephemeral lab board ID")
    staged_labs: List[str] = Field(default_factory=list, description="List of completed lab board IDs staged for distillation")
    boards: List[BoardMetadata] = Field(default_factory=list, description="Registered boards list")

    def get_board(self, board_id: str) -> Optional[BoardMetadata]:
        for b in self.boards:
            if b.id == board_id:
                return b
        return None

    def active_boards(self) -> List[BoardMetadata]:
        return [b for b in self.boards if b.category.lower() == "active" and b.environment.upper() != "SANDBOX"]

    def staging_boards(self) -> List[BoardMetadata]:
        return [b for b in self.boards if b.category.lower() == "staging" and b.environment.upper() != "SANDBOX"]

    def archived_boards(self) -> List[BoardMetadata]:
        return [b for b in self.boards if b.category.lower() == "archived" and b.environment.upper() != "SANDBOX"]

    def sandbox_boards(self) -> List[BoardMetadata]:
        return [b for b in self.boards if b.category.lower() == "sandbox" or b.environment.upper() == "SANDBOX"]


# ==============================================================================
# Prompt Compiler & Dialectical Handoff Models (Flight 2 / Topic 16 / rj-24)
# ==============================================================================


class ThesisBlock(BaseModel):
    model_config = ConfigDict(extra="allow")

    core_intent: str = Field(description="Primary user objective or architectural thesis")
    user_goals: List[str] = Field(default_factory=list, description="Explicit bulleted user goals")
    source_note: Optional[str] = Field(default=None, description="Path or title of the original source note")


class AntithesisBlock(BaseModel):
    model_config = ConfigDict(extra="allow")

    reviewer_persona: str = Field(
        default="The Dissolution / Adversarial Auditor",
        description="Persona framing for adversarial stress-testing",
    )
    echo_chamber_shatter: str = Field(
        default="Direct confrontation with real-world implementation traps",
        description="Friction summary breaking confirmation bias",
    )
    structural_vulnerabilities: List[str] = Field(
        default_factory=list,
        description="Identified risks, traps, or failure modes",
    )
    orthogonal_vectors: List[str] = Field(
        default_factory=list,
        description="Constructive lateral alternatives and architectural resolutions",
    )


class SynthesisTargetConfig(BaseModel):
    model_config = ConfigDict(extra="allow")

    materialization: str = Field(
        default="Python Tooling / Antigravity Sidecar Component",
        description="Concrete materialization target (e.g., C++ JUCE DSP, Sidecar Component, CLI Tool)",
    )
    the_blueprint: str = Field(
        default="Definitive drop-in implementation plan adhering to Sleepy Architecture",
        description="Expected deliverable structure and format",
    )
    effort_level: str = Field(default="MINIMAL", description="Expected coding effort level (MINIMAL / MODERATE)")
    impact_level: str = Field(default="MAXIMUM", description="Expected feature leverage (MAXIMUM / HIGH)")
    status: str = Field(default="PENDING_REASONING", description="Clearance status")


class PromptHandoffPayload(BaseModel):
    model_config = ConfigDict(extra="allow")

    thread_id: str = Field(description="Unique session or topic identifier")
    reconciliation_protocol: str = Field(
        default="Standard Dialectical (Method B+)",
        description="Protocol directing model reasoning",
    )
    thesis: ThesisBlock = Field(description="Core intent and goals")
    antithesis: AntithesisBlock = Field(default_factory=AntithesisBlock, description="Adversarial stress-test")
    targets: SynthesisTargetConfig = Field(default_factory=SynthesisTargetConfig, description="Target constraints")
    prompt_mandate: str = Field(
        default=(
            "Analyze the structural tension between my core intent (Thesis) and the subagent's counterarguments (Antithesis). "
            "Using your extended deep-thinking reasoning mode, reconcile these perspectives into a definitive software architecture spec. "
            "Ensure the final blueprint strictly optimizes for the Sleepy Architecture principles: achieve MAXIMUM feature leverage with MINIMAL coding execution overhead."
        ),
        description="Command directive for the flagship reasoning model",
    )

    def to_markdown(self) -> str:
        lines = []
        lines.append("# SYSTEM HANDOFF: DIALECTICAL SYNTHESIS ENGINE")
        lines.append(f"**Thread Identity:** `{self.thread_id}`")
        lines.append(f"**Reconciliation Protocol:** `{self.reconciliation_protocol}`\n")

        lines.append("## [THESIS: CORE INTENT]")
        if self.thesis.source_note:
            lines.append(f"*Source Note:* `{self.thesis.source_note}`\n")
        lines.append(self.thesis.core_intent)
        if self.thesis.user_goals:
            lines.append("\n### Key Objectives")
            for goal in self.thesis.user_goals:
                lines.append(f"- {goal}")
        lines.append("")

        lines.append("## [ANTITHESIS: ADVERSARIAL STRESS-TEST]")
        lines.append(f"**Reviewer Persona:** {self.antithesis.reviewer_persona}")
        lines.append(f"**Shatter Metric:** {self.antithesis.echo_chamber_shatter}\n")

        lines.append("### Identified Structural Vulnerabilities")
        if self.antithesis.structural_vulnerabilities:
            for v in self.antithesis.structural_vulnerabilities:
                lines.append(f"- [!] {v}")
        else:
            lines.append("- No immediate vulnerabilities exposed.")
        lines.append("")

        lines.append("### Orthogonal Vectors & Alternatives")
        if self.antithesis.orthogonal_vectors:
            for vec in self.antithesis.orthogonal_vectors:
                lines.append(f"- [x] {vec}")
        else:
            lines.append("- No alternative architectural trajectories recommended.")
        lines.append("")

        lines.append("## [CONSTRAINTS & TARGETS]")
        lines.append(f"- **Materialization Target:** {self.targets.materialization}")
        lines.append(f"- **The Blueprint Context:** `{self.targets.the_blueprint}`")
        lines.append(f"- **Sleepy Architecture Metric:** Effort: `{self.targets.effort_level}` / Impact: `{self.targets.impact_level}`")
        lines.append(f"- **Mayor Clearance Status:** `{self.targets.status}`\n")

        lines.append("---")
        lines.append("## PROMPT / COMMAND TO EXECUTE")
        lines.append(self.prompt_mandate)

        return "\n".join(lines)

    def to_clipboard(self) -> bool:
        from tools.compile_prompt import copy_to_clipboard
        return copy_to_clipboard(self.to_markdown())

    def to_vault_file(self, target_path: Path) -> Path:
        target_path.parent.mkdir(parents=True, exist_ok=True)
        target_path.write_text(self.to_markdown(), encoding="utf-8")
        return target_path


# ==============================================================================
# Configurable Lens Profiles (Flight 4 / Topic 9)
# ==============================================================================


class LensDefinition(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str = Field(description="Unique identifier for the lens (e.g. ergonomics, signal_flow)")
    name: str = Field(description="Human-readable title of the evaluation lens")
    prompt: str = Field(description="Detailed evaluation instructions or guiding criteria")


class LensProfile(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str = Field(default="", description="Unique profile identifier (e.g. audio_dev, studio_hardware)")
    display_name: str = Field(description="Human-readable name of the profile")
    tags: List[str] = Field(default_factory=list, description="Descriptive tags used for automatic profile detection")
    lenses: List[LensDefinition] = Field(description="List of 4 specialized evaluation lenses")


class LensProfilesConfig(BaseModel):
    model_config = ConfigDict(extra="allow")

    version: str = Field(default="1.0.0", description="SemVer version of the lens profiles configuration")
    default_profile: str = Field(default="audio_dev", description="Default profile id if no tags match")
    profiles: Dict[str, LensProfile] = Field(default_factory=dict, description="Dictionary of available lens profiles")


DEFAULT_LENS_PROFILES_PATH = Path(__file__).resolve().parent / "lens_profiles.json"


def load_lens_profiles(config_path: Optional[Path] = None) -> LensProfilesConfig:
    """
    Loads and validates the declarative lens profiles configuration from JSON.
    Falls back to embedded defaults if file is missing or invalid.
    """
    target = config_path or DEFAULT_LENS_PROFILES_PATH
    if target.exists():
        try:
            content = target.read_text(encoding="utf-8", errors="replace")
            data = json.loads(content)
            # Ensure profile id matches dict key if missing
            if "profiles" in data and isinstance(data["profiles"], dict):
                for k, v in data["profiles"].items():
                    if isinstance(v, dict) and "id" not in v:
                        v["id"] = k
            return LensProfilesConfig.model_validate(data)
        except Exception as e:
            print(f"⚠️ Warning: Could not parse lens profiles from {target}: {e}")

    # Fallback default configuration
    return LensProfilesConfig(
        version="1.0.0",
        default_profile="audio_dev",
        profiles={
            "audio_dev": LensProfile(
                id="audio_dev",
                display_name="Audio Plugin & DSP Engineering",
                tags=["audio", "dsp", "vst", "synth", "juce", "tks"],
                lenses=[
                    LensDefinition(id="ergonomics", name="Real-World / Daily Ergonomics", prompt="Practical day-to-day developer friction, velocity, cognitive load, and maintenance realities."),
                    LensDefinition(id="industry", name="Audio Software Industry", prompt="How top synth/plugin developers (Vital, Kilohearts, FabFilter, Elektron, Bitwig, Reaper) solve it."),
                    LensDefinition(id="sota", name="Wider Software Industry / SOTA", prompt="Modern systems, agent harnesses, and web architectures (Anthropic, OpenAI, GitHub, Linux)."),
                    LensDefinition(id="best_practices", name="Best Practices & Sustainability", prompt="Audio thread safety, zero-allocations, token economics, and avoiding premature optimization."),
                ],
            )
        },
    )


def detect_lens_profile(
    tags: Optional[List[str]] = None,
    text: str = "",
    config: Optional[LensProfilesConfig] = None,
) -> str:
    """
    Auto-detects the optimal lens profile based on tags and text keywords.
    - Tags containing studio, hardware, eurorack, wiring, m8, tbd-16 -> studio_hardware
    - Tags containing ttrpg, writing, lyrics, gamedesign, creative -> creative_general
    - Default: audio_dev
    """
    cfg = config or load_lens_profiles()
    tag_set = {str(t).lower().lstrip("#").strip() for t in (tags or [])}

    # Also extract inline hashtag markers from text (e.g. #hardware, #ttrpg)
    if text:
        inline_tags = re.findall(r"#([a-zA-Z0-9_\-]+)", text.lower())
        tag_set.update(inline_tags)

    # 1. Check profiles in config for tag overlap
    for profile_id, profile in cfg.profiles.items():
        if profile_id == cfg.default_profile:
            continue
        p_tags = {t.lower() for t in profile.tags}
        if tag_set.intersection(p_tags):
            return profile_id

    # 2. Text keyword scan if tags didn't match non-default
    text_lower = text.lower()
    for profile_id, profile in cfg.profiles.items():
        if profile_id == cfg.default_profile:
            continue
        for t in profile.tags:
            if t in text_lower:
                return profile_id

    return cfg.default_profile



