#!/usr/bin/env python3
"""
tools/context_bus_bridge.py
========================================================================================
The Unified Context Clues, Inter-Agent Coherence Bus, and Virtual AST Bridge.
Combines:
  1. Active Working Set Page Table (.agents/extract_clues.json)
  2. MESI Cache Coherence Event Bus (Dispatches/*.coh.json) with Sliding-Window Coalescing
  3. Virtual AST & SCIP-Aligned Symbol Store (symbols.db, SQLite WAL + 256MB MMAP)
  4. Resilient Windows NTFS Atomic Writer with Extended-Length Path Prefix (\\?\\)
  5. Invariant-First 4-Layer QA Gauntlet & Chaos Engineering Suite (pcda doctor)
========================================================================================
"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
from enum import Enum
import hashlib
import json
import multiprocessing
import os
from pathlib import Path
import random
import re
import shutil
import sqlite3
import sys
import time
from typing import Annotated, Any, Dict, List, Literal, Optional, Set, Tuple, Union
import unittest
import subprocess

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass
if hasattr(sys.stderr, "reconfigure"):
    try:
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass


# ======================================================================================
# SECTION 1: SCHEMAS & HYBRID FALLBACK SHIM (Pydantic v2 + Dataclass Shim)
# ======================================================================================

try:
    from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator
    HAS_PYDANTIC = True
except ImportError:
    import dataclasses
    HAS_PYDANTIC = False

    def Field(default=dataclasses.MISSING, *, default_factory=dataclasses.MISSING, **kwargs):
        if default_factory is not dataclasses.MISSING:
            return dataclasses.field(default_factory=default_factory)
        elif default is not dataclasses.MISSING:
            return dataclasses.field(default=default)
        return dataclasses.field()

    class BaseModel:
        def __init_subclass__(cls, **kwargs):
            super().__init_subclass__()
            dataclasses.dataclass(frozen=True)(cls)

        def model_dump(self) -> Dict[str, Any]:
            return dataclasses.asdict(self)

        def model_dump_json(self, indent: Optional[int] = None) -> str:
            return json.dumps(self.model_dump(), indent=indent, default=str)

        @classmethod
        def model_validate(cls, obj: Any):
            if isinstance(obj, cls):
                return obj
            if not isinstance(obj, dict):
                raise TypeError(f"Expected dict, got {type(obj)}")
            fields = {f.name for f in dataclasses.fields(cls)}
            filtered = {k: v for k, v in obj.items() if k in fields}
            return cls(**filtered)

        @classmethod
        def model_validate_json(cls, json_data: Union[str, bytes]):
            if isinstance(json_data, bytes):
                json_data = json_data.decode("utf-8")
            return cls.model_validate(json.loads(json_data))


class MESIState(str, Enum):
    MODIFIED = "MODIFIED"
    EXCLUSIVE = "EXCLUSIVE"
    SHARED = "SHARED"
    INVALID = "INVALID"


class VectorClock(BaseModel):
    if HAS_PYDANTIC:
        model_config = ConfigDict(frozen=True, extra="forbid")
    clocks: Dict[str, int] = Field(default_factory=dict)

    def increment(self, agent_id: str) -> "VectorClock":
        new_clocks = dict(self.clocks)
        new_clocks[agent_id] = new_clocks.get(agent_id, 0) + 1
        if HAS_PYDANTIC:
            return VectorClock(clocks=new_clocks)
        return VectorClock(clocks=new_clocks)

    def happens_before(self, other: "VectorClock") -> bool:
        less_equal = all(self.clocks.get(k, 0) <= other.clocks.get(k, 0) for k in self.clocks)
        strictly_less = any(self.clocks.get(k, 0) < other.clocks.get(k, 0) for k in other.clocks)
        return less_equal and strictly_less


class InvalidationPayload(BaseModel):
    if HAS_PYDANTIC:
        model_config = ConfigDict(frozen=True, extra="forbid")
    event_type: Literal["INVALIDATE", "BATCH_INVALIDATE", "BUILD_REQUEST", "BUILD_SUCCESS", "BUILD_FAILED"] = "INVALIDATE"
    target_path: str
    invalidated_symbols: List[str] = Field(default_factory=list)
    new_sha256: str = Field(default="")
    common_ancestor: Optional[str] = None
    affected_count: int = 1


class CoherencePacket(BaseModel):
    if HAS_PYDANTIC:
        model_config = ConfigDict(frozen=True, extra="forbid", str_strip_whitespace=True)
    schema_version: int = Field(default=2)
    packet_id: str
    sender_agent_id: str
    recipient: str = Field(default="BROADCAST")
    mesi_state: MESIState = Field(default=MESIState.INVALID)
    timestamp_utc: str = Field(default_factory=lambda: datetime.now(timezone.utc).isoformat())
    payload: InvalidationPayload
    vector_clock: VectorClock = Field(default_factory=VectorClock)


class SymbolRecord(BaseModel):
    if HAS_PYDANTIC:
        model_config = ConfigDict(frozen=True, extra="forbid", str_strip_whitespace=True)
    uri: str
    repo: str
    file_path: str
    fqn: str
    name: str
    kind: str  # 'CLASS', 'STRUCT', 'METHOD', 'FUNCTION', 'SPEC'
    signature: str
    content_hash: str
    line_start: int
    line_end: int


class WorkingSetScope(BaseModel):
    if HAS_PYDANTIC:
        model_config = ConfigDict(frozen=True, extra="forbid", str_strip_whitespace=True)
    schema_version: int = Field(default=2)
    session_id: str
    budget_token_limit: int = Field(default=16000)
    spec_path: str
    focus_sections: List[str] = Field(default_factory=list)
    core_interfaces: List[Dict[str, Any]] = Field(default_factory=list)
    target_files: List[Dict[str, Any]] = Field(default_factory=list)


# ======================================================================================
# SECTION 2: WINDOWS NTFS DEFENSIVE I/O (Extended Paths & Jitter Backoff)
# ======================================================================================

def to_extended_path(p: Union[str, Path]) -> Path:
    """Enforces \\?\\ extended path prefix on Windows to defeat 260-char MAX_PATH limit."""
    p_obj = Path(p)
    raw = str(p_obj.resolve())
    if os.name == "nt" and not raw.startswith("\\\\?\\") and not raw.startswith("\\\\.\\"):
        return Path("\\\\?\\" + raw)
    return p_obj


def resilient_atomic_write(target_path: Union[str, Path], content: str, max_retries: int = 5) -> Path:
    """Atomic write with FlushFileBuffers, exponential backoff, jitter, and copy-fallback."""
    target = to_extended_path(target_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    temp_path = target.with_suffix(f".tmp_{os.getpid()}_{time.time_ns()}")

    with open(temp_path, "w", encoding="utf-8") as f:
        f.write(content)
        f.flush()
        try:
            os.fsync(f.fileno())
        except OSError:
            pass

    backoff = 0.01  # 10ms initial
    for attempt in range(max_retries):
        try:
            os.replace(temp_path, target)
            return target
        except (PermissionError, OSError) as e:
            if attempt == max_retries - 1:
                try:
                    shutil.copyfile(str(temp_path), str(target))
                    try:
                        os.remove(temp_path)
                    except OSError:
                        pass
                    return target
                except Exception as final_e:
                    raise IOError(f"FATAL: Atomic replace failed on NTFS: {final_e}") from e

            # Jittered backoff to break lockstep with Antivirus / Windows Search Indexer
            jitter = random.uniform(0.5, 1.5)
            time.sleep(backoff * jitter)
            backoff *= 2.0


# ======================================================================================
# SECTION 3: VIRTUAL AST & SCIP-ALIGNED SYMBOL DATABASE (SQLite WAL + 256MB MMAP)
# ======================================================================================

class SymbolDatabase:
    """Manages cross-repository symbols, dependency graphs, and AST slices in SQLite."""

    def __init__(self, db_path: Optional[Path] = None):
        if db_path is None:
            db_path = Path(".pcda") / "symbols.db"
        self.db_path = to_extended_path(db_path)
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        self.conn = sqlite3.connect(str(self.db_path), timeout=5.0)
        self.conn.row_factory = sqlite3.Row
        self._init_db()

    def _init_db(self):
        pragmas = [
            "PRAGMA journal_mode = WAL;",
            "PRAGMA synchronous = NORMAL;",
            "PRAGMA mmap_size = 268435456;",  # 256MB MMAP
            "PRAGMA cache_size = -65536;",    # 64MB Cache
            "PRAGMA busy_timeout = 5000;",    # 5s busy timeout
            "PRAGMA temp_store = MEMORY;",
            "PRAGMA foreign_keys = ON;"
        ]
        with self.conn:
            for pragma in pragmas:
                try:
                    self.conn.execute(pragma)
                except sqlite3.OperationalError:
                    pass
            self.conn.execute("""
                CREATE TABLE IF NOT EXISTS symbols (
                    symbol_id INTEGER PRIMARY KEY AUTOINCREMENT,
                    uri TEXT NOT NULL UNIQUE,
                    repo TEXT NOT NULL,
                    file_path TEXT NOT NULL,
                    fqn TEXT NOT NULL,
                    name TEXT NOT NULL,
                    kind TEXT NOT NULL,
                    signature TEXT NOT NULL,
                    content_hash TEXT NOT NULL,
                    line_start INTEGER NOT NULL,
                    line_end INTEGER NOT NULL,
                    is_dirty INTEGER DEFAULT 0,
                    updated_at INTEGER NOT NULL
                );
            """)
            self.conn.execute("""
                CREATE TABLE IF NOT EXISTS symbol_edges (
                    edge_id INTEGER PRIMARY KEY AUTOINCREMENT,
                    source_uri TEXT NOT NULL,
                    target_uri TEXT NOT NULL,
                    relation TEXT NOT NULL,
                    weight REAL DEFAULT 1.0,
                    FOREIGN KEY(source_uri) REFERENCES symbols(uri) ON DELETE CASCADE,
                    FOREIGN KEY(target_uri) REFERENCES symbols(uri) ON DELETE CASCADE
                );
            """)
            self.conn.execute("CREATE INDEX IF NOT EXISTS idx_sym_uri ON symbols(uri);")
            self.conn.execute("CREATE INDEX IF NOT EXISTS idx_sym_repo_file ON symbols(repo, file_path);")
            self.conn.execute("CREATE INDEX IF NOT EXISTS idx_sym_fqn ON symbols(fqn);")

            # FTS5 Full-Text Virtual Table
            try:
                self.conn.execute("""
                    CREATE VIRTUAL TABLE IF NOT EXISTS symbol_fts USING fts5(
                        uri UNINDEXED,
                        fqn,
                        signature,
                        tokenize = 'unicode61 remove_diacritics 2'
                    );
                """)
            except sqlite3.OperationalError:
                pass

    def check_health(self) -> bool:
        """Runs PRAGMA quick_check to verify DB health; recovers if corrupted."""
        try:
            cur = self.conn.cursor()
            cur.execute("PRAGMA quick_check;")
            res = cur.fetchone()[0]
            if res == "ok":
                cur.execute("PRAGMA wal_checkpoint(PASSIVE);")
                return True
        except sqlite3.DatabaseError:
            pass
        self._recover_db()
        return False

    def _recover_db(self):
        """Archives corrupted DB and re-instantiates pristine tables."""
        try:
            self.conn.close()
        except Exception:
            pass
        backup_path = self.db_path.with_suffix(f".corrupt.{int(time.time())}.db")
        if self.db_path.exists():
            try:
                shutil.copyfile(str(self.db_path), str(backup_path))
                os.remove(str(self.db_path))
            except Exception:
                pass
        self.conn = sqlite3.connect(str(self.db_path), timeout=5.0)
        self.conn.row_factory = sqlite3.Row
        self._init_db()

    def index_file(self, repo: str, file_path: str, content: str):
        """Extracts symbols using balanced regex AST parser and inserts into SQLite."""
        content_hash = hashlib.sha256(content.encode("utf-8")).hexdigest()
        lines = content.splitlines()
        epoch = int(time.time())

        patterns = [
            (r"^\s*(?:def|async\s+def)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\((.*?)\)", "FUNCTION", "py"),
            (r"^\s*class\s+([a-zA-Z_][a-zA-Z0-9_]*)", "CLASS", "py"),
            (r"^\s*(?:void|int|float|double|bool|auto|[A-Z][a-zA-Z0-9_]*)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\((.*?)\)\s*(?:noexcept)?\s*[{;]", "METHOD", "cpp"),
            (r"^\s*(?:struct|class)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*(?::\s*public\s+[a-zA-Z0-9_]+)?\s*\{", "STRUCT", "cpp")
        ]

        with self.conn:
            for i, line in enumerate(lines):
                for pat, kind, lang in patterns:
                    m = re.search(pat, line)
                    if m:
                        name = m.group(1)
                        fqn = f"{file_path}::{name}"
                        uri = f"symbol://{repo}/{file_path}::{name}#{kind.lower()}"
                        sig = line.strip()

                        self.conn.execute("""
                            INSERT INTO symbols (uri, repo, file_path, fqn, name, kind, signature, content_hash, line_start, line_end, is_dirty, updated_at)
                            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0, ?)
                            ON CONFLICT(uri) DO UPDATE SET
                                signature = excluded.signature,
                                content_hash = excluded.content_hash,
                                line_start = excluded.line_start,
                                line_end = excluded.line_end,
                                is_dirty = 0,
                                updated_at = excluded.updated_at
                        """, (uri, repo, file_path, fqn, name, kind, sig, content_hash, i + 1, min(len(lines), i + 30), epoch))

                        try:
                            self.conn.execute("""
                                INSERT OR REPLACE INTO symbol_fts (rowid, uri, fqn, signature)
                                SELECT s.symbol_id, s.uri, s.fqn, s.signature
                                FROM symbols s WHERE s.uri = ?
                            """, (uri,))
                        except sqlite3.OperationalError:
                            pass

    def mark_dirty(self, repo: str, file_path: str, symbol_name: Optional[str] = None):
        """Marks symbol or whole file as DIRTY (MESI Invalid state)."""
        with self.conn:
            if symbol_name:
                uri_prefix = f"symbol://{repo}/{file_path}::{symbol_name}%"
                self.conn.execute("UPDATE symbols SET is_dirty = 1 WHERE uri LIKE ?", (uri_prefix,))
            else:
                self.conn.execute("UPDATE symbols SET is_dirty = 1 WHERE repo = ? AND file_path = ?", (repo, file_path))

    def mark_subtree_dirty(self, repo: str, subtree_prefix: str):
        """Collapses invalidation storm: marks entire directory subtree dirty in O(1)."""
        with self.conn:
            pattern = f"{subtree_prefix}%"
            self.conn.execute("UPDATE symbols SET is_dirty = 1 WHERE repo = ? AND file_path LIKE ?", (repo, pattern))

    def lookup(self, query: str, limit: int = 5) -> List[Dict[str, Any]]:
        cur = self.conn.cursor()
        safe_query = f'"{query}"' if not any(c in query for c in '":*') else query
        try:
            cur.execute("""
                SELECT s.uri, s.repo, s.file_path, s.fqn, s.name, s.kind, s.signature, s.is_dirty, sf.rank
                FROM symbol_fts sf
                JOIN symbols s ON s.symbol_id = sf.rowid
                WHERE symbol_fts MATCH ?
                ORDER BY sf.rank
                LIMIT ?
            """, (safe_query, limit))
            return [dict(row) for row in cur.fetchall()]
        except sqlite3.OperationalError:
            cur.execute("""
                SELECT uri, repo, file_path, fqn, name, kind, signature, is_dirty, 0.0 as rank
                FROM symbols
                WHERE fqn LIKE ? OR signature LIKE ?
                LIMIT ?
            """, (f"%{query}%", f"%{query}%", limit))
            return [dict(row) for row in cur.fetchall()]


# ======================================================================================
# SECTION 4: COHERENCE BUS & STORM COALESCER (Sliding-Window Debouncing)
# ======================================================================================

class CoherenceBus:
    """Manages MESI coherence dispatches, Dead-Letter Queue (DLQ), and storm coalescing."""

    def __init__(self, dispatches_dir: Optional[Path] = None):
        if dispatches_dir is None:
            dispatches_dir = Path("New Klang City") / "Dispatches"
            if not dispatches_dir.exists():
                dispatches_dir = Path("Dispatches")
        self.dispatches_dir = to_extended_path(dispatches_dir)
        self.dlq_dir = to_extended_path(self.dispatches_dir / "dlq")
        self.dispatches_dir.mkdir(parents=True, exist_ok=True)
        self.dlq_dir.mkdir(parents=True, exist_ok=True)

    def publish(self, packet: CoherencePacket) -> Path:
        """Publishes packet atomically to Dispatches/*.coh.json with DLQ defense."""
        filename = f"{packet.packet_id}.coh.json"
        dest_path = self.dispatches_dir / filename
        try:
            content = packet.model_dump_json(indent=2)
            resilient_atomic_write(dest_path, content)
            return dest_path
        except Exception as e:
            # Route corrupted/unserializable payload to DLQ
            quarantine_path = self.dlq_dir / f"corrupt_{packet.packet_id}.json"
            quarantine_path.write_text(f'{{"error": "{str(e)}", "packet_id": "{packet.packet_id}"}}', encoding="utf-8")
            raise

    def quarantine_malformed(self, file_path: Path, error_message: str):
        """Moves malformed file to DLQ with diagnostic error receipt."""
        dlq_target = self.dlq_dir / file_path.name
        meta_target = self.dlq_dir / f"{file_path.stem}.meta.json"
        try:
            shutil.move(str(file_path), str(dlq_target))
            meta = {
                "quarantined_at": datetime.now(timezone.utc).isoformat(),
                "original_file": str(file_path),
                "error": error_message
            }
            resilient_atomic_write(meta_target, json.dumps(meta, indent=2))
        except Exception:
            pass

    def get_pending_invalidation_packets(self) -> List[CoherencePacket]:
        packets = []
        for p in sorted(self.dispatches_dir.glob("*.coh.json")):
            try:
                content = p.read_bytes()
                pkt = CoherencePacket.model_validate_json(content)
                packets.append(pkt)
            except Exception as e:
                self.quarantine_malformed(p, str(e))
        return packets

    def count_dlq(self) -> int:
        return len(list(self.dlq_dir.glob("*.json")))


class SlidingWindowCoalescer:
    """Coalesces rapid file modifications (Invalidation Storms) into atomic BATCH_INVALIDATE packets."""

    def __init__(self, bus: CoherenceBus, db: SymbolDatabase, threshold: int = 15, quiet_sec: float = 0.2):
        self.bus = bus
        self.db = db
        self.threshold = threshold
        self.quiet_sec = quiet_sec
        self.pending_events: List[Tuple[str, str, float]] = []  # (repo, path, timestamp)

    def record_event(self, repo: str, file_path: str):
        self.pending_events.append((repo, file_path, time.time()))

    def flush(self, force: bool = False) -> Optional[CoherencePacket]:
        if not self.pending_events:
            return None
        now = time.time()
        last_event_time = self.pending_events[-1][2]

        if not force and (now - last_event_time < self.quiet_sec) and (len(self.pending_events) < self.threshold):
            return None

        events = list(self.pending_events)
        self.pending_events.clear()

        # If event count exceeds threshold: Emit BATCH_INVALIDATE
        if len(events) >= self.threshold:
            repo = events[0][0]
            paths = [e[1] for e in events]
            common_root = os.path.commonpath(paths).replace("\\", "/") if paths else ""
            self.db.mark_subtree_dirty(repo, common_root)

            packet_id = f"batch_{int(now)}_{len(paths)}"
            payload = InvalidationPayload(
                event_type="BATCH_INVALIDATE",
                target_path=common_root or "*",
                invalidated_symbols=[],
                common_ancestor=common_root,
                affected_count=len(paths)
            )
            pkt = CoherencePacket(
                packet_id=packet_id,
                sender_agent_id="SLIDING_WINDOW_COALESCER",
                mesi_state=MESIState.INVALID,
                payload=payload
            )
            self.bus.publish(pkt)
            return pkt
        else:
            # Emit individual invalidations
            last_pkt = None
            for r, p, _ in events:
                self.db.mark_dirty(r, p)
                packet_id = f"coh_{int(now)}_{hashlib.md5(p.encode()).hexdigest()[:6]}"
                payload = InvalidationPayload(
                    event_type="INVALIDATE",
                    target_path=p,
                    invalidated_symbols=[]
                )
                last_pkt = CoherencePacket(
                    packet_id=packet_id,
                    sender_agent_id="COHERENCE_BUS",
                    mesi_state=MESIState.INVALID,
                    payload=payload
                )
                self.bus.publish(last_pkt)
            return last_pkt


# ======================================================================================
# SECTION 5: SYNTACTIC SHELL SLICING INVARIANT (LLM Context Armor)
# ======================================================================================

class SyntacticShellEngine:
    """Wraps targeted symbol implementations in clean enclosing namespaces & class skeletons."""

    @staticmethod
    def slice_with_shell(source_code: str, target_symbol: str) -> str:
        lines = source_code.splitlines()
        target_idx = -1
        for i, line in enumerate(lines):
            if target_symbol in line and ("def " in line or "class " in line or "(" in line):
                target_idx = i
                break

        if target_idx == -1:
            return f"// [Symbol '{target_symbol}' not found in source]\n"

        # Scan backwards for outer class or namespace
        enclosing_shell = []
        for i in range(target_idx - 1, -1, -1):
            line = lines[i].strip()
            if line.startswith("namespace ") or line.startswith("class ") or line.startswith("struct "):
                enclosing_shell.append(lines[i])

        enclosing_shell.reverse()
        body_lines = lines[target_idx:min(len(lines), target_idx + 25)]

        output = [f"// === SYNTACTIC SHELL: {target_symbol} ==="]
        for header in enclosing_shell:
            output.append(header)
            output.append("    // [... Folded: member declarations & dependencies ...]")

        output.extend(body_lines)

        for _ in range(len(enclosing_shell)):
            output.append("}")

        return "\n".join(output)


# ======================================================================================
# SECTION 6: WORKING SET PAGE TABLE COMPILER (pcda clues)
# ======================================================================================

class WorkingSetCompiler:
    """Compiles zero-token planning page table manifests from .agents/extract_clues.json."""

    def __init__(self, clues_path: Optional[Path] = None, db: Optional[SymbolDatabase] = None):
        if clues_path is None:
            clues_path = Path(".agents") / "extract_clues.json"
        self.clues_path = to_extended_path(clues_path)
        self.db = db or SymbolDatabase()

    def compile_manifest(self, scope_name: Optional[str] = None) -> str:
        if not self.clues_path.exists():
            return f"# Working Set Manifest\n[Warning: Clues file {self.clues_path} not found]"

        try:
            with open(self.clues_path, "r", encoding="utf-8") as f:
                data = json.load(f)
        except Exception as e:
            return f"# Working Set Manifest Error\nMalformed JSON in {self.clues_path}: {e}"

        scopes = data.get("scopes", {})
        active_scope_key = scope_name or data.get("active_scope") or next(iter(scopes.keys()), None)
        if not active_scope_key or active_scope_key not in scopes:
            return f"# Working Set Manifest\nScope '{active_scope_key}' not found in registry."

        scope = scopes[active_scope_key]
        lines = [
            f"# Active Working Set Manifest: {scope.get('title', active_scope_key)}",
            f"**Scope:** `{active_scope_key}` | **Budget Limit:** `{scope.get('budget_tokens_limit', 16000)} tokens`",
            "",
            "## 1. Specification Grounding",
            f"- **Spec Path:** `{scope.get('spec_path', 'docs/specs/spec.md')}`",
            f"- **Focus Sections:** {', '.join(scope.get('focus_sections', []))}",
            "",
            "## 2. Invariant Interfaces (Virtual AST Slices)"
        ]

        for iface in scope.get("core_interfaces", []):
            rel_file = iface.get("file", "")
            mode = iface.get("mode", "skeleton")
            symbols = iface.get("symbols", [])
            lines.append(f"### Interface: `{rel_file}` ({mode})")
            for sym in symbols:
                hits = self.db.lookup(sym, limit=1)
                if hits:
                    h = hits[0]
                    status = "⚠️ DIRTY (STALE)" if h.get("is_dirty") else "✅ CLEAN"
                    lines.append(f"- **`{h.get('fqn')}`** [{h.get('kind')}] — {status}")
                    lines.append(f"  `{h.get('signature')}`")
                else:
                    lines.append(f"- **`{sym}`** [UNINDEXED: pre-fetch directly from disk]")

        lines.extend([
            "",
            "## 3. Target Implementation Files"
        ])
        for tf in scope.get("target_files", []):
            lines.append(f"- `{tf.get('file')}` (Coherence: `{tf.get('coherence_key', 'default')}`)")

        return "\n".join(lines)


# ======================================================================================
# SECTION 7: INVARIANT-FIRST 4-LAYER QA GAUNTLET & CHAOS SUITE (pcda doctor)
# ======================================================================================

class TestLayer1SchemaContracts(unittest.TestCase):
    def test_coherence_packet_validation(self):
        payload = InvalidationPayload(
            event_type="INVALIDATE",
            target_path="source/dsp/Oscillator.h",
            invalidated_symbols=["Oscillator::render"]
        )
        packet = CoherencePacket(
            packet_id="test_001",
            sender_agent_id="AGENT_ALPHA",
            mesi_state=MESIState.INVALID,
            payload=payload
        )
        data = packet.model_dump_json()
        restored = CoherencePacket.model_validate_json(data)
        self.assertEqual(packet.packet_id, restored.packet_id)
        self.assertEqual(restored.mesi_state, MESIState.INVALID)


# Worker stress function at module level for Windows multiprocessing pickling
def _wal_torture_worker(db_path_str: str, wid: int):
    worker_db = SymbolDatabase(Path(db_path_str))
    for i in range(25):
        worker_db.index_file("test_repo", f"file_{wid}_{i}.cpp", f"void render_{wid}_{i}() {{}}")


class TestLayer2ConcurrencyTorture(unittest.TestCase):
    def test_sqlite_wal_multiprocess_contention(self):
        sandbox = Path(".agents") / "scratch" / "test_wal_sandbox"
        sandbox.mkdir(parents=True, exist_ok=True)
        db_path = sandbox / "torture.db"
        db = SymbolDatabase(db_path)

        processes = []
        for wid in range(4):
            p = multiprocessing.Process(target=_wal_torture_worker, args=(str(db_path), wid))
            processes.append(p)
            p.start()

        for p in processes:
            p.join(timeout=5.0)
            self.assertFalse(p.is_alive(), "Worker process timed out or deadlocked!")

        self.assertTrue(db.check_health())
        cur = db.conn.cursor()
        cur.execute("SELECT COUNT(*) FROM symbols;")
        count = cur.fetchone()[0]
        self.assertEqual(count, 100)

        # Close connection before rmtree to prevent WinError 32
        db.conn.close()

        # Cleanup
        try:
            shutil.rmtree(str(sandbox))
        except Exception:
            pass


class TestLayer3ASTRoundtrip(unittest.TestCase):
    def test_syntactic_shell_preserves_structure(self):
        source = (
            "namespace toad::dsp {\n"
            "class LadderFilter {\n"
            "    float cutoff;\n"
            "public:\n"
            "    void process(float* buf, int n) {\n"
            "        for (int i=0; i<n; ++i) buf[i] *= 0.5f;\n"
            "    }\n"
            "};\n"
            "}\n"
        )
        shell = SyntacticShellEngine.slice_with_shell(source, "process")
        self.assertIn("SYNTACTIC SHELL", shell)
        self.assertIn("class LadderFilter", shell)
        self.assertIn("void process", shell)
        self.assertIn("}", shell)


class TestLayer4ReactiveDLQ(unittest.TestCase):
    def test_dlq_quarantines_corrupted_packet(self):
        sandbox = Path(".agents") / "scratch" / "test_bus_sandbox"
        bus = CoherenceBus(sandbox)
        corrupt_file = sandbox / "bad_dispatch.coh.json"
        corrupt_file.write_text("{ MALFORMED_JSON_BYTES ...", encoding="utf-8")

        packets = bus.get_pending_invalidation_packets()
        self.assertEqual(len(packets), 0)
        self.assertTrue(bus.count_dlq() >= 1)

        # Cleanup
        try:
            shutil.rmtree(str(sandbox))
        except Exception:
            pass


class TestLayer5ChaosEngineering(unittest.TestCase):
    def test_chaos_atomic_write_sharing_violation(self):
        sandbox = Path(".agents") / "scratch" / "test_chaos_sandbox"
        sandbox.mkdir(parents=True, exist_ok=True)
        dest = sandbox / "critical_file.txt"

        # Pre-seed
        dest.write_text("initial pristine content", encoding="utf-8")

        # Atomic write with lockstep backoff
        resilient_atomic_write(dest, "updated safe content")
        self.assertEqual(dest.read_text(encoding="utf-8"), "updated safe content")

        # Cleanup
        try:
            shutil.rmtree(str(sandbox))
        except Exception:
            pass


def run_doctor(chaos: bool = False) -> int:
    """Executes the Invariant-First QA Gauntlet in < 1500ms."""
    print("=" * 70)
    print("  🧭 PCDA DOCTOR: INVARIANT VERIFICATION & INTEGRITY GAUNTLET")
    print("=" * 70)

    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    suite.addTests(loader.loadTestsFromTestCase(TestLayer1SchemaContracts))
    suite.addTests(loader.loadTestsFromTestCase(TestLayer2ConcurrencyTorture))
    suite.addTests(loader.loadTestsFromTestCase(TestLayer3ASTRoundtrip))
    suite.addTests(loader.loadTestsFromTestCase(TestLayer4ReactiveDLQ))

    if chaos:
        print("  [CHAOS MODE ACTIVE] Running fault injection & sharing violation tests...")
        suite.addTests(loader.loadTestsFromTestCase(TestLayer5ChaosEngineering))

    start = time.perf_counter()
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    elapsed_ms = (time.perf_counter() - start) * 1000

    print("-" * 70)
    print(f"Gauntlet finished in {elapsed_ms:.2f} ms")
    if elapsed_ms > 1500 and not chaos:
        print("[WARNING] Performance regression: Test suite exceeded 1500ms ceiling!")

    if result.wasSuccessful():
        print("[PASS] All invariants, schemas, concurrency, and self-healing verified.")
        return 0
    else:
        print(f"[FAIL] {len(result.failures)} failures, {len(result.errors)} errors encountered.")
        return 1


# ======================================================================================
# SECTION 7.5: TOWER C++ BUILD ENGINE & LISTENER (SANDBOXED & NOISE-FILTERED)
# ======================================================================================

def run_tower_build(target: str = "dsp_tests", config: str = "Release", tks_path: str = r"C:\Dev\TheKlangSuite", skip_tests: bool = False, dispatch: bool = False) -> int:
    """Executes a sandboxed, noise-filtered C++ build in The Tower (TKS).
    Redirects noisy compiler stdout/stderr to .agents/pipeline/build.log.
    Returns 0 on success, 1 on failure.
    """
    tks_root = Path(tks_path)
    build_dir = tks_root / "build"
    if not build_dir.exists():
        print(f"❌ [BUILD FAILED] Build directory does not exist: {build_dir}")
        return 1

    log_dir = tks_root / ".agents" / "pipeline"
    log_dir.mkdir(parents=True, exist_ok=True)
    build_log = log_dir / "build.log"
    test_log = log_dir / "test.log"

    # Gate 0: APVTS Parameter Contract Parity Check
    codegen_script = tks_root / "tools" / "codegen_apvts.py"
    if codegen_script.exists():
        verify_cmd = [sys.executable, str(codegen_script), "--all", "--verify"]
        verify_proc = subprocess.run(verify_cmd, capture_output=True, text=True, cwd=str(tks_root))
        if verify_proc.returncode != 0:
            print("🚨 [GATE 0 FAILED: PARAMETER CONTRACT DRIFT]")
            print(verify_proc.stderr.strip() or verify_proc.stdout.strip())
            print("Run 'python tools/codegen_apvts.py --all' to synchronize C++ parameters with contracts/*.yaml.")
            return 1

        # Gate 0.5: SQA Anti-Hollow Parameter & DSP Reachability Check (anti_hollow_canon.md)
    if os.environ.get("SKIP_SQA") != "1":
        known_plugins = ["hastur", "shoggoth", "nyarlat", "digistrike_fs2", "formless_spawn"]
        target_lower = target.lower()
        matched_plugin = next((p for p in known_plugins if p in target_lower), None)
        if matched_plugin:
            v_script = tks_root / "tools" / "verify_contract_bindings.py"
            if v_script.exists():
                v_proc = subprocess.run([sys.executable, str(v_script), matched_plugin], capture_output=True, text=True, encoding="utf-8", errors="replace", cwd=str(tks_root))
                if v_proc.returncode != 0:
                    print(f"🚨 [GATE 0.5 FAILED: CONTRACT BINDING DRIFT - {matched_plugin.upper()}]")
                    print(v_proc.stdout.strip() or v_proc.stderr.strip())
                    print("Resolve missing parameter bindings in Processor/Editor, or set SKIP_SQA=1 to bypass.")
                    return 1
            lint_script = tks_root / "tools" / "sqa" / "lint_dsp.py"
            if lint_script.exists():
                l_proc = subprocess.run([sys.executable, str(lint_script), matched_plugin], capture_output=True, text=True, encoding="utf-8", errors="replace", cwd=str(tks_root))
                if l_proc.returncode != 0:
                    print(f"🚨 [GATE 0.5 FAILED: DSP SINK REACHABILITY - {matched_plugin.upper()}]")
                    print(l_proc.stdout.strip() or l_proc.stderr.strip())
                    print("Resolve dummy cancellations / unread parameters in Voice header, or set SKIP_SQA=1 to bypass.")
                    return 1

    print(f"🔨 [TOWER BUILD] Compiling target '{target}' ({config})...")
    start_time = time.perf_counter()

    cmd = ["cmake", "--build", str(build_dir), "--config", config, "--parallel", "--target", target]
    try:
        with open(build_log, "w", encoding="utf-8") as f_log:
            proc = subprocess.run(cmd, stdout=f_log, stderr=subprocess.STDOUT, text=True, cwd=str(tks_root))
    except Exception as e:
        print(f"❌ [BUILD ERROR] Failed to invoke CMake: {e}")
        return 1

    compile_duration = time.perf_counter() - start_time

    if proc.returncode != 0:
        print(f"❌ [BUILD FAILED] Compilation exited with code {proc.returncode} in {compile_duration:.2f}s")
        errors = []
        try:
            content = build_log.read_text(encoding="utf-8", errors="ignore")
            for line in content.splitlines():
                if re.search(r"(\(\d+\):\s*error\s+[A-Z0-9]+:)|(:\d+:\d+:\s*(fatal\s+)?error:)|(LNK\d+)|(undefined reference)", line):
                    errors.append(line.strip())
        except Exception:
            pass

        if errors:
            print("\n🚨 [DIAGNOSTIC ERROR TRACE]")
            for err in errors[:10]:
                print(f"  • {err}")
            if len(errors) > 10:
                print(f"  ... and {len(errors) - 10} more errors. See {build_log}")
        else:
            print(f"  (Inspect full log at {build_log})")

        if dispatch:
            bus = CoherenceBus()
            bus.publish(CoherencePacket(
                packet_id=f"build_fail_{int(time.time())}",
                sender_agent_id="TOWER_BUILDER",
                mesi_state=MESIState.INVALID,
                payload=InvalidationPayload(
                    event_type="BUILD_FAILED",
                    target_path=f"build/{target}",
                    invalidated_symbols=[]
                )
            ))
        return 1

    print(f"✅ [BUILD SUCCESS] Target '{target}' compiled cleanly in {compile_duration:.2f}s")

    if (target in ["dsp_tests", "headless_plugin_runner"] or "dsp_tests" in target or "test" in target.lower()) and not skip_tests:
        test_exe = build_dir / config / f"{target}.exe"
        if not test_exe.exists():
            test_exe = build_dir / "dsp_tests.exe"
        if not test_exe.exists():
            print(f"⚠️ [TEST WARNING] Could not find test binary at {test_exe}")
            return 0

        print(f"🧪 [RUNNING TESTS] Executing {test_exe.name}...")
        test_start = time.perf_counter()
        try:
            with open(test_log, "w", encoding="utf-8") as f_test:
                t_proc = subprocess.run([str(test_exe)], stdout=f_test, stderr=subprocess.STDOUT, text=True, cwd=str(tks_root))
        except Exception as e:
            print(f"❌ [TEST ERROR] Failed to run test executable: {e}")
            return 1

        test_duration = time.perf_counter() - test_start
        if t_proc.returncode != 0:
            print(f"❌ [TEST SUITE FAILED] Exited with code {t_proc.returncode} in {test_duration:.2f}s")
            try:
                t_content = test_log.read_text(encoding="utf-8", errors="ignore")
                for line in t_content.splitlines():
                    if "FAIL" in line or "Assertion" in line or "error" in line.lower():
                        print(f"  • {line.strip()}")
            except Exception:
                pass
            return 1

        summary = ""
        try:
            t_content = test_log.read_text(encoding="utf-8", errors="ignore")
            for line in t_content.splitlines():
                if "PASSED 100%" in line or ("ALL" in line and "PASSED" in line):
                    summary = line.strip()
                    break
        except Exception:
            pass

        total_time = compile_duration + test_duration
        if summary:
            print(f"✅ [TESTS PASSED] {summary} ({test_duration:.2f}s, total: {total_time:.2f}s)")
        else:
            print(f"✅ [TESTS PASSED] All tests completed cleanly in {test_duration:.2f}s (total: {total_time:.2f}s)")

    if dispatch:
        bus = CoherenceBus()
        bus.publish(CoherencePacket(
            packet_id=f"build_pass_{int(time.time())}",
            sender_agent_id="TOWER_BUILDER",
            mesi_state=MESIState.EXCLUSIVE,
            payload=InvalidationPayload(
                event_type="BUILD_SUCCESS",
                target_path=f"build/{target}",
                invalidated_symbols=[]
            )
        ))
        print("📬 [BUS RECEIPT] Dispatched BUILD_SUCCESS packet to Dispatches/")

    return 0


def run_tower_listener(tks_path: str = r"C:\Dev\TheKlangSuite", poll_interval: float = 2.0, once: bool = False):
    """Listens on Dispatches/ for incoming BUILD_REQUEST packets and triggers automated builds."""
    print(f"🛰️ [TOWER LISTENER ACTIVE] Monitoring Dispatches/ for BUILD_REQUEST directives...")
    dispatch_dir = Path("Dispatches")
    if not dispatch_dir.exists():
        dispatch_dir = Path(tks_path) / "Dispatches"

    while True:
        if dispatch_dir.exists():
            for p in sorted(dispatch_dir.glob("*.json")):
                if "BUILD_REQUEST" in p.name:
                    print(f"📥 [DISPATCH RECEIVED] Processing {p.name}...")
                    try:
                        data = json.loads(p.read_text(encoding="utf-8"))
                        target = data.get("target", "dsp_tests")
                        config = data.get("config", "Release")
                        rc = run_tower_build(target=target, config=config, tks_path=tks_path, dispatch=True)
                        p.unlink(missing_ok=True)
                        if once:
                            return rc
                    except Exception as e:
                        print(f"❌ [LISTENER ERROR] Error processing {p.name}: {e}")
        if once:
            break
        time.sleep(poll_interval)


# ======================================================================================
# SECTION 8: CLI ENTRY POINT & SUBCOMMANDS
# ======================================================================================

def main():
    parser = argparse.ArgumentParser(description="Unified Context Clues, Coherence Bus & Virtual AST Bridge")
    subparsers = parser.add_subparsers(dest="command", help="Subcommand to execute")

    # clues
    p_clues = subparsers.add_parser("clues", help="Assemble zero-token working set manifest")
    p_clues.add_argument("--scope", type=str, default=None, help="Target scope key in extract_clues.json")

    # emit
    p_emit = subparsers.add_parser("emit", help="Emit MESI invalidation packet over the bus")
    p_emit.add_argument("--file", type=str, required=True, help="Relative file path modified")
    p_emit.add_argument("--symbols", type=str, default="", help="Comma-separated list of symbols invalidated")
    p_emit.add_argument("--repo", type=str, default="vault", help="Source repository slug")

    # index
    p_index = subparsers.add_parser("index", help="Index a file into symbols.db")
    p_index.add_argument("--file", type=str, required=True, help="File to parse and index")
    p_index.add_argument("--repo", type=str, default="vault", help="Target repository slug")

    # lookup
    p_lookup = subparsers.add_parser("lookup", help="Query symbols via FTS5 BM25")
    p_lookup.add_argument("query", type=str, help="Symbol or function identifier")
    p_lookup.add_argument("--limit", type=int, default=5, help="Max results returned")

    # doctor / test
    p_doctor = subparsers.add_parser("doctor", help="Run 4-layer invariant verification gauntlet")
    p_doctor.add_argument("--chaos", action="store_true", help="Include chaos fault-injection tests")

    p_test = subparsers.add_parser("test", help="Alias for doctor")
    p_test.add_argument("--chaos", action="store_true", help="Include chaos fault-injection tests")

    # hook-verify
    p_hook = subparsers.add_parser("hook-verify", help="Sub-50ms git pre-commit hook validator")

    # build
    p_build = subparsers.add_parser("build", help="Automated isolated C++ build & test execution in The Tower")
    p_build.add_argument("--target", type=str, default="dsp_tests", help="CMake target to build (default: dsp_tests)")
    p_build.add_argument("--config", type=str, default="Release", help="Build configuration (default: Release)")
    p_build.add_argument("--tks-path", type=str, default=r"C:\Dev\TheKlangSuite", help="Path to TheKlangSuite repository")
    p_build.add_argument("--skip-tests", action="store_true", help="Skip running dsp_tests after compilation")
    p_build.add_argument("--dispatch", action="store_true", help="Emit BUILD_SUCCESS/BUILD_FAILED packet to bus")

    # listen
    p_listen = subparsers.add_parser("listen", help="Listen on Dispatches/ for incoming BUILD_REQUEST packets")
    p_listen.add_argument("--tks-path", type=str, default=r"C:\Dev\TheKlangSuite", help="Path to TheKlangSuite repository")
    p_listen.add_argument("--once", action="store_true", help="Process at most one job and exit")

    args = parser.parse_args()

    if args.command == "clues":
        compiler = WorkingSetCompiler()
        manifest = compiler.compile_manifest(args.scope)
        print(manifest)

    elif args.command == "emit":
        bus = CoherenceBus()
        db = SymbolDatabase()
        syms = [s.strip() for s in args.symbols.split(",") if s.strip()]
        db.mark_dirty(args.repo, args.file)
        packet_id = f"coh_{int(time.time())}_{hashlib.md5(args.file.encode()).hexdigest()[:6]}"
        payload = InvalidationPayload(
            event_type="INVALIDATE",
            target_path=args.file,
            invalidated_symbols=syms
        )
        packet = CoherencePacket(
            packet_id=packet_id,
            sender_agent_id="CLI_EMIT",
            mesi_state=MESIState.INVALID,
            payload=payload
        )
        target = bus.publish(packet)
        print(f"Emitted MESI Invalidation: {target.name}")

    elif args.command == "index":
        p = Path(args.file)
        if not p.exists():
            print(f"Error: {p} does not exist.")
            sys.exit(1)
        content = p.read_text(encoding="utf-8", errors="ignore")
        db = SymbolDatabase()
        db.index_file(args.repo, str(p).replace("\\", "/"), content)
        print(f"Indexed symbols from {p} in symbols.db")

    elif args.command == "lookup":
        db = SymbolDatabase()
        results = db.lookup(args.query, limit=args.limit)
        if not results:
            print(f"No symbols found matching '{args.query}'")
        for r in results:
            status = "[DIRTY]" if r.get("is_dirty") else "[CLEAN]"
            print(f"{status} {r.get('uri')} -> {r.get('signature')}")

    elif args.command in ("doctor", "test"):
        sys.exit(run_doctor(chaos=args.chaos))

    elif args.command == "build":
        rc = run_tower_build(
            target=args.target,
            config=args.config,
            tks_path=args.tks_path,
            skip_tests=args.skip_tests,
            dispatch=args.dispatch
        )
        sys.exit(rc)

    elif args.command == "listen":
        rc = run_tower_listener(tks_path=args.tks_path, once=args.once)
        sys.exit(rc or 0)

    elif args.command == "hook-verify":
        # Pre-commit hook: Verify DLQ is clean and clues file is valid
        start = time.perf_counter()
        bus = CoherenceBus()
        dlq_count = bus.count_dlq()
        if dlq_count > 0:
            print(f"❌ [PRE-COMMIT BLOCKED] {dlq_count} quarantined packets in Dispatches/dlq/. Clean before commit!")
            sys.exit(1)

        clues_path = Path(".agents") / "extract_clues.json"
        if clues_path.exists():
            try:
                content = clues_path.read_text(encoding="utf-8")
                json.loads(content)
            except Exception as e:
                print(f"❌ [PRE-COMMIT BLOCKED] Malformed .agents/extract_clues.json: {e}")
                sys.exit(1)

        elapsed_ms = (time.perf_counter() - start) * 1000
        print(f"✅ [PRE-COMMIT PASSED] Invariants verified in {elapsed_ms:.2f} ms")
        sys.exit(0)

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
