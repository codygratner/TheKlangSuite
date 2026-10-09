# N'kai Distribution Bundle & Deployment Architecture (Topic 6.12)

## 1. Architectural Intent & Scope

The **N'kai Asymmetric Sidecar Framework** (`nkai`) provides visual sidecar canvases, interactive interview labs (`visual-grill-me`), deep architectural research boards (`deep-research`), structured post-mortem triage (`post-mortem`), and webview linters (`web-qa`). 

As the ecosystem expands across the Four Pillars (`TheKlangSuite`, `ToadTracker`, `nkai`, `TheKlangResearch`) and external client projects, distributing these agent capabilities must be instantaneous, robust, and reproducible.

### The Problem: Fragmented Tooling & Drift
Without a formal distribution bundle:
1. **Manual Duplication**: Skills, sidecar templates, and verification scripts are manually copied between repositories, causing version divergence and silent bugs.
2. **Domain Pollution**: Non-audio projects (such as pure webapps, CLI tools, or embedded hardware control firmware) risk getting polluted with specialized C++ audio-thread guardrails (zero `malloc`, zero mutexes).
3. **Broken Sidecar Infrastructure**: Missing Git-ignore boundaries or missing template files cause ephemeral sidecar artifacts (100KB+ rendered HTML files) to accidentally get committed to Git repositories.

### The Solution: Option B (Monolithic All-in-One Distribution Bundle)
Formally approved under **Topic 6.12**, N'kai packages its canonical pairing core and developer tools into a single, standardized bundle directory (`c:\Dev\nkai\bundle/`) deployed via a single-command PowerShell installer (`install_nkai_bundle.ps1`).

---

## 2. Bundle Topology & File Structure

The canonical distribution bundle resides in `c:\Dev\nkai\bundle/` and is organized into four standardized directories:

```text
c:\Dev\nkai\bundle/
├── templates/
│   ├── template.html                   # Universal Antigravity Dark Blue sidecar shell
│   └── context_clues_template.md       # Canonical 5-section context clues template
├── skills/
│   ├── visual-grill-me/                # Interactive architectural & UI interview lab
│   ├── deep-research/                  # Multi-domain research prompter & board generator
│   ├── post-mortem/                    # Methodical research & milestone triage lab
│   └── web-qa/                         # Sidecar HTML/CSS/JS auditor & linter
├── rules/
│   ├── nkai_guidelines.md              # Universal sidecar pairing protocols & handshake
│   └── audio_guardrails_addon.md       # Domain add-on: real-time audio thread invariants
└── tools/
    ├── audit_sidecar.py                # Standalone HTML tag & syntax linter
    └── verify_context_clues.py         # 5-section heading & link resolution validator
```

---

## 3. The Universal Pairing Core vs. Audio Add-on Separation

To serve both audio plugins and general-purpose software projects cleanly, the bundle enforces a two-tier installation model:

```
┌────────────────────────────────────────────────────────────────────────┐
│                      UNIVERSAL PAIRING CORE                            │
│  • Skills: visual-grill-me, deep-research, post-mortem, web-qa         │
│  • Templates: template.html, context_clues_template.md                 │
│  • Rules: nkai_guidelines.md (Asymmetric Split, Handshake Gate)        │
│  • Tools: audit_sidecar.py, verify_context_clues.py                    │
│  Target: ANY software repository using Antigravity pair-programming   │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                         Optional   │ [-Audio] Flag
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                        AUDIO GUARDRAILS ADD-ON                         │
│  • Rules: audio_guardrails_addon.md                                    │
│    - Zero allocations (new/malloc) in processBlock                     │
│    - Zero locks (mutex/critical section) on audio thread               │
│    - Zero blocking I/O (DBG, printf, file operations)                  │
│    - SIMD FastMath over CRT transcendentals                            │
│  Target: TheKlangSuite, ToadTracker, C++ DSP engines                   │
└────────────────────────────────────────────────────────────────────────┘
```

1. **Universal Pairing Core (Default)**:
   - Always installed regardless of target repository type.
   - Delivers interactive visual sidecars, design interviews, post-mortem triage, and link validation.
   - Zero DSP or C++ specific assumptions.
2. **Audio Guardrails Add-on (`-Audio` switch)**:
   - Installed selectively when `-Audio` is passed to the installer.
   - Installs `audio_guardrails_addon.md` into the target project's rule/directive directory.
   - Enforces real-time audio thread safety invariants without burdening non-audio repositories.

---

## 4. Single-Command Deployment Contract

### The Installer: `install_nkai_bundle.ps1`
The bundle is deployed using a cross-platform PowerShell 7+ script located at `c:\Dev\nkai\install_nkai_bundle.ps1` (with a convenience copy in `TheKlangSuite/tools/install_nkai_bundle.ps1`).

#### Invocation Syntax:
```powershell
# Standard deployment to current workspace
.\install_nkai_bundle.ps1

# Target a specific external workspace with Audio Guardrails
.\install_nkai_bundle.ps1 -TargetDir "C:\Dev\ToadTracker" -Audio

# Dry-run preview without modifying disk
.\install_nkai_bundle.ps1 -TargetDir "C:\Dev\MyProject" -DryRun
```

#### Parameter Contract:
| Parameter | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `-TargetDir` | `string` | `$PWD` | Absolute or relative path to the root of the target project workspace. |
| `-Audio` | `switch` | `$false` | When present, installs the Audio Guardrails Add-on into the target repository. |
| `-DryRun` | `switch` | `$false` | Previews file copy actions and directory creations without writing to disk. |
| `-Force` | `switch` | `$false` | Overwrites existing files in target destination without prompting. |

---

## 5. Target Workspace Directory Layout & Git Boundaries

Upon installation, `install_nkai_bundle.ps1` establishes the following structure in `-TargetDir`:

```text
<TargetDir>/
├── .agents/
│   ├── skills/
│   │   ├── visual-grill-me/
│   │   ├── deep-research/
│   │   ├── post-mortem/
│   │   └── web-qa/
│   └── sidecar/
│       ├── template.html               # Kept in Git: canonical base template
│       └── packages/                   # Tiers 2 state interchange (gitignored)
├── docs/
│   └── templates/
│       └── context_clues_template.md   # Kept in Git: canonical context clues skeleton
├── tools/
│   ├── audit_sidecar.py                # Kept in Git: web QA linter
│   └── verify_context_clues.py         # Kept in Git: link validator
└── .gitignore                          # Auto-patched with sidecar ignore rules
```

### Git-Ignore Boundary Invariant
Interactive sidecar canvases generated during daily work (e.g. `visual_grill_me.html`, `ui_ux_research_board.html`) can be hundreds of kilobytes and contain temporary session data.

To prevent repository bloat, `install_nkai_bundle.ps1` automatically verifies that `<TargetDir>/.gitignore` includes the canonical sidecar exclusion block:

```gitignore
# =====================================================================
# N'kai Interactive Sidecars & Webview Artifacts (Topic 6.12)
# =====================================================================
.agents/sidecar/*.html
!.agents/sidecar/template.html
.agents/sidecar/packages/
```

- **Preserved in Git**: `.agents/sidecar/template.html` (the foundational shell) and all skills/tools.
- **Ignored by Git**: All generated sidecar instances (`*.html`) and session data packages (`packages/`).

---

## 6. Verification & Health Monitoring

A deployed bundle must pass two rigorous automated health checks:
1. **Sidecar Template Hygiene (`tools/audit_sidecar.py`)**:
   - Asserts 100% HTML tag balance, script syntax validity, and pointer-event transparency on `.agents/sidecar/template.html`.
2. **Context Clues & Link Integrity (`tools/verify_context_clues.py`)**:
   - Asserts all 5 mandatory sections exist in `docs/templates/context_clues_template.md`.
   - Asserts 100% of local file hyperlinks resolve to valid files on disk with exit code `0`.

---

## 7. Relationship to Other Specifications
- **Topic 6.16: N'kai Canonical State Architecture**: Defines how session packages in `.agents/sidecar/packages/` serialize into Tier 2 JSON.
- **Topic 6.7: Context Clues Universal Template**: Defines the 5 canonical sections bundled in `docs/templates/context_clues_template.md`.
- **Topic 6.6: Dynamic Shortcuts Bar**: Defines the `[ ⌨️ Shortcuts ]` helper HUD embedded within `template.html`.
