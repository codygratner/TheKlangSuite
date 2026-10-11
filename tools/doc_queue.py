#!/usr/bin/env python3
"""
Deep Docs Queue Manager (Flight 15 / Topic 6.11)
Manages the staging queue for deep technical research papers, guides, and autopsies.
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

    args = parser.parse_args()

    if args.command == "add":
        item = queue_deep_doc(
            title=args.title,
            target_file=args.target,
            audience=args.audience,
            priority=args.priority,
        )
        print(f"Added [{item['id']}] '{item['title']}' -> {item['target_file']}")
    else:
        q = load_doc_queue()
        items = q.get("queue", [])
        print(f"Deep Docs Queue ({len(items)} items queued):")
        for it in items:
            print(f"  [{it['id']}] ({it.get('priority', 'P1')}) {it['title']} -> {it['target_file']} ({it.get('status', 'QUEUED')})")

