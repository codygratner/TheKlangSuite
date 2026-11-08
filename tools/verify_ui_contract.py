#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/verify_ui_contract.py - CrackedChimesUI static scan + UI contract gate (Phase 1)
Gates (PLAN.md S9.3 / S14):
  (a) ERROR  DiceButton / diceButton_ symbols
  (b) ERROR  juce::String::formatted / String::formatted with %s or %c
  (c) WARN   drawText/drawFittedText inside Visualizer/Canvas/Oled/Scope classes
  (d) ERROR  legacy UIComponents.h include / RotaryKnobSlider
Also validates contracts/ui/<plugin>_ui.yaml when present.
Usage: --plugin <name> | --all  [--strict]
"""
import sys
import re
import argparse
from pathlib import Path
from typing import List, Literal, Optional

from pydantic import BaseModel, Field, model_validator

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "source"
PLUGINS = ["hastur", "nyarlat", "shoggoth", "digistrike_fs2", "formless"]


# ---------------- Pydantic UI contract models ----------------
class KnobModel(BaseModel):
    id: str
    size: int = Field(..., ge=52)
    hero: bool = False

    @model_validator(mode="after")
    def hero_floor(self):
        if self.hero and self.size < 72:
            raise ValueError(f"Hero knob '{self.id}' size {self.size} < 72")
        return self


class RingBindingModel(BaseModel):
    param: str
    source: str
    pip_telemetry: Optional[str] = None

    @model_validator(mode="after")
    def need_pip(self):
        if self.source != "xmod" and not self.pip_telemetry:
            raise ValueError(f"Ring binding '{self.param}' (source={self.source}) requires pip_telemetry")
        return self


class OledModel(BaseModel):
    id: str
    text: Literal[False] = False


class UIContractModel(BaseModel):
    plugin: str
    knobs: List[KnobModel] = []
    rings: List[RingBindingModel] = []
    oleds: List[OledModel] = []


# ---------------- Static scan ----------------
RE_DICE = re.compile(r"\b(DiceButton\w*|diceButton_\w*)")
RE_FMT = re.compile(r"String::formatted\s*\([^;]*%[-0-9.]*[sc]")
RE_LEGACY = re.compile(r'#\s*include\s*[<"][^>"]*UIComponents\.h[>"]|\bRotaryKnobSlider\b')
RE_CLASS = re.compile(r"\b(?:class|struct)\s+(\w+)|\b(\w+)::\w+\s*\(")
RE_DRAW = re.compile(r"\b(drawText|drawFittedText)\s*\(")
VIS = re.compile(r"Visualizer|Canvas|Oled|Scope")


def files_for(plugin: str) -> List[Path]:
    exts = (".h", ".cpp")
    if plugin == "formless":
        root_files = [p for p in SRC.glob("Formless*") if p.suffix in exts]
        sub = SRC / "formless_spawn"
        sub_files = [p for p in sub.rglob("*") if p.suffix in exts] if sub.exists() else []
        return sorted(set(root_files + sub_files))
    d = SRC / plugin
    return sorted(p for p in d.rglob("*") if p.suffix in exts) if d.exists() else []


def scan(plugin: str):
    errs, warns = [], []
    for f in files_for(plugin):
        try:
            lines = f.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            continue
        rel = f.relative_to(ROOT).as_posix()
        cur_cls = ""
        for i, ln in enumerate(lines, 1):
            s = ln.strip()
            if s.startswith("//"):
                continue
            m = RE_CLASS.search(ln)
            if m:
                cur_cls = m.group(1) or m.group(2) or cur_cls
            if RE_DICE.search(ln):
                errs.append(f"{rel}:{i}: [a] dice symbol: {RE_DICE.search(ln).group(1)}")
            if RE_FMT.search(ln):
                errs.append(f"{rel}:{i}: [b] String::formatted with %s/%c")
            if RE_LEGACY.search(ln):
                errs.append(f"{rel}:{i}: [d] legacy UIComponents.h/RotaryKnobSlider")
            if RE_DRAW.search(ln) and VIS.search(cur_cls or ""):
                warns.append(f"{rel}:{i}: [c] text draw in visualizer class {cur_cls}")
    return errs, warns


def check_ui_yaml(plugin: str) -> List[str]:
    p = ROOT / "contracts" / "ui" / f"{plugin}_ui.yaml"
    if not p.exists():
        print(f"    (no {p.relative_to(ROOT).as_posix()} - skipped)")
        return []
    import yaml
    with open(p, "r", encoding="utf-8") as fh:
        data = yaml.safe_load(fh)
    try:
        UIContractModel.model_validate(data)
        return []
    except Exception as e:  # pydantic ValidationError
        return [f"{p.name}: {e}"]


def main():
    ap = argparse.ArgumentParser(description="CrackedChimesUI static + UI contract gate")
    ap.add_argument("--plugin", choices=PLUGINS)
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--strict", action="store_true", help="warnings fail")
    a = ap.parse_args()
    targets = PLUGINS if a.all or not a.plugin else [a.plugin]
    tot_e = tot_w = 0
    for pl in targets:
        e, w = scan(pl)
        e += check_ui_yaml(pl)
        print(f"[*] {pl}: {len(e)} error(s), {len(w)} warning(s)")
        for x in e:
            print(f"  ERROR {x}")
        for x in w:
            print(f"  WARN  {x}")
        tot_e += len(e)
        tot_w += len(w)
    fail = tot_e > 0 or (a.strict and tot_w > 0)
    print(f"[=] {'FAIL' if fail else 'PASS'}: {tot_e} errors, {tot_w} warnings")
    sys.exit(1 if fail else 0)


if __name__ == "__main__":
    main()
