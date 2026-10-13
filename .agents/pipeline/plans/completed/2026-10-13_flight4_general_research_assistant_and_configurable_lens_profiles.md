# Flight 4: General Research Assistant & Configurable Lens Profiles

**TARGET:** TOOLING_HARNESS  
**Role:** New Klang City Architecture & Tooling Engineering  
**Recommended Model:** Gemini 3.8 Flash (Thinking: High) — Tier 2 (Balanced Engineering & UI)  
> **Status:** COMPLETED ✅  

---

### Executive Overview & Architectural Dialectic
- **Anti-Yak-Shaving Invariant (Never Fork the Product)**:
  - N'kai remains ONE unified sidecar tool. Rejects building separate spin-off apps or multi-tenant plugin engines ("The Second-System Effect") to avoid derailment from core audio products (*The Klang Farmer*, *ToadTracker*).
- **Invariant Dialectic Core**:
  - The underlying Hegelian Dialectic Triad (Thesis ➔ Antithesis ➔ Synthesis) and the 3-Bullet Decision Card (🎯 Core Intent, 🛠️ Proposed Mechanism, 🚦 Action Route) remain permanently invariant across all domains.
  - Only the 4 domain-specific evaluation lenses adapt based on declarative JSON profiles.
- **Zero-Click Auto-Detection**:
  - Automatically matches note/card tags to specialized profiles:
    - `#studio`, `#hardware`, `#eurorack`, `#wiring`, `#m8`, `#tbd-16` ➔ `studio_hardware`
    - `#ttrpg`, `#writing`, `#lyrics`, `#gamedesign`, `#creative` ➔ `creative_general`
    - `#audio`, `#dsp`, `#vst`, `#juce`, `#tks`, or untagged ➔ `audio_dev` (default)
- **CLI & Sidecar Overrides**:
  - Full override support via `--profile <id>` across CLI tools (`check_mail.py`, `research_daemon.py`, `compile_prompt.py`).
  - Visual lens profile badge in the PCDA tab of the dossier drawer.

---

### Phase 1: Declarative Lens Dictionary Schema (`tools/lens_profiles.json`)
- [x] Create `tools/lens_profiles.json`:
  - Profiles: `audio_dev`, `studio_hardware`, `creative_general`.
  - 4 specialized lenses per profile with `id`, `name`, and `prompt`.
  - Tag lists for zero-click automatic detection.

---

### Phase 2: Pydantic Data Models & Profile Resolver (`tools/models.py`)
- [x] Implement in `tools/models.py`:
  - `LensDefinition`, `LensProfile`, and `LensProfilesConfig` Pydantic models.
  - `load_lens_profiles(config_path: Optional[Path] = None) -> LensProfilesConfig`
  - `detect_lens_profile(tags: List[str], text: str = "", config: Optional[LensProfilesConfig] = None) -> str`
  - Strict UTF-8 and graceful error fallbacks.

---

### Phase 3: Research Daemon & Prompt Compiler Lens Integration
- [x] Update `tools/research_daemon.py`:
  - Add `--profile` CLI option.
  - Auto-detect profile when synthesizing Pass 2 PCDA metrics.
  - Generate profile-tailored lenses and store `profile` and `profile_name` in PCDA metrics.
- [x] Update `tools/compile_prompt.py`:
  - Add `--profile` CLI option and tag auto-detection.
  - Inject active profile lens prompts into the compiled prompt handoff payload.

---

### Phase 4: Sidecar Visual Badge & Template Sync
- [x] Update `tools/board_renderer.py`:
  - Render dynamic `[ 🔬 Lens: <Profile Name> ]` pill badge in the PCDA tab header.
- [x] Sync templates using `python tools/sync_templates.py sync`.

---

### Phase 5: Unit Test Suite & Verification Gauntlet
- [x] Author `tools/tests/test_lens_profiles.py`:
  - Test profile loading from JSON.
  - Test tag auto-detection across all 3 profiles and fallback.
  - Test explicit `--profile` override.
  - Test `research_daemon.py` Pass 2 generation under multiple profiles.
  - Test `compile_prompt.py` profile injection.
- [x] Run `python -m unittest discover tools/tests/` (100% pass rate).
- [x] Run `python tools/verify_utf8.py --strict` (100% compliant).
- [x] Update `docs/BACKLOG.md` marking Flight 4 as COMPLETED.
- [x] Rebuild runway board (`python tools/build_runway.py`).
