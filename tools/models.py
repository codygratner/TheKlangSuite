#!/usr/bin/env python3
"""
Pydantic Data Models for Mailbag, Triage Ledger, and Vault Studio Hub (Topic 6.11 / Flight 5)
Provides schema validation, priority queue sorting, and atomic transaction receipts.
"""

from datetime import datetime
from enum import Enum
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
    category: str = Field(default="active", description="Board category ('active' | 'archived')")
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
        return [b for b in self.boards if b.category.lower() == "active"]

    def archived_boards(self) -> List[BoardMetadata]:
        return [b for b in self.boards if b.category.lower() == "archived"]


