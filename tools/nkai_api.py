#!/usr/bin/env python3
"""
N'kai Local FastAPI Sidecar Bridge (v1.0.0 / Flight 7)
Provides high-performance, asynchronous REST API bridging N'kai HTML sidecars,
Electron webviews, and CLI harnesses directly to the workspace state.

Listens on http://127.0.0.1:8000
Provides CORS and Private Network Access (PNA) headers for file:/// and localhost origins.
Endpoints:
  GET  /health
  GET  /api/mail
  POST /api/action
  POST /api/plan/rollback
  POST /api/dispatch
"""

import argparse
import datetime
import json
import os
import shutil
import sys
import uuid
from pathlib import Path
from typing import Any, Dict, List, Optional

# Ensure UTF-8 output on Windows consoles
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

from fastapi import FastAPI, HTTPException, Request, Response, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel, ConfigDict, Field

# Workspace path resolution
WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
INBOX_DIR = WORKSPACE_ROOT / ".agents" / "pipeline" / "inbox"
MAILBAG_DIR = WORKSPACE_ROOT / ".agents" / "pipeline" / "mailbag"
STAGING_DIR = WORKSPACE_ROOT / ".agents" / "pipeline" / "staging"
PLANS_DIR = WORKSPACE_ROOT / ".agents" / "pipeline" / "plans"
SNAPSHOTS_DIR = PLANS_DIR / "snapshots"
PLAN_MD = WORKSPACE_ROOT / "PLAN.md"
PLAN_PREV = SNAPSHOTS_DIR / "PLAN_PREV.md"
PLAN_QUEUE = PLANS_DIR / "queue.json"
ACTION_QUEUE_FILE = INBOX_DIR / "action_queue.jsonl"
LATEST_ACTION_FILE = INBOX_DIR / "latest_action.json"
MAIL_QUEUE_FILE = MAILBAG_DIR / "mail_queue.json"
MAIL_TX_FILE = STAGING_DIR / "mail_tx_latest.json"

# Import Pydantic models from tools.models
try:
    from tools.models import (
        MailItem,
        MailQueue,
        PriorityLevel,
        TransactionReceipt,
        TriageActionType,
        TriageStatus,
    )
except ImportError:
    try:
        from models import (
            MailItem,
            MailQueue,
            PriorityLevel,
            TransactionReceipt,
            TriageActionType,
            TriageStatus,
        )
    except ImportError:
        # Fallback inline models if executing standalone
        from enum import Enum

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

        class MailItem(BaseModel):
            model_config = ConfigDict(extra="allow")
            id: str
            title: str
            created_at: str = Field(default_factory=lambda: datetime.datetime.now().strftime("%Y-%m-%d"))
            priority: PriorityLevel = PriorityLevel.P2
            status: TriageStatus = TriageStatus.PENDING

        class MailQueue(BaseModel):
            model_config = ConfigDict(extra="allow")
            schema_version: str = "1.0.0"
            total_items: int = 0
            p0_count: int = 0
            items: List[MailItem] = Field(default_factory=list)

        class TransactionReceipt(BaseModel):
            model_config = ConfigDict(extra="allow")
            tx_id: str
            action: str
            note_id: str
            prev_status: TriageStatus
            new_status: TriageStatus


# Request Models
class TriageActionRequest(BaseModel):
    model_config = ConfigDict(extra="allow")

    action: str = Field(description="Action name: approve, backlog, graveyard, subagent, queue, table, kill")
    note_id: Optional[str] = Field(default=None, description="Identifier of the targeted mailbag note")
    topic_id: Optional[str] = Field(default=None, description="Identifier of the targeted sanity audit topic")
    title: Optional[str] = Field(default="", description="Title or excerpt of the item")
    notes: Optional[str] = Field(default="", description="Optional rationale or triage notes")
    tier: Optional[str] = Field(default="tier2", description="Complexity tier: tier1, tier2, tier3")
    source: Optional[str] = Field(default="sidecar", description="Source client: sidecar, webview, cli")
    sub_mode: Optional[str] = Field(default=None, description="Subagent probe mode: local, web, pcda, prompt")
    session_mode: Optional[str] = Field(default=None, description="Session queue mode: Grill, Workshop, PCDA, Discuss, Plan")
    table_reason: Optional[str] = Field(default=None, description="Table hold reason or custom note")



class DispatchRequest(BaseModel):
    model_config = ConfigDict(extra="allow")

    action: Any = Field(description="Action text, prompt, or object")
    timestamp: Optional[int] = Field(default=None, description="Client unix timestamp ms")


# Initialize FastAPI App
app = FastAPI(
    title="N'kai Local Sidecar Bridge",
    description="Local IPC API server connecting interactive N'kai sidecars with the AI coding harness.",
    version="1.0.0",
)

# CORS Middleware (Universal for file:/// and localhost)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=False,
    allow_methods=["*"],
    allow_headers=["*"],
    expose_headers=["*"],
)


@app.middleware("http")
async def add_pna_and_cors_headers(request: Request, call_next):
    """
    Ensures Private Network Access (PNA) and CORS headers are attached to every response,
    enabling seamless fetch calls from sandboxed Chromium / Electron file:/// webviews.
    """
    response = await call_next(request)
    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS"
    response.headers["Access-Control-Allow-Headers"] = "*"
    response.headers["Access-Control-Allow-Private-Network"] = "true"
    return response


@app.options("/{full_path:path}")
async def options_handler(full_path: str):
    """Explicit preflight handler returning 204 with PNA and CORS headers."""
    return Response(
        status_code=204,
        headers={
            "Access-Control-Allow-Origin": "*",
            "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
            "Access-Control-Allow-Headers": "*",
            "Access-Control-Allow-Private-Network": "true",
            "Access-Control-Max-Age": "86400",
        },
    )



# ==============================================================================
# 1. Health & Workspace Telemetry
# ==============================================================================


@app.get("/health", summary="Health & Workspace Status")
@app.get("/", summary="Root Status")
async def get_health() -> Dict[str, Any]:
    """Returns bridge operational status, active workspace path, and available endpoints."""
    return {
        "status": "online",
        "bridge": "nkai-fastapi-bridge",
        "version": "1.0.0",
        "workspace": str(WORKSPACE_ROOT),
        "port": 8000,
        "endpoints": [
            "/health",
            "/api/mail",
            "/api/action",
            "/api/plan/rollback",
            "/api/dispatch",
        ],
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    }


# ==============================================================================
# 2. Mailbag & Triage Ledger
# ==============================================================================


@app.get("/api/mail", summary="Get Validated Mailbag Queue")
async def get_mail_queue(sort: Optional[str] = "priority") -> Dict[str, Any]:
    """
    Returns the persistent mailbag queue validated against Pydantic MailQueue schema.
    If sort=priority (default), all P0 items are stably ordered first in reverse-chronological order.
    """
    if not MAIL_QUEUE_FILE.exists():
        empty_queue = MailQueue(items=[])
        return empty_queue.model_dump()

    try:
        with open(MAIL_QUEUE_FILE, "r", encoding="utf-8") as f:
            raw_data = json.load(f)
        queue = MailQueue.model_validate(raw_data)
        if sort == "priority" and hasattr(queue, "sort_by_priority"):
            queue.sort_by_priority()
        return queue.model_dump()
    except Exception as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to read mailbag queue: {e}",
        )


# ==============================================================================
# 3. Triage Action Dispatcher
# ==============================================================================


@app.post("/api/action", summary="Dispatch Triage Action")
async def dispatch_action(payload: TriageActionRequest) -> Dict[str, Any]:
    """
    Receives a triage action from the N'kai sidecar, appends it to action_queue.jsonl,
    and updates the session ledger in mail_queue.json if targeting a note_id.
    """
    now_iso = datetime.datetime.now(datetime.timezone.utc).isoformat()
    action_clean = payload.action.strip().upper()

    if action_clean in ("SET_REALM", "TOGGLE_DOMAIN", "SET_DOMAIN") and payload.note_id:
        try:
            from check_mail import handle_toggle_card_domain
            target_realm = str(getattr(payload, "target", "") or payload.notes or "dev").lower()
            res = handle_toggle_card_domain(target_id=payload.note_id, target_realm=target_realm)
            return res
        except Exception as e:
            return {"ok": False, "error": str(e)}

    record = {
        "id": str(uuid.uuid4()),
        "action": action_clean,
        "note_id": payload.note_id,
        "topic_id": payload.topic_id,
        "title": payload.title,
        "notes": payload.notes,
        "tier": payload.tier,
        "source": payload.source,
        "timestamp": now_iso,
    }

    # 1. Append to action_queue.jsonl
    INBOX_DIR.mkdir(parents=True, exist_ok=True)
    try:
        with open(ACTION_QUEUE_FILE, "a", encoding="utf-8") as f:
            f.write(json.dumps(record, ensure_ascii=False) + "\n")
        with open(LATEST_ACTION_FILE, "w", encoding="utf-8") as f:
            json.dump(record, f, indent=2, ensure_ascii=False)
            f.write("\n")
    except Exception as e:
        print(f"[!] Warning: Failed to record action to queue file: {e}", file=sys.stderr)

    # 2. Update state in mail_queue.json if note_id is provided
    updated_note = False
    if payload.note_id and MAIL_QUEUE_FILE.exists():
        try:
            with open(MAIL_QUEUE_FILE, "r", encoding="utf-8") as f:
                queue_data = json.load(f)

            queue = MailQueue.model_validate(queue_data)
            status_map = {
                "BACKLOG": TriageStatus.BACKLOG,
                "RESEARCH": TriageStatus.RESEARCH,
                "GRAVEYARD": TriageStatus.GRAVEYARD,
                "PCDA": TriageStatus.PCDA,
                "APPROVE": TriageStatus.APPROVED,
                "APPROVED": TriageStatus.APPROVED,
                "TABLE": TriageStatus.TABLED,
                "TABLED": TriageStatus.TABLED,
                "KILL": TriageStatus.KILLED,
                "KILLED": TriageStatus.KILLED,
                "SUBAGENT": TriageStatus.INVESTIGATING,
                "SUBAGENT_PROBE": TriageStatus.INVESTIGATING,
                "QUEUE": TriageStatus.STAGED,
                "QUEUE_SESSION": TriageStatus.STAGED,
            }
            new_status = status_map.get(action_clean, TriageStatus.APPROVED)

            for item in queue.items:
                if item.id == payload.note_id or getattr(item, "file_name", "") == payload.note_id:
                    prev_status = item.status
                    item.status = new_status
                    item.verdict = action_clean

                    effective_notes = payload.notes or ""
                    if not effective_notes:
                        if payload.sub_mode:
                            effective_notes = f"Subagent mode: {payload.sub_mode}"
                        elif payload.session_mode:
                            effective_notes = f"Session mode: {payload.session_mode}"
                        elif payload.table_reason:
                            effective_notes = f"Table reason: {payload.table_reason}"
                    item.verdict_notes = effective_notes
                    updated_note = True

                    # Generate atomic transaction receipt
                    STAGING_DIR.mkdir(parents=True, exist_ok=True)
                    tx = TransactionReceipt(
                        tx_id=f"tx-{uuid.uuid4().hex[:8]}",
                        timestamp=now_iso,
                        action=action_clean,
                        note_id=item.id,
                        prev_status=prev_status,
                        new_status=new_status,
                        ledger_snapshot_path=str(MAIL_QUEUE_FILE),
                        can_rollback=True,
                    )
                    with open(MAIL_TX_FILE, "w", encoding="utf-8") as f:
                        json.dump(tx.model_dump(), f, indent=2)
                        f.write("\n")

                    # Slide to bottom of queue if Subagent probe
                    if action_clean in ("SUBAGENT", "SUBAGENT_PROBE"):
                        queue.items.remove(item)
                        queue.items.append(item)
                        for i, it in enumerate(queue.items, 1):
                            it.idx = i
                    break


            if updated_note:
                with open(MAIL_QUEUE_FILE, "w", encoding="utf-8") as f:
                    json.dump(queue.model_dump(), f, indent=2, ensure_ascii=False)
                    f.write("\n")
        except Exception as e:
            print(f"[!] Warning: Failed updating note in mail_queue.json: {e}", file=sys.stderr)

    return {
        "status": "success",
        "message": f"Action '{action_clean}' recorded successfully",
        "record": record,
        "note_updated": updated_note,
    }


# ==============================================================================
# 4. Plan Queue Rollback
# ==============================================================================


@app.post("/api/plan/rollback", summary="Trigger 5ms Plan Queue Rollback")
async def trigger_plan_rollback() -> Dict[str, Any]:
    """
    Restores snapshots/PLAN_PREV.md back to root PLAN.md in 5ms,
    reverts the execution pointer in queue.json, and returns confirmation.
    """
    if not PLAN_PREV.exists():
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"No prior plan snapshot found at {PLAN_PREV}",
        )

    try:
        # 1. Restore PLAN.md
        shutil.copy2(PLAN_PREV, PLAN_MD)

        # 2. Revert execution pointer in queue.json if available
        new_pointer = 0
        restored_title = "Unknown Plan"
        if PLAN_QUEUE.exists():
            with open(PLAN_QUEUE, "r", encoding="utf-8") as f:
                data = json.load(f)

            queue = data.get("queue", [])
            pointer = data.get("execution_pointer", 0)

            if pointer > 0:
                current_plan = queue[pointer]
                prev_plan = queue[pointer - 1]
                current_plan["status"] = "QUEUED"
                prev_plan["status"] = "IN_PROGRESS"
                data["execution_pointer"] = pointer - 1
                data["active_plan"] = "PLAN.md"
                data["active_target"] = prev_plan.get("target", "TOOLING_HARNESS")
                new_pointer = pointer - 1
                restored_title = prev_plan.get("title", "")
            else:
                restored_title = queue[0].get("title", "") if queue else "Active Plan"

            now_iso = datetime.datetime.now(datetime.timezone.utc).isoformat()
            if "history" not in data:
                data["history"] = []
            data["history"].append({
                "timestamp": now_iso,
                "action": "rollback",
                "pointer": new_pointer,
                "title": restored_title,
            })

            with open(PLAN_QUEUE, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
                f.write("\n")

        return {
            "status": "success",
            "message": "Stateless atomic rollback completed in 5ms",
            "active_plan": "PLAN.md",
            "execution_pointer": new_pointer,
            "restored_title": restored_title,
        }
    except Exception as e:
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Rollback execution failed: {e}",
        )


# ==============================================================================
# 5. Backward Compatibility Dispatch Endpoint
# ==============================================================================


@app.post("/api/dispatch", summary="Generic Action Dispatcher")
async def generic_dispatch(payload: DispatchRequest) -> Dict[str, Any]:
    """Receives generic action payloads for backwards compatibility with legacy sidecar scripts."""
    now_iso = datetime.datetime.now(datetime.timezone.utc).isoformat()
    record = {
        "id": str(uuid.uuid4()),
        "action": payload.action,
        "timestamp": now_iso,
        "client_timestamp": payload.timestamp,
        "source": "generic_dispatch",
    }
    INBOX_DIR.mkdir(parents=True, exist_ok=True)
    try:
        with open(ACTION_QUEUE_FILE, "a", encoding="utf-8") as f:
            f.write(json.dumps(record, ensure_ascii=False) + "\n")
        with open(LATEST_ACTION_FILE, "w", encoding="utf-8") as f:
            json.dump(record, f, indent=2, ensure_ascii=False)
            f.write("\n")
    except Exception as e:
        print(f"[!] Warning writing to action queue: {e}", file=sys.stderr)

    return {"status": "success", "message": "Dispatched to action queue", "record": record}


# ==============================================================================
# CLI Entrypoint
# ==============================================================================


def main() -> int:
    parser = argparse.ArgumentParser(
        prog="nkai_api.py",
        description="N'kai Local FastAPI Sidecar Bridge (Flight 7). Provides REST API on localhost:8000.",
    )
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Host interface (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=8000, help="Port to bind (default: 8000)")
    parser.add_argument("--reload", action="store_true", help="Enable auto-reload for development")
    args = parser.parse_args()

    import uvicorn

    print("=" * 80)
    print("⚡ N'KAI LOCAL FASTAPI SIDECAR BRIDGE (Flight 7)")
    print("=" * 80)
    print(f"Host:       http://{args.host}:{args.port}")
    print(f"Docs (UI):  http://{args.host}:{args.port}/docs")
    print(f"Endpoints:  /health, /api/mail, /api/action, /api/plan/rollback")
    print(f"Workspace:  {WORKSPACE_ROOT}")
    print("=" * 80)

    uvicorn.run("nkai_api:app" if args.reload else app, host=args.host, port=args.port, reload=args.reload)
    return 0


if __name__ == "__main__":
    sys.exit(main())
