#!/usr/bin/env python3
"""
tools/audiothread_guard.py — Canonical CLI Entrypoint for Real-Time Audio-Thread Safety Linter
=============================================================================================
Provides --terse / --token-shield mode for subagents and CI pipelines, eliminating token churn
when auditing C++ DSP files for heap allocations, mutexes, and CRT transcendentals.
"""

import sys
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parent.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.audit_audiothread import main

if __name__ == "__main__":
    main()
