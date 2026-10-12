#!/usr/bin/env python3
"""
tools/sync_simplenote.py
Simplenote ingestion and sync engine for The Cosmos (C:\\_\\Vault).

Supports:
1. Simplenote Export Zip:
   python tools/sync_simplenote.py --zip "C:\\path\\to\\notes.zip" [--dest "📬 Inbox"]
2. Simplenote Published Note URL:
   python tools/sync_simplenote.py --url "https://simplenote.com/p/..." [--dest "📬 Inbox"]
3. Simplenote Cloud API Pull:
   python tools/sync_simplenote.py --login --email "user@example.com"
   python tools/sync_simplenote.py --pull [--dest "📬 Inbox"]
"""

import sys
import os
import re
import json
import base64
import urllib.request
import urllib.parse
import zipfile
import argparse
from pathlib import Path
from datetime import datetime

# Universal UTF-8 console output for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

VAULT_ROOT = Path(r"C:\_\Vault")
CONFIDENTIAL_DIR = VAULT_ROOT / "🌿 Klang Confidential"
CREDENTIALS_FILE = CONFIDENTIAL_DIR / "simplenote_token.json"
DEFAULT_DEST = VAULT_ROOT / "📬 Inbox"


def sanitize_filename(name: str, max_len: int = 80) -> str:
    """Sanitizes text into a safe Windows filename."""
    name = re.sub(r'[\\/*?:"<>|]', "", name)
    name = re.sub(r"\s+", " ", name).strip()
    if not name:
        name = "Untitled Note"
    return name[:max_len]


def import_zip(zip_path: Path, dest_dir: Path):
    """Imports notes directly from a Simplenote export zip archive."""
    print(f"📦 Ingesting Simplenote archive: {zip_path}")
    if not zip_path.exists():
        print(f"❌ Error: Zip archive not found at {zip_path}")
        return 1

    dest_dir.mkdir(parents=True, exist_ok=True)
    count = 0

    with zipfile.ZipFile(zip_path, "r") as zf:
        for info in zf.infolist():
            if info.is_dir() or not (info.filename.endswith(".txt") or info.filename.endswith(".md")):
                continue

            content = zf.read(info.filename).decode("utf-8", errors="replace")
            lines = content.strip().splitlines()
            title = lines[0].lstrip("#").strip() if lines else "Untitled"
            safe_title = sanitize_filename(title)
            out_file = dest_dir / f"{safe_title}.md"

            # Avoid clobbering existing notes
            counter = 1
            while out_file.exists():
                out_file = dest_dir / f"{safe_title}_{counter}.md"
                counter += 1

            # Format frontmatter
            frontmatter = f"---\ntitle: \"{title.replace('\"', '')}\"\nsource: simplenote-export\ndate: {datetime.now().strftime('%Y-%m-%d')}\ntags: [simplenote, imported]\n---\n\n"
            out_file.write_text(frontmatter + content, encoding="utf-8")
            count += 1

    print(f"✅ Successfully ingested {count} notes into {dest_dir}!")
    return 0


def import_url(url: str, dest_dir: Path):
    """Fetches a published Simplenote URL and saves it as a markdown note."""
    print(f"🌐 Fetching published Simplenote: {url}")
    dest_dir.mkdir(parents=True, exist_ok=True)

    try:
        req = urllib.request.Request(
            url,
            headers={"User-Agent": "TheCosmos/1.0 (Simplenote-Bridge)"}
        )
        with urllib.request.urlopen(req) as resp:
            html = resp.read().decode("utf-8", errors="replace")

        # Simplenote publish pages contain the raw text inside a pre or note content div
        match = re.search(r'<div[^>]*class="[^"]*note-content[^"]*"[^>]*>(.*?)</div>', html, re.DOTALL)
        if match:
            raw_text = match.group(1)
            raw_text = re.sub(r"<br\s*/?>", "\n", raw_text)
            raw_text = re.sub(r"</p>", "\n\n", raw_text)
            raw_text = re.sub(r"<[^>]+>", "", raw_text)
            raw_text = urllib.parse.unquote(raw_text)
        else:
            # Fallback: strip tags
            raw_text = re.sub(r"<[^>]+>", "", html).strip()

        lines = raw_text.strip().splitlines()
        title = lines[0].lstrip("#").strip() if lines else "Simplenote Document"
        safe_title = sanitize_filename(title)
        out_file = dest_dir / f"{safe_title}.md"

        frontmatter = f"---\ntitle: \"{title.replace('\"', '')}\"\nsource_url: {url}\ndate: {datetime.now().strftime('%Y-%m-%d')}\ntags: [simplenote, web-publish]\n---\n\n"
        out_file.write_text(frontmatter + raw_text, encoding="utf-8")
        print(f"✅ Successfully saved note to: {out_file}")
        return 0
    except Exception as e:
        print(f"❌ Error fetching URL {url}: {e}")
        return 1


def login(email: str, password: str = None):
    """Authenticates with Simplenote Simperium API and caches the auth token."""
    if not password:
        import getpass
        password = getpass.getpass("Enter Simplenote Password: ")

    print(f"🔐 Authenticating with Simplenote for {email}...")
    auth_str = f"email={urllib.parse.quote(email)}&password={urllib.parse.quote(password)}"
    base64_auth = base64.b64encode(auth_str.encode("utf-8")).decode("ascii")

    try:
        req = urllib.request.Request(
            "https://app.simplenote.com/api/login",
            data=base64_auth.encode("utf-8"),
            headers={"Content-Type": "application/x-www-form-urlencoded"}
        )
        with urllib.request.urlopen(req) as resp:
            token = resp.read().decode("utf-8").strip()

        CONFIDENTIAL_DIR.mkdir(parents=True, exist_ok=True)
        CREDENTIALS_FILE.write_text(
            json.dumps({"email": email, "token": token, "updated_at": datetime.now().isoformat()}, indent=2),
            encoding="utf-8"
        )
        print(f"✅ Authentication successful! Auth token cached safely in air-gapped {CREDENTIALS_FILE.name}.")
        return 0
    except Exception as e:
        print(f"❌ Login failed: {e}")
        return 1


def pull_notes(dest_dir: Path):
    """Pulls all notes from Simplenote API using the cached token."""
    if not CREDENTIALS_FILE.exists():
        print(f"❌ Not logged in. Run 'python tools/sync_simplenote.py --login --email <email>' first.")
        return 1

    creds = json.loads(CREDENTIALS_FILE.read_text(encoding="utf-8"))
    token = creds.get("token")
    email = creds.get("email")

    print(f"📥 Pulling notes from Simplenote account: {email}...")
    dest_dir.mkdir(parents=True, exist_ok=True)

    try:
        # Fetch note index
        url = f"https://app.simplenote.com/api2/index?auth={token}&email={urllib.parse.quote(email)}&length=100"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req) as resp:
            data = json.loads(resp.read().decode("utf-8"))

        notes = data.get("data", [])
        print(f"   Found {len(notes)} notes in index. Fetching content...")

        count = 0
        for item in notes:
            note_key = item.get("key")
            if item.get("deleted") == 1:
                continue

            note_url = f"https://app.simplenote.com/api2/data/{note_key}?auth={token}&email={urllib.parse.quote(email)}"
            with urllib.request.urlopen(note_url) as note_resp:
                note_data = json.loads(note_resp.read().decode("utf-8"))

            content = note_data.get("content", "")
            tags = note_data.get("tags", [])
            lines = content.strip().splitlines()
            title = lines[0].lstrip("#").strip() if lines else f"Simplenote_{note_key}"
            safe_title = sanitize_filename(title)
            out_file = dest_dir / f"{safe_title}.md"

            frontmatter = f"---\ntitle: \"{title.replace('\"', '')}\"\nsimplenote_key: {note_key}\ntags: {tags}\ndate: {datetime.now().strftime('%Y-%m-%d')}\n---\n\n"
            out_file.write_text(frontmatter + content, encoding="utf-8")
            count += 1

        print(f"✅ Successfully pulled {count} notes into {dest_dir}!")
        return 0
    except Exception as e:
        print(f"❌ Error pulling notes: {e}")
        return 1


def main():
    parser = argparse.ArgumentParser(description="Sync Simplenote notes into The Cosmos (C:\\_\\Vault)")
    parser.add_argument("--zip", type=Path, help="Path to a Simplenote export zip file")
    parser.add_argument("--url", type=str, help="Published Simplenote note URL")
    parser.add_argument("--login", action="store_true", help="Log in to Simplenote account")
    parser.add_argument("--email", type=str, help="Simplenote account email")
    parser.add_argument("--password", type=str, help="Simplenote password (or prompt)")
    parser.add_argument("--pull", action="store_true", help="Pull notes from Simplenote account using cached token")
    parser.add_argument("--dest", type=Path, default=DEFAULT_DEST, help=f"Destination directory in The Cosmos (default: {DEFAULT_DEST})")

    args = parser.parse_args()

    if args.zip:
        return import_zip(args.zip, args.dest)
    elif args.url:
        return import_url(args.url, args.dest)
    elif args.login:
        if not args.email:
            print("❌ --email is required for login.")
            return 1
        return login(args.email, args.password)
    elif args.pull:
        return pull_notes(args.dest)
    else:
        parser.print_help()
        return 0


if __name__ == "__main__":
    sys.exit(main())
