## Context

The benchmark currently has a single documented user path: `python run.py --flow flow.json`. That keeps execution reproducible, but authoring JSON flows is verbose for real scenarios that repeat patterns such as wait, fling, snapshot, and screenshot many times. Existing local flow files show large repeated command blocks that would be easier to express with Python loops.

Internally, runtime options previously carried `flow_path`, and `src.runner.run_benchmark()` loaded and validated JSON from that path. That coupled scenario content to runtime options and prevented a clean programmatic API. The output `flow.json` was copied from the input file, which worked only for CLI-originated flows and did not represent programmatically generated scenarios.

## Goals / Non-Goals

**Goals:**

- Provide `lib.py` as the documented public programmatic interface.
- Keep `run.py` as a CLI-only adapter for existing JSON flows.
- Let users construct validated flows using thin Python wrappers around the existing Pydantic schema models.
- Keep `src.*` undocumented as public API.
- Keep `BenchmarkOptions` and `ResultStore` internal to the implementation.
- Make `lib.run()` accept flow content explicitly, revalidate it, execute the benchmark, generate reports, and return `None` on success.
- Require programmatic callers to pass `out_dir` explicitly.
- Preserve CLI defaults: `reboot=False`, `logs=True`.
- Write canonical validated flow JSON to `<out_dir>/flow.json` for both CLI and programmatic runs.

**Non-Goals:**

- No predefined app registry or app catalog helpers.
- No high-level scenario DSL beyond thin wrappers over existing Pydantic models.
- No hidden defaults in builder wrappers unless the underlying schema model already has defaults.
- No public exposure of `BenchmarkOptions`, `ResultStore`, `Flow`, `AppFlow`, `Device`, or `Hdc` as documented user-facing names.
- No change to command semantics, output artifact layout besides canonical `flow.json`, or report generation metrics.

## Decisions

### `lib.py` is the public programmatic facade

Expose a lean interface from repository-root `lib.py`:

```python
get_hdc(hdc_exe_path: str | pathlib.Path)
get_device(hdc)
run(flow, device, *, out_dir: str | pathlib.Path, reboot: bool = False, logs: bool = True) -> None

flow(apps)
app(label, bundle, ability, terminate, commands)
wait(seconds)
snapshot(label)
screenshot(label)
key(name)
tap(x_pct, y_pct)
double_tap(x_pct, y_pct)
long_tap(x_pct, y_pct)
swipe(x1_pct, y1_pct, x2_pct, y2_pct, velocity)
drag(x1_pct, y1_pct, x2_pct, y2_pct, velocity)
fling(x1_pct, y1_pct, x2_pct, y2_pct, velocity, step_length)
directional_fling(direction, velocity, step_length)
input_text(x_pct, y_pct, text)
text(text)
```

The wrappers construct the existing Pydantic models immediately, so invalid labels, invalid coordinates, invalid velocities, or duplicate labels fail through the same validation system already used by JSON flows.

**Alternative considered:** expose schema classes directly from `lib.py`. Rejected for now to keep the public interface small and avoid making model class names part of the documented API.

### `run.py` remains CLI-only

`run.py` should parse arguments, load `.env`, construct HDC/device, load and validate the JSON flow, and call `lib.run()`. It should not be the documented Python API.

**Alternative considered:** keep `run.py::run` as the public API. Rejected because mixing CLI and library responsibilities makes imports and documentation less clear.

### `BenchmarkOptions` lives in the runner layer

`lib.run()` receives runtime options as keyword-only arguments and constructs the internal `BenchmarkOptions` object before calling the runner. `BenchmarkOptions` lives in `src.runner`, no longer contains `flow_path`, and contains only runtime execution settings such as `out_dir`, `reboot`, and `logs`.

**Alternative considered:** expose `BenchmarkOptions` through `lib.py`. Rejected because callers can pass the small option set directly, and keeping the dataclass internal avoids exposing more implementation structure.

### `ResultStore` remains internal

`lib.run()` returns `None` on success and raises on failure. Programmatic callers already passed `out_dir`, so they can inspect that path directly after a successful run.

**Alternative considered:** return `ResultStore`. Rejected to avoid making `ResultStore` public API.

### The runner accepts a validated `Flow`

Change `src.runner.run_benchmark()` to accept a `Flow` object directly, in addition to internal options, store, and device. Flow loading from JSON moves to `run.py`.

`lib.run()` revalidates the provided flow before execution. This protects the execution boundary when a user mutates a Pydantic model after construction and breaks model-level invariants.

### Output `flow.json` is canonical validated JSON

Output initialization should write `validated_flow.model_dump_json(indent=2)` to `<out_dir>/flow.json` rather than copying an input file. This makes CLI and programmatic runs produce equivalent reproducibility evidence.

## Risks / Trade-offs

- Public wrappers may not satisfy every advanced scenario → Advanced users can still import internals, but docs keep the supported surface small.
- Revalidating a Pydantic model adds a small overhead → Flow sizes are tiny relative to device benchmark execution.
- Canonical `flow.json` is not byte-for-byte identical to CLI input → Existing users do not depend on exact input formatting, and canonical output is better evidence.
- `lib.py` at repository root is simple but not a full package boundary → Acceptable for the current project shape; packaging can be revisited later.

## Migration Plan

1. Add `lib.py` facade and README programmatic example.
2. Move internal `BenchmarkOptions` into `src.runner` and remove `flow_path`.
3. Update `run_benchmark()` to accept `Flow` directly.
4. Update `run.py` to load/validate JSON and call `lib.run()`.
5. Update output initialization to write canonical validated flow JSON.
6. Keep existing CLI flags and defaults unchanged.
7. Update tests for CLI compatibility, programmatic API, revalidation, and canonical flow output.

## Open Questions

None currently. Deferred future work: predefined app registry helpers and higher-level scenario combinators.
