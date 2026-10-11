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
    q = load_doc_queue()
    print(f"Loaded {len(q.get('queue', []))} queued doc items from {DEFAULT_QUEUE_PATH}")
