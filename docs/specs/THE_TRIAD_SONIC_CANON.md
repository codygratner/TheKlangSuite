# The Triad Sonic Canon: Synthetic, Organic, Cyborg
*(The Master Signature Tone Philosophy of The Cult of Klang)*

## 🧭 Core Philosophy & Founding Doctrine
Across all synthesizers, drum machines, audio effects, and DSP processors created for **The Klang Suite (`TKS`)**, **TheKlangDSP (`TKDSP`)**, and **ToadTracker (`TT`)**, sound design and circuit modeling are anchored to a tripartite aesthetic signature:

> *"Clean virtual analog for **Synthetic**, sloshy tube-back warmth with LPGs all over for **Organic**, and built out of nuclear reactor waste for **Cyborg**."*

This triad is the unmistakable fingerprint of Klang instruments. While individual plugins may implement the triad as a 3-way toggle (like Hastur HS-01's *Lifeform*), a continuous tri-polar morph axis, or distinct selectable voice engines, every instrument must embody these three sonic personalities.

---

## 🏛️ The Three Signature Pillars

### 1. 🔷 SYNTHETIC — *"Clean Virtual Analog"*
*The archetype of crystalline mathematical order, laser-focused punch, and immaculate headroom.*

* **Sonic Signature**: Pristine fundamental tracking, zero sub-harmonic jitter, surgically defined transients, wide high-headroom stereophony, and clinical punch without low-end mud.
* **Oscillator Core**:
  - Exact 2-point PolyBLEP or MinBLEP band-limiting on all discontinuity edges (saw, square, pulse, hard sync).
  - High-precision phase accumulators with zero sub-sample jitter on sync wraps.
  - Transparent harmonic morphs (pure geometric transformations, Padé sine wrapping).
* **Filter Topologies**:
  - Zero-Delay Feedback (ZDF) linear/algebraic topologies (4-pole cascades, State Variable Filters).
  - High dynamic headroom with soft algebraic clipping ($\tanh$ or Padé rational approximations) that maintains $>70\text{ dB}$ harmonic purity until driven hard.
  - Resonance that sings as a pure, focused sinusoidal pitch without self-destructive collapse.
* **Dynamics & Envelopes**:
  - Lightning-fast transconductance OTA VCAs (1985 Roland-style).
  - Pure exponential/linear slew limiting (2–4ms click-free settling).
  - Zero amplitude bleed; instantaneous cutoff when gate releases.
* **Visual & Industrial Identity**:
  - **Color**: Tron Blue / Pure Cyan (`#00C8FF`).
  - **Aesthetic**: Polished brushed aluminum, crisp high-contrast OLED vectors, clean engineering readouts.

---

### 2. 🌿 ORGANIC — *"Sloshy Tube-Back Warmth with LPGs All Over"*
*The archetype of living physical matter, thermal inertia, acoustic decay, and spongy optocoupler droop.*

* **Sonic Signature**: Warm golden glow, breathing dynamics, spongy compression under load, natural acoustic dampening, and the unmistakable tactile "plop and droop" of light-dependent resistors.
* **Oscillator Core**:
  - Non-linear thermal drift (inter-voice micro-pitch wander via smooth Runge-Kutta or low-frequency random walks).
  - Soft cathode-follower buffer saturation that gently rounds off sharp transient peaks.
  - Parabolic wave fillets on triangle and saw morphs, simulating analog integrator slewing.
* **Filter Topologies & The LPG Fleet**:
  - **"LPGs all over"**: Dedicated vactrol-based Low Pass Gates (Buchla 292 / Meng Qi DPLPG / VTL5C3 dual-exponential optical CdS models).
  - Natural acoustic dampening where cutoff frequency organically hugs amplitude (higher frequencies die out faster as the vactrol settles).
  - Asymmetric triode tube feedback loops (12AX7 emulation) generating rich even-order 2nd and 4th harmonics.
  - Sub-audio DC breathing: Sample-rate normalized 4Hz leaky DC decoupling simulating physical cathode bypass capacitors.
* **Dynamics & Envelopes**:
  - Asymmetric optical response: Snappy 2–4ms strike attack followed by a two-stage sluggish droop (fast 28ms initial decay transitioning into a 120–250ms phosphorescent tail).
  - Spongy power-supply sag under sustained high-gain notes.
* **Visual & Industrial Identity**:
  - **Color**: Bioluminescent Forest Green / Warm Moss (`#6BBF59`).
  - **Aesthetic**: Bakelite chassis, amber glowing tube filament windows, soft curved bezel cards.

---

### 3. ☢️ CYBORG — *"Built Out of Nuclear Reactor Waste"*
*The archetype of Chernobyl fallout, mutant industrial decay, screaming diode clippers, and weaponized digital artifacting.*

* **Sonic Signature**: Toxic, radioactive, abrasive, rusted, and unhinged. Vicious intermodulation distortion, screaming self-oscillation that devours the fundamental, and broken digital clocks bleeding into analog rails.
* **Oscillator Core**:
  - Deliberately degraded clock dividers (sub-oscillators operating from starved timers or unstable PLLs).
  - Truncated phase accumulation and 6-to-8-bit DAC ladder quantization (64 to 256 discrete voltage steps).
  - Intentionally calibrated single-point BLEP edge tearing ($65\%$ scaling) to produce razor-sharp harmonic bite without aliasing into Nyquist mud.
  - Brutal Casio CZ-style illegal phase-distortion and harsh cross-modulation.
* **Filter Topologies & Waveshaping**:
  - Starved op-amp rail foldback: Hard rail collisions that fold back inward instead of clipping cleanly.
  - Screaming diode clippers (Korg MS-10 / MS-20 OTA abuse) driven into violent self-oscillation.
  - Non-linear Infinifold wavefolders with dynamic parabolic fillets that crackle like Geiger counters under heavy drive.
* **Dynamics & Envelopes**:
  - **Leaky Passive Vactrols**: A persistent $-30\text{ dB}$ to $-40\text{ dB}$ bleed floor that refuses to fully silence, oozing background hiss and radioactive hum.
  - Jagged staircase control-voltage quantization (stepping LFOs and envelopes).
  - Sub-sample charge-injection transient spikes ($20\mu\text{s}$ impulse bursts on gate transitions).
* **Visual & Industrial Identity**:
  - **Color**: Hazardous Rad-Waste Yellow / Industrial Toxic Amber (`#E08C00` / `#FFE600`).
  - **Aesthetic**: Heavy construction equipment patina, yellow/black hazard striping, radiation trefoils, stamped industrial warning stencils.

---

## 🎛️ The Universal Cross-Product Translation Matrix

When applying the Triad across different product pillars, translate the characteristics as follows:

| Category | 🔷 SYNTHETIC | 🌿 ORGANIC | ☢️ CYBORG |
| :--- | :--- | :--- | :--- |
| **Synthesizers** *(Hastur, TKF, TT Synths)* | High-precision PolyBLEP DCOs, 4-pole ZDF IR3R05, clean Roland OTA VCA | Thermally drifted DCOs, resonant tube LPGs, cathode-follower saturation | Clock-starved DCOs, MS-10 diode scream, Infinifold, leaky -38dB LPG |
| **Drum Engines** *(MME-BD, MME-SD, etc.)* | Pure mathematical pitch-dives (Syncussion/Machinedrum TRX), punchy click-free snap | Resonant analog membrane (Jomox MBrane, Vermona twin-T), vactrol droop decay | Pērkons BBD clock whine, Machinedrum SRR/BRR decimation, overdriven Germanium bite |
| **Audio Effects** *(The Klang Mill, Mega-FX)* | Studio-grade transparent algorithmic delays/reverbs, linear-phase multiband comp | BBD/Tape analog bucket-brigade, optical opto-compressor, warm tube preamp drive | Corrupted buffer glitch slicers, radioactive wavefolders, bit-crushed feedback loops |
| **Modulation / LFOs** *(TKC, TT Quadramod)* | Exact sine/triangle/saw LFOs, mathematical slew, rigid BPM sync | Asymmetric wander, chaos-pendulum LFOs, optocoupler slew lag | Stepped sample & hold, sample-rate decimated ramps, erratic glitch triggers |

---

## 🔒 Engineering & Code Invariants

1. **Clean-Room Code Representation**:
   - In C++ parameter layouts, the mode enum MUST be represented as:
     ```cpp
     enum class Lifeform {
         Synthetic = 0, // Clean Virtual Analog
         Cyborg    = 1, // Nuclear Reactor Waste
         Organic   = 2  // Sloshy Tube-Back Warmth & LPGs
     };
     ```
   - In UI color definitions, the three primary accent hues are pinned to:
     - `kSynthetic = juce::Colour(0xFF00C8FF);` // Tron Blue
     - `kCyborg    = juce::Colour(0xFFE08C00);` // Toxic Amber / Rad-Waste Yellow
     - `kOrganic   = juce::Colour(0xFF6BBF59);` // Warm Bioluminescent Green
2. **Never Slop the Synthetic**: Synthetic mode must never introduce unrequested pitch drift, DC offset, or harmonic dirt. It is the benchmark of pure engineering precision.
3. **Never Fake the Organic**: Organic mode must use true physical/optical modeling principles (dual-exponential lag curves, frequency-dependent damping, even-order tube curvature), not just a generic high-shelf rolloff.
4. **Calibrate the Nuclear Waste**: Cyborg mode is aggressive, but it is **musical**. Edge tearing and wavefolding must be band-limited to prevent unpitched white noise or Nyquist foldover explosions. It should sound like an industrial machine operating at the brink of meltdown, not broken code.
