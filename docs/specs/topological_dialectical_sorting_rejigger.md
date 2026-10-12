# Topological Dialectical Sorting: The Universal Rejigger Pattern (`TDS`)

## 1. Architectural Intent & Systems Metaphor

In daily pair-programming across The Klang Suite ecosystem, the operation known colloquially as **"Rejiggering"** represents our core algorithmic and dialectical method for resolving non-linear human ideation into a contiguous execution sequence: **Topological Dialectical Sorting (TDS)**.

Canonical Research Specification:
- [`C:\Dev\Research\agentic\topological_dialectical_sorting_the_rejigger_pattern.md`](file:///C:/Dev/Research/agentic/topological_dialectical_sorting_the_rejigger_pattern.md)

### The Problem It Solves
Standard backlog prioritization (High/Medium/Low bucketing) fails in complex systems engineering because it treats tasks as independent beads on a string. Real software engineering involves deep implicit dependencies: foundational plumbing must precede UI surfaces, schema contracts must precede parsers, and saturation math must precede filter oversampling.

### The Mechanism
1. **The Unranked Bag**: Raw items are gathered into a normalized container (Pydantic `MailItem` or `FlightStage`).
2. **Method B Adversarial Dialectic**: Every candidate is evaluated through Thesis $\to$ Friction $\to$ Antithesis $\to$ 80/20 Resolution (with mandatory *"Where I Disagree With The Prompt"* clauses).
3. **Directed Acyclic Graph (DAG) Resolution**: Dependencies are explicitly mapped ($A \to B$).
4. **Contiguous Indexing ($1..N$)**: The DAG is linearized into a contiguous execution array with an explicit "Why Here?" rationale for each rank.

$$\text{Rejiggering} = \text{TopologicalSort}(\mathcal{G}) \circ \text{HegelianDialectic}(\mathcal{I})$$

```
┌────────────────────────────────────────────────────────────────────────┐
│                   TOPOLOGICAL DIALECTICAL SORTER                       │
├────────────────────┬────────────────────┬──────────────────────────────┤
│ STAGE              │ IMPLEMENTATION     │ FUNCTION                     │
├────────────────────┼────────────────────┼──────────────────────────────┤
│ 1. Intake          │ models.py          │ Unranked normalization       │
│ 2. Dialectic       │ Method B PCDA      │ 80/20 Friction analysis      │
│ 3. DAG Extraction  │ Dependency graph   │ Prerequisite resolution      │
│ 4. Contiguous Rank │ check_mail.py/plan │ Linear 1..N critical path    │
└────────────────────┴────────────────────┴──────────────────────────────┘
```

## 2. Core Invariants
- **Never Execute an Unranked Bag**: Always rejigger an unordered collection before modifying code or compiling.
- **Prerequisites Precede Polish**: Infrastructure and schema contracts always take topological precedence over visual styling or secondary features.
- **Dynamic Re-ranking over Static Overlays**: Ranks are computed dynamically at render time (`idx = 1..N`) based on `(priority_weight, status_weight, date_desc)` rather than maintaining brittle static integer arrays.
