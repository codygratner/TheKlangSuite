#!/usr/bin/env python3
"""
Pydantic Data Models for Mailbag, Triage Ledger, and Vault Studio Hub (Topic 6.11 / Flight 5)
Provides schema validation, priority queue sorting, and atomic transaction receipts.
"""

from datetime import datetime
from enum import Enum
from typing import Any, Dict, List, Optional
from pydantic import BaseModel, ConfigDict, Field, field_validator


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


class MailQueue(BaseModel):
    model_config = ConfigDict(extra="allow")

    schema_version: str = "1.0.0"
    last_scanned: str = Field(default_factory=lambda: datetime.now().isoformat())
    total_items: int = 0
    p0_count: int = 0
    items: List[MailItem] = Field(default_factory=list)

    def sort_by_priority(self) -> "MailQueue":
        """
        Sorts self.items so that all P0 items are placed first (sorted in reverse-chronological
        order by date, newest first), followed by P1, P2, P3 (also reverse-chronological).
        Updates self.total_items and self.p0_count.
        """
        priority_rank = {
            PriorityLevel.P0: 0,
            PriorityLevel.P1: 1,
            PriorityLevel.P2: 2,
            PriorityLevel.P3: 3,
        }

        # Python's timsort is stable:
        # First sort by date descending:
        self.items.sort(key=lambda x: str(x.created_at or ""), reverse=True)
        # Then sort by rank ascending (stable preserve date descending within rank):
        self.items.sort(key=lambda x: priority_rank.get(x.priority, 2))

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

