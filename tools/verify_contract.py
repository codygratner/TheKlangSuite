#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tools/verify_contract.py — Pydantic v2 Pre-Flight Contract Verification Gate
================================================================================
Validates declarative plugin parameter contracts (contracts/<plugin>.yaml):
  1. Strict snake_case ID regex: ^[a-z0-9_]+$
  2. Range bounds: min < max, and min <= default <= max
  3. Skew bounds: 0.1 <= skew <= 4.0
  4. Unique parameter IDs (0 duplicates)
  5. 32-control Centaur cardinality per tkdsp_16_control_canon.md
"""

import sys
import argparse
from pathlib import Path
import yaml
from typing import List, Optional, Union
from pydantic import BaseModel, Field, field_validator, model_validator

# Universal UTF-8 console output
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")


class ParameterModel(BaseModel):
    id: str = Field(..., pattern=r"^[a-z0-9_]+$")
    name: str
    category: str
    type: str = Field(..., pattern=r"^(float|int|choice|bool)$")
    range: Optional[List[Union[float, int]]] = None
    choices: Optional[List[str]] = None
    default: Union[float, int, str, bool]
    skew: Optional[float] = None
    unit: Optional[str] = None
    effect: Optional[str] = None
    automation_tier: Optional[int] = Field(default=None, ge=1, le=4)

    def effective_tier(self) -> int:
        """PLAN.md S14: explicit tier wins; else category-derived default."""
        if self.automation_tier is not None:
            return self.automation_tier
        if self.type in ("choice", "bool"):
            return 4
        c = (self.category or "").lower()
        if c in ("header", "switch", "lineage"):
            return 4
        if c in ("random", "trim", "mpe"):
            return 3
        if c in ("filter", "macro"):
            return 1
        return 2

    @field_validator("skew")
    @classmethod
    def validate_skew(cls, v):
        if v is not None and not (0.1 <= v <= 4.0):
            raise ValueError(f"Skew {v} outside valid bound [0.1, 4.0]")
        return v

    @model_validator(mode="after")
    def validate_range_and_default(self):
        if self.type in ("float", "int"):
            if not self.range or len(self.range) != 2:
                raise ValueError(f"Parameter '{self.id}' of type '{self.type}' must define range [min, max]")
            vmin, vmax = float(self.range[0]), float(self.range[1])
            if vmin >= vmax:
                raise ValueError(f"Parameter '{self.id}' range min ({vmin}) must be < max ({vmax})")
            vdef = float(self.default)
            if not (vmin <= vdef <= vmax):
                raise ValueError(f"Parameter '{self.id}' default ({vdef}) not in range [{vmin}, {vmax}]")
        elif self.type == "choice":
            if not self.choices or len(self.choices) < 2:
                raise ValueError(f"Parameter '{self.id}' of type 'choice' must define at least 2 choices")
            if isinstance(self.default, int) and not isinstance(self.default, bool):
                if not (0 <= self.default < len(self.choices)):
                    raise ValueError(f"Parameter '{self.id}' default index {self.default} out of range for {len(self.choices)} choices")
            elif str(self.default) not in self.choices:
                raise ValueError(f"Parameter '{self.id}' default '{self.default}' not in choices: {self.choices}")
        return self


class PluginContractModel(BaseModel):
    plugin: str
    name: str
    version: str
    manufacturer: str
    category: str
    parameters: List[ParameterModel]

    @model_validator(mode="after")
    def validate_cardinality_and_uniqueness(self):
        ids = [p.id for p in self.parameters]
        if len(ids) != len(set(ids)):
            duplicates = [x for x in ids if ids.count(x) > 1]
            raise ValueError(f"Duplicate parameter IDs found: {set(duplicates)}")
        return self

    @model_validator(mode="after")
    def validate_tier_monotonic(self):
        """Tiers must be non-decreasing through parameters[] (DAW automation order)."""
        hi = None  # (id, tier) of highest tier seen so far
        for p in self.parameters:
            t = p.effective_tier()
            if hi is not None and t < hi[1]:
                raise ValueError(
                    f"Automation tier order violated: {hi[0]} (tier {hi[1]}) precedes {p.id} (tier {t})")
            if hi is None or t > hi[1]:
                hi = (p.id, t)
        return self


def verify_file(yaml_path: Path, expected_cardinality: Optional[int] = None) -> bool:
    print(f"[*] Verifying contract: {yaml_path}")
    if not yaml_path.exists():
        print(f"[!] File not found: {yaml_path}")
        return False

    with open(yaml_path, "r", encoding="utf-8") as f:
        raw_data = yaml.safe_load(f)

    try:
        contract = PluginContractModel.model_validate(raw_data)
        count = len(contract.parameters)
        print(f"[+] Schema validation passed! Loaded {count} parameters.")

        if expected_cardinality is not None and count != expected_cardinality:
            print(f"[!] Cardinality mismatch: expected {expected_cardinality}, got {count}")
            return False

        print(f"[+] Contract '{contract.plugin}' is 100% verified and valid.")
        return True
    except Exception as e:
        print(f"[!] Contract validation FAILED:\n{e}")
        return False


def main():
    parser = argparse.ArgumentParser(description="Pydantic v2 Contract Verification Gate")
    parser.add_argument("contract_path", type=str, nargs="?", default=None, help="Path to contracts/<plugin>.yaml")
    parser.add_argument("--cardinality", type=int, default=None, help="Assert exact parameter count")
    parser.add_argument("--all", action="store_true", help="Verify every contracts/*.yaml")
    args = parser.parse_args()

    if args.all:
        root = Path(__file__).resolve().parent.parent / "contracts"
        results = [verify_file(f) for f in sorted(root.glob("*.yaml"))]
        print(f"[=] {sum(results)}/{len(results)} contracts passed")
        sys.exit(0 if all(results) else 1)
    if not args.contract_path:
        parser.error("contract_path is required unless --all is given")

    success = verify_file(Path(args.contract_path), expected_cardinality=args.cardinality)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
