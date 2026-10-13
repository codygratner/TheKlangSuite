#!/usr/bin/env python3
"""
tools/vault_link_rewriter.py — Sync-Time Link Rewriter for Obsidian Vault (Flight 23)
====================================================================================
Transforms file:/// links targeting mirrored documentation files into vault-relative
links during repo -> vault synchronization passes.

Invariants:
- Converts file:/// links targeting mirrored docs into vault-relative links.
- Strips line number anchors (#L123-L145), keeping heading anchors (#heading-name).
- Decodes %20 into spaces defensively.
- Preserves native code links (.cpp, .h, .json, .py, .png) as file:/// links.
"""

from pathlib import Path
import re
import sys
from urllib.parse import unquote

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


def rewrite_link_target(raw_url: str) -> str:
    """
    Rewrites a single file:/// link target into a vault-relative path if it points
    to a mirrored markdown document.
    """
    if not raw_url.startswith("file:///"):
        return raw_url

    # Strip prefix and unquote URL encoding
    clean_url = unquote(raw_url[8:])  # strip "file:///"

    # Separate fragment/anchor
    anchor = ""
    if "#" in clean_url:
        path_part, anchor_part = clean_url.split("#", 1)
        # Check if anchor is a line number anchor (e.g. L10, L123-L145)
        if not re.match(r"^L\d+(-L\d+)?$", anchor_part, re.IGNORECASE):
            anchor = f"#{anchor_part}"
    else:
        path_part = clean_url

    # Normalize path separators
    norm_path = path_part.replace("\\", "/")

    # Check extension — only rewrite markdown files
    if not norm_path.lower().endswith(".md"):
        # Native code or media asset: keep as file:/// with cleaned anchor
        return f"file:///{path_part}{anchor}"

    # Determine vault-relative mapping
    norm_lower = norm_path.lower()

    # 1. Target inside TheKlangSuite/docs/ -> Docs/
    if "/docs/" in norm_lower:
        idx = norm_lower.rfind("/docs/")
        rel_in_docs = norm_path[idx + 6:]  # strip up to /docs/
        return f"Docs/{rel_in_docs}{anchor}"

    # 2. Target is root PLAN.md
    if norm_lower.endswith("/plan.md") or norm_lower == "plan.md":
        return f"Docs/PLAN.md{anchor}"

    # 3. Target inside TheKlangVault/
    if "/theklangvault/" in norm_lower:
        idx = norm_lower.rfind("/theklangvault/")
        rel_in_vault = norm_path[idx + 15:]
        return f"{rel_in_vault}{anchor}"

    # 4. Target inside COSMOS/Vault/
    if "/vault/" in norm_lower:
        idx = norm_lower.rfind("/vault/")
        rel_in_vault = norm_path[idx + 7:]
        return f"{rel_in_vault}{anchor}"

    # 5. Target inside TheKlangResearch/ -> Research/
    if "/theklangresearch/" in norm_lower:
        idx = norm_lower.rfind("/theklangresearch/")
        rel_in_res = norm_path[idx + 18:]
        return f"Research/{rel_in_res}{anchor}"

    # Fallback: return file basename if it's a markdown note
    fn = Path(path_part).name
    return f"Docs/{fn}{anchor}"


def rewrite_markdown_links_for_vault(text: str) -> str:
    """
    Parses markdown text and rewrites all file:/// links that target mirrored docs.
    """
    pattern = re.compile(r'\[([^\]]+)\]\((file:///[^)]+)\)')

    def replacer(match):
        label = match.group(1)
        raw_url = match.group(2)
        new_url = rewrite_link_target(raw_url)
        return f"[{label}]({new_url})"

    return pattern.sub(replacer, text)


def main():
    if len(sys.argv) < 2:
        print("Usage: python tools/vault_link_rewriter.py <file_or_string> [--write]")
        sys.exit(1)

    arg = sys.argv[1]
    p = Path(arg)
    if p.exists() and p.is_file():
        content = p.read_text(encoding="utf-8", errors="replace")
        rewritten = rewrite_markdown_links_for_vault(content)
        if "--write" in sys.argv:
            p.write_text(rewritten, encoding="utf-8")
            print(f"✅ Rewrote vault links in: {p}")
        else:
            print(rewritten)
    else:
        print(rewrite_markdown_links_for_vault(arg))


if __name__ == "__main__":
    main()
