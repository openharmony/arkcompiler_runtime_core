# memmem

Small OpenHarmony memory benchmark runner. It launches configured apps, executes flow commands, captures `/proc/<pid>/smaps` and screenshots, receives raw evidence, and writes CSV reports and SVG memory trend plots.

## Authors

- Nazarov Konstantin
- Petrov Igor

## Prerequisites

- Python 3.11
- HDC available on host
- Connected OpenHarmony device with `uitest`, `aa`, `pidof`, `cat`, `date`, `mkdir`, and `rm`

Create `.env`:

```bash
HDC_PATH=/path/to/hdc
```

## Setup

```bash
make venv
source .venv/bin/activate
```

## Run (run.py)

```bash
python run.py --flow flow.json
```

- `--flow`: benchmark flow JSON; may contain repeat macros in the unprocessed form; output `flow.json` is the canonicalized validated expanded flow and may differ in formatting from the input file
- `--out-dir`: directory to store benchmark results; when omitted, a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory is created; the directory must not exist yet
- `--repeats`: number of times to repeat the flow to collect averaged measurements; a positive int, defaults to `1`
- `--reboot` / `--no-reboot`: reboot before running the flow; defaults to `--no-reboot`
- `--hilog` / `--no-hilog`: collect one run-wide device hilog artifact; defaults to `--hilog`
- `--memmem-log-level`: memmem app log level, one of `info`, `warn`, or `err`; defaults to `err`
- `--memmem-log-file`: optional memmem app log file; when omitted, info/warnings go to stdout and errors go to stderr; when set, all emitted memmem logs overwrite that file. Passing an empty string is invalid: **breaking change** from previous behavior where `""` routed to stdout/stderr and `None`/omitted now does that instead
- `--smaps-filter`: Regex matched with `re.match` against each normalized smaps tag; Only matching mappings are counted in reports; E.g. `.*\.so` counts contribution of only shared-library mappings
- output is written to a timestamped `memmem-out-YYYYMMDD_HHMMSS_microseconds` directory

Path arguments (`--flow`, `--out-dir`, `--memmem-log-file`, and `HDC_PATH` from `.env`) are resolved against the directory the CLI was invoked from unless the path is absolute.

## Record (record.py)

```bash
python record.py --timeout 30
```

`record.py` records UI inputs from the connected device with `uitest uiRecord` and writes a generated `flow-YYYYMMDD_HHMMSS_microseconds.json` file.

- optional `--timeout`: recording duration in seconds; if omitted, recording continues until you type `y` and press Enter
- script will prompt user for input when ready
- generated flows contain UI input commands only; add waits, snapshots, and screenshots manually
- generated `AppFlow` entries preserve recorded bundle/ability identity order and always use `terminate: false`
- if a recorder event has empty bundle or ability metadata, the generated field remains `""` and the script prints a warning; edit those fields manually

## Flow schema

Flow JSON has two forms. The **canonical form** is what the runner executes and what result evidence stores; it is also the output of preprocessing. The **unprocessed form** additionally accepts macros. `python run.py --flow flow.json` accepts either canonical or unprocessed form and runs preprocessing step to canonicalize.

### Canonical form

```json
{
  "$desc": "Measures memory after launch and one tap.",
  "flow": [
    {
      "label": "APP_A",
      "bundle": "com.example.app",
      "ability": "EntryAbility",
      "terminate": true,
      "commands": [
        { "action": "wait", "payload": 2.1 },
        { "action": "tap", "payload": { "x_pct": 50, "y_pct": 80 } },
        { "action": "screenshot", "payload": "after_tap" },
        { "action": "snapshot", "payload": "after_tap" },
        { "action": "key", "payload": { "key": "Home" } }
      ]
    }
  ]
}
```

`$desc` is optional top-level metadata field and does not affect execution.

When `terminate` is `true`, the runner terminates that bundle after commands.

App labels may contain only letters, digits, `_`, and `-`. App labels must be unique among app labels.

### Repeat macros

Repeat macros is used to avoid copy-pasting repeating patterns and improve json maintainability.

```json
{
  "flow": [
    {
      "label": "APP_A",
      "bundle": "com.example.app",
      "ability": "EntryAbility",
      "terminate": true,
      "commands": [
        {
          "macro": "repeat",
          "payload": {
            "iter_var": "i",
            "n_iter": 3,
            "commands": [
              { "action": "wait", "payload": 2 },
              { "action": "snapshot", "payload": "after_wait_{i}" }
            ]
          }
        }
      ]
    }
  ]
}
```

`iter_var` names the replacement variable and must match `^[A-Za-z_][A-Za-z0-9_]*$`. `n_iter` is the non-negative iteration count. During expansion, every `{<iter_var>}` occurrence inside string typed payloads is replaced with the iteration number in the range `[0, n_iter-1]`. The macro is replaced in place by the expanded commands in the same order.

Labels are validated only after expansion: `"after_tap_{i}"` expands to `"after_tap_0"`, `"after_tap_1"`, ... and each resulting label must satisfy the canonical label rules.

### Programmatic API (lib.py)

User can use `lib.py` to build flows and write Python helpers for repeated command sequences when Flow JSON file become large.

```python
import pathlib

import lib


def scroll_samples(prefix: str, count: int, wait_s: float) -> list[lib.Command]:
    commands: list[lib.Command] = []
    for i in range(1, count + 1):
        commands.extend([
            lib.wait(wait_s),
            lib.directional_fling("down", velocity=20000, step_length=10),
            lib.snapshot(f"{prefix}{i}"),
            lib.screenshot(f"{prefix}{i}"),
        ])
    return commands


scenario = lib.flow(
    [
        lib.app_flow(
            label="app_label",
            bundle="com.app.bundle",
            ability="Ability",
            terminate=True,
            commands=[
                lib.snapshot("on_startup"),
                *scroll_samples("after_fling", count=10, wait_s=5),
            ],
        )
    ],
    desc="Measures memory across repeated scroll samples.",
)

device = lib.get_device(lib.get_hdc(pathlib.Path("/path/to/hdc")))
out_dir = pathlib.Path("memmem-out-1234")

lib.configure_logger("info")
lib.run(scenario, device, out_dir=out_dir, reboot=False, hilog=True)
```

`lib.run()` raises on failure. Since `out_dir` is passed explicitly, user can inspect that directory after the run for results.

Python users can also author macros programmatically. `lib.unprocessed_flow`, `lib.unprocessed_app_flow`, `lib.unprocessed_snapshot`, `lib.unprocessed_screenshot`, and `lib.repeat(n_iter, iter_var, commands)` build an `UnprocessedFlow`, and `lib.preprocess_flow(flow)` expands it into a canonical `lib.Flow` that `lib.run()` executes. This is the same transformation `run.py` applies to macro-bearing flow JSON:

```python
scenario = lib.unprocessed_flow([
    lib.unprocessed_app_flow(
        label="app_label",
        bundle="com.app.bundle",
        ability="Ability",
        terminate=True,
        commands=[
            lib.unprocessed_snapshot("on_startup"),
            lib.repeat(
                n_iter=10,
                iter_var="i",
                commands=[
                    lib.wait(5),
                    lib.directional_fling("down", velocity=20000, step_length=10),
                    lib.unprocessed_snapshot("after_fling_{i}"),
                    lib.unprocessed_screenshot("after_fling_{i}"),
                ],
            ),
        ],
    )
], desc="Measures memory across repeated scroll samples.")

canonical = lib.preprocess_flow(scenario)
lib.run(canonical, device, out_dir=out_dir, reboot=False, hilog=True)
```

However, use of `lib.unprocessed_*` API is highly discouraged for programmatic users -- for better readability, prefer python statements to construct flows that do not need preprocessing.

Optional memmem application logging is configured separately via `lib.configure_logger(level, log_file=None)`, retrieved via `lib.get_logger()`, and reset via `lib.reset_logger()`. `level` is one of `"info"`, `"warn"`, or `"err"`. When `log_file` is `None` (default), info/warnings go to stdout and errors go to stderr; when set, all emitted memmem logs overwrite that file. An empty-string `log_file` is invalid: **breaking change** from previous behavior where `""` routed to stdout/stderr and `None` now does that instead.

## Commands

### Existing commands

```json
{ "action": "wait", "payload": 1.0 }
{ "action": "snapshot", "payload": "label" }
{ "action": "screenshot", "payload": "label" }
{ "action": "key", "payload": { "key": "Home" } }
```

`wait` blocks execution for the given amount of seconds.

`snapshot` captures smaps for all app labels launched so far whose stored PID is still alive. Snapshot labels may contain only letters, digits, `_`, and `-`. Snapshot labels must be unique among snapshot labels.

`screenshot` captures the current screen as PNG raw evidence. Screenshot labels may contain only letters, digits, `_`, and `-`. Screenshot labels must be unique among screenshot labels.

`key` accepts only `Home`, `Back`, or `Power` and sends corresponding input to the device.

### UI commands

Coordinates are integer percentages from `0` to `100`:

```json
{ "action": "tap", "payload": { "x_pct": 50, "y_pct": 50 } }
{ "action": "double_tap", "payload": { "x_pct": 50, "y_pct": 50 } }
{ "action": "long_tap", "payload": { "x_pct": 50, "y_pct": 50 } }
{ "action": "input_text", "payload": { "x_pct": 50, "y_pct": 40, "text": "hello" } }
{ "action": "text", "payload": "hello" }
```

Swipe-like commands require `velocity` in `200..40000`:

```json
{
  "action": "swipe",
  "payload": {
    "x1_pct": 50,
    "y1_pct": 80,
    "x2_pct": 50,
    "y2_pct": 20,
    "velocity": 800
  }
}
```

`drag` has the same payload as `swipe`.

`fling` also requires positive `step_length`:

```json
{
  "action": "fling",
  "payload": {
    "x1_pct": 50,
    "y1_pct": 80,
    "x2_pct": 50,
    "y2_pct": 20,
    "velocity": 1200,
    "step_length": 20
  }
}
```

Directional fling:

```json
{
  "action": "directional_fling",
  "payload": { "direction": "up", "velocity": 1200, "step_length": 20 }
}
```

`direction` is one of `left`, `right`, `up`, `down`.

## Output

### Single run (`--repeats 1`)

The output directory contains:

- `flow.json`: canonicalized validated flow JSON
- `app_metadata.json`: app launch metadata in flow order
- `snapshots/metadata.json`: snapshot labels and timestamps in command order
- `screenshots/metadata.json`: screenshot labels and timestamps in command order
- `snapshots/<snapshot_label>/<app_label>.smaps`: raw smaps evidence
- `screenshots/<screenshot_label>.png`: raw screenshot evidence
- `hilog/hilog.log`: run-wide hilog artifact when hilog collection is enabled
- `summary.csv`: app label, snapshot label, and total memory metrics per received snapshot
- `breakdowns/<snapshot_label>/<app_label>.csv`: per-tag metrics per snapshot
- `plots/<app_label>/<metric>.svg`: per-app linear memory trend plot for each metric

### Repeated runs (`--repeats N`)

With `N` is greater than `1`, the output directory instead holds one output directory per iteration. Names for these directories are `iteration_0` through `iteration_{N-1}`, each laid out exactly as described above. Additionally, output directory holds a `summary/` directory with two averaged reports, `summary.csv` and `summary_filtered.csv`, an `outliers.csv` file, averaged plots, and averaged breakdowns.

`summary.csv` is the aggregate of all usable iterations. `summary_filtered.csv` is the filtered aggregate, computed after discarding outlying iterations. Both files share the same columns: `app_label`, `snapshot_label`, `n_samples`, then `<metric>_<statistic>` for each memory metric and each of `mean`, `geomean`, `median`, `std`, `min`, `max`. `n_samples` counts the iterations that produced that row. `std` is the sample standard deviation and is `0.00` when `n_samples` is `1`. The geomean is reported as `0` when any contributing value is `0`.

The filtered report omits rows that appear only in discarded iterations. When no iteration is discarded, `summary_filtered.csv` matches `summary.csv`.

`outliers.csv` has a single column, `discarded_iteration`, listing one discarded iteration directory name per row.

`summary/plots/<app_label>/<metric>.svg` holds one averaged trend plot per app label and metric, plotting the geomean across snapshots with std error bars, built from the retained (filtered) iterations. `summary/breakdowns/<snapshot_label>/<app_label>.csv` holds one averaged breakdown CSV per pair present in any retained iteration, with the same metric column names as `summary.csv` and per-tag statistics matching the averaged summary. Averaged breakdowns are skipped for pairs that appear only in discarded iterations.

Averaging fails if no iteration produced a `summary.csv`.

Reboot, when enabled, applies before each iteration's flow execution.

## Testing

```bash
source .venv/bin/activate
make tests_full
```

Useful individual checks:

```bash
make test
make mypy
```

## Update requirements

```bash
pip freeze > requirements.txt
```
