#!/usr/bin/env python3
"""
tools/tests/test_sentry_auth.py — Unit tests for Sentry Bearer Authentication
=============================================================================
"""

import json
import os
import sys
import tempfile
import unittest
from pathlib import Path

# Ensure UTF-8 console output for Windows terminal safety
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

from tools.sentry_auth import get_or_create_sentry_token, verify_sentry_token


class TestSentryAuth(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.auth_file = Path(self.temp_dir.name) / "sentry_auth.json"

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_get_or_create_sentry_token(self):
        token1 = get_or_create_sentry_token(self.auth_file)
        self.assertIsInstance(token1, str)
        self.assertGreaterEqual(len(token1), 64)  # 32 bytes hex = 64 chars

        # Second call should load existing token without regenerating
        token2 = get_or_create_sentry_token(self.auth_file)
        self.assertEqual(token1, token2)

    def test_verify_sentry_token_valid(self):
        token = "test_sentry_secret_key_1234567890abcdef"
        self.assertTrue(verify_sentry_token(f"Bearer {token}", token))
        self.assertTrue(verify_sentry_token(f"bearer {token}", token))
        self.assertTrue(verify_sentry_token(f"BEARER {token}", token))

    def test_verify_sentry_token_invalid(self):
        token = "test_sentry_secret_key_1234567890abcdef"
        self.assertFalse(verify_sentry_token("Bearer wrong_token", token))
        self.assertFalse(verify_sentry_token("", token))
        self.assertFalse(verify_sentry_token(None, token))
        self.assertFalse(verify_sentry_token("Basic 12345", token))
        self.assertFalse(verify_sentry_token(token, token))  # missing 'Bearer '
        self.assertFalse(verify_sentry_token("Bearer", token))
        self.assertFalse(verify_sentry_token(f"Bearer {token} extra", token))


if __name__ == "__main__":
    unittest.main()
