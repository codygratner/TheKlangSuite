#!/usr/bin/env python3
"""
🛰️ The Cosmic Codex & Ecosystem Fast Knowledge Search Engine
Uses Python standard library SQLite FTS5 for sub-millisecond full-text search,
BM25 relevance ranking, contextual highlighted snippets, and zero token churn.

DB Location: C:\\_\\Vault\\.agents\\index\\knowledge.db (Ignored in Git, hidden from Obsidian)
"""

import os
import sys
import time
import sqlite3
import argparse
import re

# Universal UTF-8 stdout/stderr invariant on Windows
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stderr.reconfigure(encoding="utf-8")
    except Exception:
        pass

DB_DIR = r"C:\_\Vault\.agents\index"
DB_PATH = os.path.join(DB_DIR, "knowledge.db")

# Target roots to index across the ecosystem
SEARCH_ROOTS = [
    {
        "root": r"C:\_\Vault",
        "name": "Vault",
        "skip_dirs": {".obsidian", ".agents", ".git", ".trash", ".cache", "Archive"}
    },
    {
        "root": r"C:\Dev\TheKlangSuite\docs",
        "name": "TKS Docs",
        "skip_dirs": {".git", ".agents", "build", "snapshots"}
    },
    {
        "root": r"C:\Dev\TheKlangDSP\docs",
        "name": "TheKlangDSP Docs",
        "skip_dirs": {".git", "build"}
    }
]

def init_db(db_path=DB_PATH):
    os.makedirs(os.path.dirname(db_path), exist_ok=True)
    con = sqlite3.connect(db_path)
    cur = con.cursor()
    
    cur.execute("""
    CREATE TABLE IF NOT EXISTS file_meta (
        path TEXT PRIMARY KEY,
        root_name TEXT,
        mtime REAL,
        size INTEGER,
        title TEXT,
        category TEXT,
        tags TEXT
    );
    """)
    
    cur.execute("""
    CREATE VIRTUAL TABLE IF NOT EXISTS docs_fts USING fts5(
        title,
        category,
        tags,
        content,
        path UNINDEXED,
        root_name UNINDEXED,
        tokenize='porter unicode61'
    );
    """)
    
    con.commit()
    return con

def extract_meta_and_content(file_path):
    try:
        with open(file_path, "r", encoding="utf-8", errors="replace") as f:
            raw = f.read()
    except Exception:
        return None, None, None, None

    title = ""
    tags = ""
    category = ""
    
    # Check for YAML frontmatter
    fm_match = re.match(r"^---\s*\n(.*?)\n---\s*\n", raw, re.DOTALL)
    if fm_match:
        fm_text = fm_match.group(1)
        for line in fm_text.splitlines():
            line_str = line.strip()
            if line_str.startswith("title:"):
                title = line_str.split("title:", 1)[1].strip().strip('"\'')
            elif line_str.startswith("tags:"):
                tags = line_str.split("tags:", 1)[1].strip()
            elif line_str.startswith("category:"):
                category = line_str.split("category:", 1)[1].strip().strip('"\'')

    # If no title in frontmatter, find first markdown H1
    if not title:
        h1_match = re.search(r"^#\s+(.+)$", raw, re.MULTILINE)
        if h1_match:
            title = h1_match.group(1).strip()
        else:
            title = os.path.basename(file_path)

    # Inferred category from directory structure if not in frontmatter
    if not category:
        parent = os.path.basename(os.path.dirname(file_path))
        grandparent = os.path.basename(os.path.dirname(os.path.dirname(file_path)))
        if parent in ("dsp", "specs", "manuals", "guides"):
            category = f"{grandparent}/{parent}"
        else:
            category = parent

    return title, category, tags, raw

def index_repository(con, verbose=False):
    cur = con.cursor()
    indexed_count = 0
    updated_count = 0
    deleted_count = 0
    
    # Fetch existing indexed file paths
    cur.execute("SELECT path, mtime FROM file_meta;")
    existing_records = {row[0]: row[1] for row in cur.fetchall()}
    current_files = set()
    
    start_time = time.perf_counter()
    
    for target in SEARCH_ROOTS:
        root_dir = target["root"]
        root_name = target["name"]
        skip_dirs = target["skip_dirs"]
        
        if not os.path.exists(root_dir):
            continue
            
        for dirpath, dirnames, filenames in os.walk(root_dir):
            dirnames[:] = [d for d in dirnames if d not in skip_dirs and not d.startswith(".")]
            
            for fn in filenames:
                if not fn.endswith(".md"):
                    continue
                
                full_path = os.path.normpath(os.path.join(dirpath, fn))
                current_files.add(full_path)
                
                try:
                    stat = os.stat(full_path)
                    mtime = stat.st_mtime
                    size = stat.st_size
                except Exception:
                    continue
                
                if full_path in existing_records:
                    if abs(existing_records[full_path] - mtime) < 1e-4:
                        # Unchanged, skip re-indexing
                        continue
                    else:
                        # Updated file
                        title, category, tags, content = extract_meta_and_content(full_path)
                        if content is not None:
                            # Remove old FTS entry
                            cur.execute("DELETE FROM docs_fts WHERE path = ?;", (full_path,))
                            # Insert new FTS entry
                            cur.execute(
                                "INSERT INTO docs_fts(title, category, tags, content, path, root_name) VALUES (?, ?, ?, ?, ?, ?);",
                                (title, category, tags, content, full_path, root_name)
                            )
                            # Update meta
                            cur.execute(
                                "UPDATE file_meta SET mtime = ?, size = ?, title = ?, category = ?, tags = ? WHERE path = ?;",
                                (mtime, size, title, category, tags, full_path)
                            )
                            updated_count += 1
                else:
                    # New file
                    title, category, tags, content = extract_meta_and_content(full_path)
                    if content is not None:
                        cur.execute(
                            "INSERT INTO file_meta(path, root_name, mtime, size, title, category, tags) VALUES (?, ?, ?, ?, ?, ?, ?);",
                            (full_path, root_name, mtime, size, title, category, tags)
                        )
                        cur.execute(
                            "INSERT INTO docs_fts(title, category, tags, content, path, root_name) VALUES (?, ?, ?, ?, ?, ?);",
                            (title, category, tags, content, full_path, root_name)
                        )
                        indexed_count += 1

    # Purge deleted files
    deleted_files = set(existing_records.keys()) - current_files
    for del_path in deleted_files:
        cur.execute("DELETE FROM file_meta WHERE path = ?;", (del_path,))
        cur.execute("DELETE FROM docs_fts WHERE path = ?;", (del_path,))
        deleted_count += 1
        
    con.commit()
    elapsed = (time.perf_counter() - start_time) * 1000
    
    if verbose or indexed_count > 0 or updated_count > 0 or deleted_count > 0:
        print(f"⚡ Index updated in {elapsed:.1f}ms: {indexed_count} added, {updated_count} updated, {deleted_count} pruned. (Total: {len(current_files)} documents)")
    
    return len(current_files)

def sanitize_query(query_str):
    """Sanitize query string for SQLite FTS5 MATCH syntax."""
    tokens = re.findall(r'[A-Za-z0-9_#\-]+', query_str)
    if not tokens:
        return ""
    # Support prefix matching on the last token and NEAR / AND chaining
    clean_tokens = []
    for t in tokens:
        if t.startswith("-"):
            clean_tokens.append(f'"{t}"')
        else:
            clean_tokens.append(f'"{t}"*')
    return " AND ".join(clean_tokens)

def search(query_str, limit=5, category_filter=None, con=None):
    if con is None:
        con = init_db()
        # Fast incremental index check
        index_repository(con, verbose=False)
        
    cur = con.cursor()
    fts_query = sanitize_query(query_str)
    if not fts_query:
        print("❌ Empty or invalid search query.")
        return []
    
    start_time = time.perf_counter()
    
    sql = """
    SELECT 
        path,
        root_name,
        title,
        category,
        tags,
        snippet(docs_fts, 3, '***', '***', ' ... ', 16) AS context_snippet,
        rank
    FROM docs_fts
    WHERE docs_fts MATCH ?
    """
    params = [fts_query]
    
    if category_filter:
        sql += " AND category LIKE ?"
        params.append(f"%{category_filter}%")
        
    sql += " ORDER BY rank LIMIT ?;"
    params.append(limit)
    
    try:
        cur.execute(sql, params)
        rows = cur.fetchall()
    except sqlite3.OperationalError:
        # Fallback to plain quoted phrase if complex match fails
        fallback_query = f'"{query_str}"'
        cur.execute(
            """
            SELECT path, root_name, title, category, tags, snippet(docs_fts, 3, '***', '***', ' ... ', 16), rank
            FROM docs_fts WHERE docs_fts MATCH ? ORDER BY rank LIMIT ?;
            """,
            (fallback_query, limit)
        )
        rows = cur.fetchall()
        
    elapsed = (time.perf_counter() - start_time) * 1000
    
    print(f"\n🔍 Search for '{query_str}' ({len(rows)} results in {elapsed:.2f}ms):")
    print("=" * 70)
    
    results = []
    for idx, (path, root_name, title, category, tags, snippet, rank) in enumerate(rows, 1):
        clean_path = path.replace("\\", "/")
        file_url = f"file:///{clean_path}"
        basename = os.path.basename(path)
        
        print(f"\n[{idx}] [{basename}]({file_url})")
        print(f"    Title:    {title}")
        print(f"    Location: [{root_name}] {category}")
        if tags:
            print(f"    Tags:     {tags}")
        clean_snippet = " ".join(snippet.split())
        print(f"    Excerpt:  {clean_snippet}")
        
        results.append({
            "path": path,
            "url": file_url,
            "title": title,
            "category": category,
            "snippet": snippet,
            "rank": rank
        })
        
    print("\n" + "=" * 70)
    return results

def get_stats(con):
    cur = con.cursor()
    cur.execute("SELECT COUNT(*) FROM file_meta;")
    total_docs = cur.fetchone()[0]
    
    cur.execute("SELECT root_name, COUNT(*) FROM file_meta GROUP BY root_name;")
    by_root = cur.fetchall()
    
    size_mb = os.path.getsize(DB_PATH) / (1024 * 1024) if os.path.exists(DB_PATH) else 0.0
    
    print("\n📊 Knowledge Base Search Index Statistics:")
    print(f"    Database Path:  {DB_PATH}")
    print(f"    Database Size:  {size_mb:.2f} MB")
    print(f"    Total Indexed:  {total_docs} markdown files")
    for r_name, count in by_root:
        print(f"      • {r_name:<18} : {count} documents")
    print("")

def main():
    parser = argparse.ArgumentParser(description="Cosmic Codex & Ecosystem SQLite FTS5 Fast Search Engine")
    parser.add_argument("query", nargs="*", help="Keywords to search for")
    parser.add_argument("--reindex", action="store_true", help="Force a full rebuild of the search index")
    parser.add_argument("--status", action="store_true", help="Display index statistics and document counts")
    parser.add_argument("--limit", type=int, default=5, help="Maximum number of results to display (default: 5)")
    parser.add_argument("--category", type=str, default=None, help="Filter results by category substring")
    
    args = parser.parse_args()
    
    con = init_db()
    
    if args.reindex:
        cur = con.cursor()
        cur.execute("DROP TABLE IF EXISTS file_meta;")
        cur.execute("DROP TABLE IF EXISTS docs_fts;")
        con.commit()
        con.close()
        con = init_db()
        print("🔄 Re-indexing entire knowledge base from scratch...")
        index_repository(con, verbose=True)
        get_stats(con)
        return
        
    if args.status:
        index_repository(con, verbose=False)
        get_stats(con)
        return
        
    if args.query:
        query_str = " ".join(args.query)
        search(query_str, limit=args.limit, category_filter=args.category, con=con)
    else:
        index_repository(con, verbose=True)
        get_stats(con)
        print("💡 Usage: python tools/search.py <keywords> [--limit N] [--category <cat>]")

if __name__ == "__main__":
    main()
