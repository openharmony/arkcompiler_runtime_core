## Why

The existing `memmem` prototype grew ad-hoc around multiple entrypoints, overlapping orchestration paths, and advanced experiments that make the core memory benchmark workflow hard to reason about and extend. This change establishes a focused v1 rewrite with one CLI, one flow format, clear module boundaries, immutable raw evidence, and CSV-only summaries.

## What Changes

- Add a single user-facing `run.py` entrypoint that accepts `--flow` and optional `--out`.
- Add pydantic validation for benchmark flow JSON files, including conservative filename-safe labels.
- Add `.env`-based configuration for `HDC_PATH`.
- Add a typed smaps parser that produces total memory summaries and per-tag breakdowns.
- Add device-local smaps snapshot capture with deferred host transfer after all app flows finish.
- Add a result store layout that preserves received raw smaps snapshots under PID-keyed directories.
- Add CSV report generation:
  - `summary.csv` for total per-snapshot memory metrics.
  - `breakdowns/*.csv` for per-snapshot tag-level metrics.
- Add a lightweight HDC wrapper and device abstraction for app launch, PID resolution, PID validity checks, smaps reading, key events, and file transfer utilities.
- Add command execution for initial commands: `wait`, `snapshot`, and `key`.
- Add a runner that launches each `AppFlow`, stores the resolved PID as runtime identity, executes commands, and generates reports.
- Intentionally exclude advanced legacy features from v1: native allocation tracing, page fingerprinting, A/B runtime replacement, gallery-specific workflows, multi-run statistical analysis, app-specific scenario validation, and multiple Python CLI entrypoints.

## Capabilities

### New Capabilities
- `benchmark-flow`: Defines the v1 benchmark flow interface, command semantics, PID tracking model, and CLI behavior.
- `smaps-analysis`: Defines parsing of `/proc/<pid>/smaps` into total memory profiles and per-tag breakdowns.
- `result-evidence`: Defines output layout, raw snapshot preservation, metadata storage, and CSV report generation.
- `device-hdc`: Defines HDC-backed device operations required by the benchmark runner.

### Modified Capabilities

None.

## Impact

Affected code and project areas:

- `run.py` becomes the only user-facing CLI.
- New `src/` package contains framework modules.
- New `test/` suite validates smaps parsing, schema/config loading, result storage, CSV reporting, and CLI argument behavior.
- `requirements.txt` may gain an env-file parsing library if used.
- Runtime host side-program dependencies remain limited to Python and `hdc`; Python package dependencies must be recorded in `requirements.txt`.
