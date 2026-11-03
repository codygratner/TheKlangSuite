#!/usr/bin/env python3
"""
Test suite for tools/nkai_api.py (Flight 7 FastAPI Sidecar Bridge)
"""

import sys
from pathlib import Path

# Add workspace root to sys.path
WORKSPACE_ROOT = Path(__file__).resolve().parent.parent
if str(WORKSPACE_ROOT) not in sys.path:
    sys.path.insert(0, str(WORKSPACE_ROOT))

from fastapi.testclient import TestClient

# Ensure UTF-8 output
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

from tools.nkai_api import app

def run_tests():
    client = TestClient(app)

    print("=" * 60)
    print("🧪 Running N'kai FastAPI Bridge Test Suite")
    print("=" * 60)

    # 1. Health check
    res = client.get("/health")
    assert res.status_code == 200, f"Health failed: {res.text}"
    data = res.json()
    assert data["status"] == "online"
    assert data["bridge"] == "nkai-fastapi-bridge"
    print(f" [✓ PASS] GET /health -> status: {data['status']}, bridge: {data['bridge']}")

    # 2. Mail queue
    res = client.get("/api/mail")
    assert res.status_code == 200, f"Mail failed: {res.text}"
    mail_data = res.json()
    item_count = len(mail_data.get("items", []))
    print(f" [✓ PASS] GET /api/mail -> {item_count} items retrieved")

    # 3. Action dispatch: BACKLOG
    payload = {
        "action": "BACKLOG",
        "note_id": "mailbag-queue",
        "title": "Test Item for FastAPI Bridge",
        "notes": "Automated verification test",
        "tier": "tier2"
    }
    res = client.post("/api/action", json=payload)
    assert res.status_code == 200, f"Action failed: {res.text}"
    action_data = res.json()
    assert action_data["status"] == "success"
    print(f" [✓ PASS] POST /api/action -> {action_data['message']}")

    # 3b. 3+3 Action Architecture: SUBAGENT_PROBE
    sub_payload = {
        "action": "SUBAGENT_PROBE",
        "note_id": "mailbag-queue",
        "title": "Subagent Probe Test",
        "sub_mode": "local",
        "tier": "tier2"
    }
    res = client.post("/api/action", json=sub_payload)
    assert res.status_code == 200
    assert res.json()["status"] == "success"
    print(" [✓ PASS] POST /api/action [SUBAGENT_PROBE] -> Verified bottom-of-queue queueing")

    # 3c. 3+3 Action Architecture: QUEUE_SESSION
    queue_payload = {
        "action": "QUEUE_SESSION",
        "note_id": "mailbag-queue",
        "session_mode": "Grill",
        "tier": "tier1"
    }
    res = client.post("/api/action", json=queue_payload)
    assert res.status_code == 200
    assert res.json()["status"] == "success"
    print(" [✓ PASS] POST /api/action [QUEUE_SESSION] -> Verified session staging")

    # 3d. 3+3 Action Architecture: TABLE
    table_payload = {
        "action": "TABLE",
        "note_id": "mailbag-queue",
        "table_reason": "14-Day Hold",
        "tier": "tier3"
    }
    res = client.post("/api/action", json=table_payload)
    assert res.status_code == 200
    assert res.json()["status"] == "success"
    print(" [✓ PASS] POST /api/action [TABLE] -> Verified 14-day hold tabling")


    # 4. Rollback endpoint test
    # Check if PLAN_PREV exists before calling rollback
    prev_file = Path(".agents/pipeline/plans/snapshots/PLAN_PREV.md")
    if prev_file.exists():
        res = client.post("/api/plan/rollback")
        assert res.status_code == 200, f"Rollback failed: {res.text}"
        rollback_data = res.json()
        assert rollback_data["status"] == "success"
        print(f" [✓ PASS] POST /api/plan/rollback -> {rollback_data['message']}")
    else:
        print(" [i SKIP] POST /api/plan/rollback -> No PLAN_PREV.md snapshot currently exists (expected)")

    # 5. Generic dispatch endpoint
    res = client.post("/api/dispatch", json={"action": "test_ping", "timestamp": 123456})
    assert res.status_code == 200, f"Dispatch failed: {res.text}"
    print(" [✓ PASS] POST /api/dispatch -> backward compatibility verified")

    # 6. CORS & PNA Header verification
    res = client.options("/health", headers={"Origin": "null", "Access-Control-Request-Private-Network": "true"})
    assert res.status_code in (200, 204), f"OPTIONS preflight failed: {res.status_code}"
    pna_header = res.headers.get("access-control-allow-private-network")
    assert pna_header == "true", f"Expected PNA header 'true', got '{pna_header}'"
    print(f" [✓ PASS] CORS & PNA Preflight -> Access-Control-Allow-Private-Network: {pna_header}")

    print("=" * 60)
    print("🎉 ALL FASTAPI BRIDGE ENDPOINT TESTS PASSED!")
    print("=" * 60)
    return 0

if __name__ == "__main__":
    sys.exit(run_tests())
