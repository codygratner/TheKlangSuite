#!/usr/bin/env python3
"""
tools/agy_models.py — Pydantic v2 Contract Layer for Antigravity Bridge
========================================================================
Provides Rust-backed schema validation, forward-compatible field absorption,
and dual-case serialization (snake_case in Python <-> camelCase for ConnectRPC).
"""

from typing import List, Optional
from pydantic import BaseModel, ConfigDict, Field, AliasChoices


class BaseAgyModel(BaseModel):
    """Base configuration providing forward-compatibility and dual-case aliasing."""
    model_config = ConfigDict(
        extra="ignore",          # Immune to upstream Google updates adding new fields
        populate_by_name=True,   # Accepts both snake_case and camelCase
        str_strip_whitespace=True,
    )


class QuotaBucket(BaseAgyModel):
    """Model allowance metrics normalizing legacy and canonical ConnectRPC keys."""
    g_5h: float = Field(default=100.0, validation_alias=AliasChoices("g_5h", "gemini_5h_percent"))
    g_wk: float = Field(default=100.0, validation_alias=AliasChoices("g_wk", "gemini_weekly_percent"))
    c_5h: float = Field(default=100.0, validation_alias=AliasChoices("c_5h", "claude_5h_percent"))
    c_wk: float = Field(default=100.0, validation_alias=AliasChoices("c_wk", "claude_weekly_percent"))
    g_reset_5h_human: str = ""
    g_reset_wk_human: str = ""
    claude_active: bool = False


class AgyConversation(BaseAgyModel):
    """Sovereign conversation summary normalized from SQLite and app_storage.json."""
    id: str = Field(..., validation_alias=AliasChoices("id", "conversation_id", "cascade_id"))
    title: str = Field(default="Untitled Chat")
    is_current: bool = False
    is_pinned: bool = False
    is_astral_abyss: bool = False
    status: str = Field(default="IDLE")
    last_active: str = Field(default="", validation_alias=AliasChoices("last_active", "last_modified_time"))
    step_count: int = Field(default=0, validation_alias=AliasChoices("step_count", "numSteps"))
    project_id: str = Field(default="", validation_alias=AliasChoices("project_id", "projectId"))


class TextOrScopeItem(BaseAgyModel):
    text: str


class SendCascadeRequest(BaseAgyModel):
    """Payload sent to language_server.exe SendUserCascadeMessage."""
    cascade_id: str = Field(..., serialization_alias="cascadeId", validation_alias=AliasChoices("cascade_id", "cascadeId"))
    items: List[TextOrScopeItem]
    message_text: Optional[str] = Field(default=None, serialization_alias="messageText")


class TrajectoryTurn(BaseAgyModel):
    """Normalized chat turn for 1:1 rendering in cockpit.html#chat."""
    turn_index: int
    sender: str  # "user" | "assistant" | "system"
    text: str
    thinking_text: str = ""
    thinking_duration_sec: float = 0.0
    status: str = "done"  # "thinking" | "streaming" | "done" | "error"
    created_at: str = ""
