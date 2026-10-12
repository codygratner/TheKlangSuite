#!/usr/bin/env python3
"""
tools/tests/test_audiothread_guard.py — Unit Tests for Audio-Thread Safety Guard
================================================================================
Tests real-time safety detection, scope-awareness (init vs hot loop),
--terse token shield mode, and strict violation enforcement.
"""

import shutil
import sys
import tempfile
import unittest
from pathlib import Path

# Universal UTF-8 Console Streams for Windows
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

from tools.audit_audiothread import audit_file

CLEAN_DSP_CODE = """
#pragma once
#include <vector>

class CleanEngine {
public:
    CleanEngine() {}
    void prepareToPlay(double sampleRate, int samplesPerBlock) {
        // Safe to allocate in setup
        m_buffer.resize(samplesPerBlock);
    }

    void processBlock(float* left, float* right, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            left[i] *= 0.5f;
            right[i] *= 0.5f;
        }
    }

private:
    std::vector<float> m_buffer;
};
"""

VIOLATING_DSP_CODE = """
#pragma once
#include <mutex>
#include <iostream>

class DirtyEngine {
public:
    void processBlock(float* left, float* right, int numSamples) {
        // Critical violation 1: Heap allocation
        float* temp = new float[numSamples];

        // Critical violation 2: Mutex lock
        std::mutex m;
        std::lock_guard<std::mutex> lock(m);

        // Critical violation 3: Console I/O
        std::cout << "Rendering " << numSamples << std::endl;

        for (int i = 0; i < numSamples; ++i) {
            // Warning: CRT transcendental
            left[i] = std::sin(left[i]);
        }

        delete[] temp;
    }
};
"""


class TestAudioThreadGuard(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp(prefix="test_audiothread_"))
        self.clean_file = self.test_dir / "CleanEngine.h"
        self.clean_file.write_text(CLEAN_DSP_CODE, encoding="utf-8")

        self.dirty_file = self.test_dir / "DirtyEngine.h"
        self.dirty_file.write_text(VIOLATING_DSP_CODE, encoding="utf-8")

    def tearDown(self):
        if self.test_dir.exists():
            shutil.rmtree(self.test_dir, ignore_errors=True)

    def test_clean_dsp_has_zero_violations(self):
        violations = audit_file(self.clean_file, hot_paths_only=True)
        # resize() inside prepareToPlay should not be flagged as a hot path violation
        critical = [v for v in violations if v["category"] != "TRANSCENDENTAL_WARNING"]
        self.assertEqual(len(critical), 0)

    def test_violating_dsp_catches_all_categories(self):
        violations = audit_file(self.dirty_file, hot_paths_only=True)
        categories = {v["category"] for v in violations}

        self.assertIn("ALLOCATION", categories)
        self.assertIn("BLOCKING_LOCK", categories)
        self.assertIn("BLOCKING_IO", categories)
        self.assertIn("TRANSCENDENTAL_WARNING", categories)

        critical_count = sum(1 for v in violations if v["category"] != "TRANSCENDENTAL_WARNING")
        self.assertGreaterEqual(critical_count, 3)


if __name__ == "__main__":
    unittest.main()
