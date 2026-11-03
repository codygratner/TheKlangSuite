#!/usr/bin/env python3
"""
Deep Docs Queue Manager (Flight 15 / Topic 6.11 / Deep-Docs 2.0)
Manages the staging queue for deep technical research papers, guides, and autopsies.
Supports P0 STALE_DRIFT priority elevation from AST drift detection.
"""

from datetime import datetime
import json
from pathlib import Path
import sys
from typing import Any, Dict, List, Optional

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_QUEUE_PATH = REPO_ROOT / ".agents" / "pipeline" / "docs" / "deep_docs_queue.json"


def load_doc_queue(queue_path: Optional[Path] = None) -> Dict[str, Any]:
    path = queue_path or DEFAULT_QUEUE_PATH
    if not path.exists():
        return {
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "version": "1.0.0",
            "queue": [],
        }
    return json.loads(path.read_text(encoding="utf-8"))


def save_doc_queue(data: Dict[str, Any], queue_path: Optional[Path] = None) -> Path:
    path = queue_path or DEFAULT_QUEUE_PATH
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return path


def queue_deep_doc(
    title: str,
    target_file: str,
    audience: str = "Developers & Musicians",
    priority: str = "P1",
    status: str = "QUEUED",
    queue_path: Optional[Path] = None,
) -> Dict[str, Any]:
    """Appends an item to the deep docs queue in 1 line."""
    data = load_doc_queue(queue_path)
    queue: List[Dict[str, Any]] = data.get("queue", [])
    
    doc_id = f"doc-{len(queue) + 1:02d}"
    item = {
        "id": doc_id,
        "title": title,
        "audience": audience,
        "target_file": target_file,
        "status": status,
        "priority": priority,
        "created_at": datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ"),
    }
    queue.append(item)
    data["queue"] = queue
    save_doc_queue(data, queue_path)
    return item


def bump_or_enqueue_stale(
    target_file: str,
    title: str = "",
    reason: str = "AST body-hash or symbol drift detected",
    queue_path: Optional[Path] = None,
) -> Dict[str, Any]:
    """Elevates an existing doc to P0 STALE_DRIFT or enqueues a new item if not found."""
    data = load_doc_queue(queue_path)
    queue: List[Dict[str, Any]] = data.get("queue", [])

    norm_target = target_file.replace("\\", "/").strip().lower()

    # Search existing queue
    for item in queue:
        existing_target = item.get("target_file", "").replace("\\", "/").strip().lower()
        if existing_target == norm_target or existing_target.endswith(norm_target) or norm_target.endswith(existing_target):
            item["status"] = "STALE_DRIFT"
            item["priority"] = "P0"
            item["reason"] = reason
            item["updated_at"] = datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")
            if title and (not item.get("title") or item["title"] == "Untitled"):
                item["title"] = title
            save_doc_queue(data, queue_path)
            return item

    # If not found, enqueue fresh item
    doc_id = f"doc-{len(queue) + 1:02d}"
    item = {
        "id": doc_id,
        "title": title or Path(target_file).stem.replace("_", " ").title(),
        "audience": "Developers & Musicians",
        "target_file": target_file.replace("\\", "/"),
        "status": "STALE_DRIFT",
        "priority": "P0",
        "reason": reason,
        "created_at": datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ"),
        "updated_at": datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ"),
    }
    queue.append(item)
    data["queue"] = queue
    save_doc_queue(data, queue_path)
    return item


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Deep Docs Queue Manager")
    subparsers = parser.add_subparsers(dest="command")

    # list
    subparsers.add_parser("list", help="List all queued deep docs")

    # add
    add_parser = subparsers.add_parser("add", help="Add a deep doc to the queue")
    add_parser.add_argument("--title", required=True, help="Title of the deep document")
    add_parser.add_argument("--target", required=True, help="Target file path (e.g. TheKlangResearch/guides/...)")
    add_parser.add_argument("--audience", default="Developers & Musicians", help="Target audience")
    add_parser.add_argument("--priority", default="P1", help="Priority (e.g. P0, P1, P2)")

    # bump
    bump_parser = subparsers.add_parser("bump", help="Bump or enqueue a stale document with P0 priority")
    bump_parser.add_argument("--target", required=True, help="Target file path")
    bump_parser.add_argument("--title", default="", help="Title of the document")
    bump_parser.add_argument("--reason", default="AST drift detected", help="Reason for stale classification")

    args = parser.parse_args()

    if args.command == "add":
        item = queue_deep_doc(
            title=args.title,
            target_file=args.target,
            audience=args.audience,
            priority=args.priority,
        )
        print(f"Added [{item['id']}] '{item['title']}' -> {item['target_file']}")
    elif args.command == "bump":
        item = bump_or_enqueue_stale(
            target_file=args.target,
            title=args.title,
            reason=args.reason
        )
        print(f"Bumped/Enqueued [{item['id']}] '{item['title']}' -> {item['target_file']} (P0: STALE_DRIFT)")
    else:
        q = load_doc_queue()
        items = q.get("queue", [])
        print(f"Deep Docs Queue ({len(items)} items queued):")
        for it in items:
            prio = it.get("priority", "P1")
            stat = it.get("status", "QUEUED")
            print(f"  [{it['id']}] ({prio}) {it['title']} -> {it['target_file']} ({stat})")
