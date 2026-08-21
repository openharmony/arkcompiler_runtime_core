## 1. Public Facade

- [x] 1.1 Add root-level `lib.py` as the documented programmatic API module.
- [x] 1.2 Add `get_hdc(hdc_exe_path: str | pathlib.Path)` that normalizes the path and returns an HDC handle.
- [x] 1.3 Add `get_device(hdc)` that returns a device handle usable by `lib.run()`.
- [x] 1.4 Add `lib.run(flow, device, *, out_dir, reboot=False, logs=True) -> None` that revalidates flow, constructs internal runtime options, creates the result store, runs the benchmark, and generates reports.

## 2. Flow and Command Builders

- [x] 2.1 Add `flow(apps)` wrapper that accepts app models and returns a validated flow model.
- [x] 2.2 Add `app(label, bundle, ability, terminate, commands)` wrapper with all fields explicit.
- [x] 2.3 Add basic command wrappers for `wait`, `snapshot`, `screenshot`, and `key`.
- [x] 2.4 Add coordinate command wrappers for `tap`, `double_tap`, `long_tap`, and `input_text`.
- [x] 2.5 Add swipe-like command wrappers for `swipe`, `drag`, `fling`, and `directional_fling`.
- [x] 2.6 Add focused text command wrapper for `text`.
- [x] 2.7 Ensure wrappers construct existing Pydantic schema models immediately and do not add new defaults.

## 3. Runner and Options Refactor

- [x] 3.1 Move internal `BenchmarkOptions` into `src.runner` and remove `flow_path` so it only carries `out_dir`, `reboot`, and `logs`.
- [x] 3.2 Change `src.runner.run_benchmark()` to accept a validated `Flow` object separately from options, store, and device.
- [x] 3.3 Move JSON flow loading and validation out of `src.runner.run_benchmark()`.
- [x] 3.4 Update execution context construction to use the provided validated flow.
- [x] 3.5 Update result store creation and runtime option call sites for the new options shape.

## 4. CLI Adapter

- [x] 4.1 Keep `run.py` as the CLI entrypoint with existing arguments and defaults.
- [x] 4.2 Update `run.py` to load and validate `--flow` JSON into a flow model before device actions.
- [x] 4.3 Update `run.py` to call `lib.run(flow, device, out_dir=..., reboot=..., logs=...)`.
- [x] 4.4 Keep CLI error handling behavior without exposing `src.*` as documented API.

## 5. Canonical Flow Evidence

- [x] 5.1 Change local output initialization to receive the validated flow object.
- [x] 5.2 Write `<out_dir>/flow.json` from `flow.model_dump_json(indent=2)` instead of copying the input file.
- [x] 5.3 Ensure CLI and programmatic runs produce the same canonical flow evidence shape.

## 6. Tests

- [x] 6.1 Add tests for public builder wrappers constructing valid flows and commands.
- [x] 6.2 Add tests for public builder validation failures.
- [x] 6.3 Add tests for `lib.run()` success returning `None` and writing output under explicit `out_dir`.
- [x] 6.4 Add tests that `lib.run()` revalidates a mutated invalid flow before device actions.
- [x] 6.5 Update runner tests to pass flow objects directly instead of relying on `flow_path` in options.
- [x] 6.6 Keep CLI tests covering parser defaults and JSON flow error handling.
- [x] 6.7 Add or update tests for canonical output `flow.json` from both CLI and programmatic paths.

## 7. Documentation and Verification

- [x] 7.1 Update README CLI output documentation to describe canonical validated `flow.json`.
- [x] 7.2 Add README programmatic API example using only `lib.py` imports.
- [x] 7.3 Document that users write their own Python helpers for repeated scenarios.
- [x] 7.4 Run `source ".venv/bin/activate" && make test`.
- [x] 7.5 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 7.6 Run `openspec validate "programmatic-interface"`.
