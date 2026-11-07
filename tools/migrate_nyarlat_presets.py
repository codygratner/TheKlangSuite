import glob
import json
import math
from pathlib import Path

def migrate_presets():
    preset_dir = Path(r"C:/Dev/TheKlangSuite/assets/presets/nyarlat")
    preset_files = list(preset_dir.glob("**/*.json"))
    print(f"Migrating {len(preset_files)} presets in {preset_dir}...")

    purged_keys = {'reverb_time', 'reverb_damp', 'reverb_mix', 'rand_cutoff'}
    timbre_types = ['morph', 'odd_even', 'tilt', 'formant', 'mask', 'speed']

    migrated_count = 0
    for pf in preset_files:
        with open(pf, 'r', encoding='utf-8') as f:
            data = json.load(f)

        params = data.get('parameters', {})
        modified = False

        # 1. Purge reverb and rand_cutoff
        for pk in purged_keys:
            if pk in params:
                del params[pk]
                modified = True

        # 2. Ensure chorus_rate exists
        if 'chorus_rate' not in params:
            params['chorus_rate'] = 1.2
            modified = True

        # 3. Ensure rand_chorus_rate exists
        if 'rand_chorus_rate' not in params:
            params['rand_chorus_rate'] = 0.0
            modified = True

        # 4. Collapse 18 oscillator chaos attenuverters into 6 macros
        for t in timbre_types:
            macro_key = f"rand_{t}"
            old_keys = [f"rand_o1_{t}", f"rand_o2_{t}", f"rand_o3_{t}"]
            vals = [params.get(k, 0.0) for k in old_keys if k in params]

            if vals:
                # RMS collapse with sign of sum
                s = sum(vals)
                rms = math.sqrt(sum(v*v for v in vals) / len(vals))
                params[macro_key] = math.copysign(rms, s) if s != 0 else 0.0
                modified = True
            elif macro_key not in params:
                params[macro_key] = 0.0
                modified = True

            for k in old_keys:
                if k in params:
                    del params[k]
                    modified = True

        if modified:
            with open(pf, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2)
            migrated_count += 1

    print(f"Migration complete: {migrated_count} presets updated.")

if __name__ == '__main__':
    migrate_presets()
