# N'kai Per-Card Response Drawer Architecture (Flight 19 / `rj-08`)

## 1. Architectural Intent & Storyline

During interactive mailbag and note triage sessions, AI agents previously emitted multi-paragraph architectural rationales, PCDA trade-off dialectics, and implementation roadmaps directly into the chat transcript. This created two critical system defects:

1. **Context Window Token Bloat**: Emitting 400-word decision matrices for every note quickly consumes active chat context, accelerating premature rolling blackbox compaction and increasing the risk of amnesia during long pair-programming flights.
2. **Context Fragmentation**: The human developer's visual review surface was split. The user evaluated card titles and tags inside the N'kai sidecar pane or browser window, but had to switch back and scroll through the chat transcript to read the agent's explanation.
3. **Mobile & Offline Incompatibility**: In N'kai v0.3.0 standalone browser tabs and future mobile companions, chat transcripts are unavailable. Without embedded reasoning, the board lacked complete institutional provenance.

**Flight 19** introduces the **Per-Card Response Drawer**:
- **In-Card Embedded Reasoning**: Agents attach long-form markdown assessments, PCDA matrices, code signatures, and benchmark targets directly to cards via the `agent_reply` schema field.
- **Chat-Minimal Pair Programming**: The chat transcript stays lean—agents emit 1–2 crisp status lines (`"Triaged to Backlog. Architectural breakdown committed to card drawer."`) while full technical provenance lives on the card.
- **Glassmorphic Accessible UI**: When `agent_reply` is present, the card renders an accessible collapsible drawer (`#drawer-<id>`) with smooth cubic-bezier transitions, code block styling, and procedural Web Audio detent haptics.

```
┌────────────────────────────────────────────────────────────────────────┐
│                   CHAT-MINIMAL PAIR PROGRAMMING FLOW                   │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│   HUMAN CLICK                                                          │
│   [ ⚖️ PCDA ] or [ 📥 Backlog ] on N'kai Sidecar Card                  │
│        │                                                               │
│        ▼                                                               │
│   AGENT EXECUTION                                                      │
│   python tools/check_mail.py --reply-card <id> --text "<markdown>"     │
│        │                                                               │
│        ├────────────────────────────────┬──────────────────────────┐   │
│        ▼                                ▼                          │   │
│   LEDGER MUTATION (<50ms)       CHAT OUTPUT (1-2 lines)            │   │
│   Updates mail_queue.json       "Triaged rj-08 to Backlog.         │   │
│   Regenerates HTML board         Detailed PCDA matrix committed    │   │
│                                  to in-card response drawer."      │   │
│                                                                    ▼   │
│                                      CONTEXT WINDOW PRESERVED!     │   │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Core Architecture

### 2.1 Data Model Expansion (`tools/models.py`)

Both `MailItem` and `RambleCard` incorporate schema fields for agent response management:

```python
class MailItem(BaseModel):
    # ... existing fields ...
    agent_reply: Optional[str] = None
    agent_reply_at: Optional[str] = None
    has_reply: bool = False

    @model_validator(mode="after")
    def sync_has_reply(self) -> "MailItem":
        if self.agent_reply and self.agent_reply.strip():
            self.has_reply = True
        elif not self.agent_reply:
            self.has_reply = False
        return self
```

- `agent_reply`: Raw Markdown content of the agent's response, supporting headings, lists, inline code, fenced code blocks, and links.
- `agent_reply_at`: ISO 8601 timestamp recording when the reply was committed.
- `has_reply`: Boolean indicator automatically set by the validator, serialized into JSON for zero-computation filtering.
- **Ramble Ingestion Integration**: `RambleCard` passes `agent_reply` and `agent_reply_at` cleanly through `to_mail_item()`.
- **Ledger Preservation**: `build_mail_queue` in `check_mail.py` preserves `agent_reply` and `agent_reply_at` across regeneration runs, preventing data loss.

---

### 2.2 Accessible HTML Drawer Markup (`master_template.html` & `board_renderer.py`)

When `item.agent_reply` is present and non-empty, `render_card_html` conditionally injects accessible drawer markup with deterministic DOM IDs:

```html
<div class="card-drawer-container" id="drawer-container-{{CARD_ID}}">
  <button class="card-drawer-toggle" id="btn-drawer-{{CARD_ID}}" onclick="toggleResponseDrawer('{{CARD_ID}}')" aria-expanded="false">
    <span class="drawer-toggle-icon">💬</span>
    <span class="drawer-toggle-label">Agent Assessment & Rationale</span>
    <span class="drawer-toggle-caret">▾</span>
  </button>
  <div class="card-drawer-content" id="drawer-{{CARD_ID}}" hidden>
    <div class="drawer-inner markdown-body">
      {{AGENT_REPLY_HTML}}
    </div>
  </div>
</div>
```

- **Deterministic IDs**: `btn-drawer-<id>`, `drawer-<id>`, and `drawer-container-<id>` match the card's sanitized identifier for automated testing and test automation hooks.
- **ARIA Semantics**: `aria-expanded` synchronizes dynamically on click (`false` ↔ `true`).
- **Progressive Disclosure**: The drawer defaults to `hidden`, keeping card vertical height compact until expanded.
- **Header Chip**: Cards with replies render a `💬 Agent Reply` badge in `.mail-card-header` alongside priority and status chips.

---

### 2.3 Visual Styling & Animation Palette (`sidecar.css`)

Styled using N'kai's dark slate palette with subtle purple accents:

- **Border Accent**: 1px subtle purple border (`rgba(168, 85, 247, 0.3)`) with a 3px solid `#a855f7` left indicator rail.
- **Background Contrast**: Translucent dark slate background (`rgba(15, 23, 42, 0.65)`) ensuring readability over the card body.
- **Caret Flip Animation**: Smooth CSS transform (`rotate(180deg)`) on `.drawer-toggle-caret` when `[aria-expanded="true"]`.
- **Slide Down Animation**: 200ms cubic-bezier transition (`drawerSlideDown`) preventing jarring layout shifts.
- **Typography & Code Blocks**: Fenced code blocks (`<pre><code>`) use `#030712` background with `#e2e8f0` mono text and purple borders.

---

### 2.4 Procedural Detent Audio & Drawer Controller (`sidecar.js`)

The client-side controller handles toggling, accessibility attributes, and Web Audio haptic feedback:

```javascript
function toggleResponseDrawer(cardId) {
  const clean = cardId.startsWith('drawer-') ? cardId.substring(7) : cardId;
  const drawer = document.getElementById('drawer-' + clean) || document.getElementById(cardId);
  const btn = document.getElementById('btn-drawer-' + clean) || document.getElementById('btn-' + cardId);
  if (!drawer || !btn) return;

  const isClosed = drawer.hasAttribute('hidden') || !drawer.classList.contains('is-open');
  if (isClosed) {
    drawer.removeAttribute('hidden');
    drawer.classList.add('is-open');
    btn.setAttribute('aria-expanded', 'true');
  } else {
    drawer.setAttribute('hidden', '');
    drawer.classList.remove('is-open');
    btn.setAttribute('aria-expanded', 'false');
  }

  if (typeof playHaptic === 'function') {
    try {
      playHaptic('tick');
    } catch (e) {}
  }
}
```

- **Haptic Feedback**: Plays `playHaptic('tick')`—a procedural 800Hz → 400Hz exponential sine decay simulating a tactile magnetic detent.
- **Silent Audio Guard**: Respects the user's audio mute state (`localStorage.getItem('nkai_audio_muted')`).

---

## 3. Fast CLI Reply Tooling (`tools/check_mail.py`)

To attach replies without re-scanning vault notes, `check_mail.py` introduces:

```bash
python tools/check_mail.py --reply-card <id> --text "<markdown>"
```

Execution sequence (<50ms):
1. Loads the target ledger from `--queue` (or `--fixture` when in `--sandbox` mode).
2. Matches the item by ID.
3. Sets `agent_reply`, `agent_reply_at`, and `has_reply = True`.
4. Writes the updated ledger back to disk.
5. Invokes `render_board()` immediately to regenerate the HTML board.

---

## 4. Supported Card Permutations & PCDA Fixtures

The synthetic QA fixture (`assets/fixtures/sandbox_mail_queue.json`) exercises all card permutations:

| Card ID | Priority / Status | Visual Feature Tested | Drawer Content Variant |
| :--- | :--- | :--- | :--- |
| `sbx-01` | P0 PENDING | Top priority alert, `important` tag | **Phoney PCDA Dialectic Matrix** (55Hz Basalt Resonator) |
| `sbx-02` | P3 PENDING | `bloated` tag, `merged_count: 3`, `dependencies: ["sbx-01"]` | **Phoney PCDA Dialectic Matrix** (Submersible Compressor) |
| `sbx-03` | P1 APPROVED | `spike` tag, approved banner, Tier 1 Pro | **Phoney PCDA Dialectic Matrix** (Biological Allpass) |
| `sbx-04` | P2 BACKLOG | `incomplete` tag, cataloged backlog banner | **Backlog Implementation Brief** (Parameters contract & acceptance criteria) |
| `sbx-05` | P3 GRAVEYARD | Graveyard status, rejected rationale banner | **Technical Autopsy & Rejection Rationale** (Audio thread lockup autopsy) |
| `sbx-06` | P0 COMPLETED | Codified banner, 4-pillar tags | **Feature Codification Summary** (AVX2 SIMD benchmarks) |
| `sbx-07` | P1 RESEARCH | Unreferenced new mail, research status | **Research Brief** (Key questions, technical RFC links, benchmarks) |
| `sbx-quick-01` | P2 PENDING | `quick-note` tag, title-only preview | **Zero Reply** (Asserts drawer toggle is cleanly absent) |

---

## 5. Verification Gauntlet & Quality Engineering

1. **Unit & Integration Suite (`test_response_drawer.py`)**:
   - Model validation and Pydantic serialization roundtrip.
   - Drawer DOM container, toggle, ARIA attributes, and inner markdown rendering.
   - Comprehensive card variants and PCDA matrix parsing.
   - Fast CLI `--reply-card` mutation and board regeneration.
   - Multi-file parity across `master_template.html`, `sidecar.css`, and `sidecar.js`.
2. **Sandbox QA Suite (`test_sandbox_board.py`)**:
   - Zero-mutation invariant on live ledger.
   - Deterministic DOM IDs for all 7 populated drawer buttons.
3. **Web QA Linter (`audit_sidecar.py`)**:
   - Tag balance validation on generated HTML.
   - Zero fatal JS syntax hazards or template literal unescaped path sequences.
4. **Universal UTF-8 Strict Audit (`verify_utf8.py --strict`)**:
   - 100% compliance across all Python entrypoints.
