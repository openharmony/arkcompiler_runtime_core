## Why

JSON benchmark flows are cumbersome for repetitive and complex scenarios, especially scroll/sample sequences that repeat command patterns many times. A supported programmatic interface lets users generate validated flows with ordinary Python loops and helpers while preserving the existing CLI workflow.

## What Changes

- Add a public `lib.py` facade for programmatic benchmark usage.
- Add thin command and flow builder functions that construct the existing validated Pydantic schema models.
- Add public HDC/device construction helpers in `lib.py` so documented user scripts do not import from `src.*`.
- Change the public programmatic run API to accept an explicit validated flow object plus device and keyword-only runtime options.
- Revalidate the accepted flow at the `lib.run()` boundary before execution.
- **BREAKING**: `BenchmarkOptions` no longer carries `flow_path`; flow content is passed separately to the runner.
- **BREAKING**: output `flow.json` becomes canonical validated flow JSON instead of a byte-for-byte copy of the input file.
- Keep `run.py` as a CLI-only adapter that reads JSON, validates it into a flow, and calls `lib.run()`.

## Capabilities

### New Capabilities
- `programmatic-interface`: Public Python interface for constructing flows, creating device handles, and running benchmarks programmatically.

### Modified Capabilities
- `benchmark-flow`: Flow content is accepted as a validated object by the runner/API, while CLI remains a JSON adapter.
- `result-evidence`: Output `flow.json` records canonical validated flow JSON rather than copying the input file.
- `testing-support`: Tests cover the public facade, flow revalidation, and CLI-to-library adapter boundary.

## Impact

- Adds `lib.py` as the documented Python API surface.
- Updates `run.py` and `src/runner.py` to keep CLI/config handling in the CLI layer and runtime options in the runner layer.
- Updates output initialization to serialize the validated flow model.
- Updates README examples for programmatic flow construction and CLI behavior.
- Existing CLI usage remains supported: `python run.py --flow flow.json`.
