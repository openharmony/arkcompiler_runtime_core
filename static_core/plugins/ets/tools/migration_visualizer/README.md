# ArkTS Migration Visualizer

`migration_visualizer` combines **Homecheck** dependency graphs and **Hapray** performance data into a single static visualization dataset, then renders HAR-level and file-level dependencies, hotspots, and navigation in a pure frontend page.

## Features

- **Two-level views**: top-level HAR graph and per-HAR file graph
- **Timing heat mapping**: node size and color are driven by `timing_cycles`
- **Non-zero filtering**: when enabled, zero-timing nodes are hidden and paths are reconnected across hidden nodes
- **Cross-HAR counters**: File -> external HAR call counts are shown on edges
- **Ranking and click-to-focus**:
  - HAR ranking and global file ranking on the top-level page
  - Per-HAR file ranking and global Top 30 in file view
  - Clicking a ranking item jumps to the node and highlights it
- **Directed edges**: all edges are rendered with arrowheads
- **Hover highlighting with depth control**: use the slider to highlight downstream nodes within N levels
- **Layout tuning**: file graphs are intentionally more spread out, and disconnected components are distributed more naturally

## Repository Layout

```text
migration_visualizer/
├─ configs/
│  ├─ config.example.json
│  └─ dependency_manifest.json
├─ pyproject.toml
├─ src/
│  ├─ arkts_migration_visualizer/
│  │  ├─ __main__.py
│  │  ├─ build/
│  │  │  └─ integrate_dep.py
│  │  ├─ cli.py
│  │  ├─ collect/
│  │  │  ├─ export_hapray_timeline_trace.py
│  │  │  └─ perf_runner.py
│  │  └─ deps/
│  │     └─ bootstrap_impl.py
│  ├─ run_all.py
│  ├─ scripts/
│  │  ├─ bootstrap.sh
│  │  └─ bootstrap.cmd
│  ├─ sample_data/
│  │  └─ hierarchical_integrated_data.json
│  └─ web/
│     └─ index.html
└─ tests/
   ├─ support.py
   ├─ test_bootstrap_impl.py
   ├─ test_export_hapray_timeline_trace.py
   ├─ test_integrate_dep.py
   ├─ test_perf_runner.py
   └─ test_run_all.py
```

## Requirements

- Python 3.10 to 3.12
- Node.js / npm
  - Node 20+ is recommended because some upstream Hapray npm dependencies declare `engine` requirements at or above Node 20
- Git
- A usable OpenHarmony / DevEco SDK
- Network access to dependency repositories and package mirrors

Notes:

- `migration_visualizer` only needs Hapray's `perf` workflow. The upstream `gui-agent` dependency (`Open-AutoGLM`, model API, etc.) is not required by default
- `bootstrap.sh` / `bootstrap.cmd` install the optional `gui-agent` dependency only when `--with-gui-agent` is passed explicitly
- When `uv` is available, bootstrap will use it for local/offline Python package installs and fall back to `pip` otherwise

## Configuration

Copy the template first:

Duplicate `configs/config.example.json` as `configs/config.json`, then edit it in place.

`configs/` stays at the repository root even though executable sources live under `src/`. This keeps local machine-specific configuration separate from tracked source files.

The most important fields in `configs/config.json` are:

- `project_path`: the OpenHarmony project to analyze
- `sdk_path`: DevEco SDK root directory (the parent directory that contains both `hms/` and `openharmony/`)
- `hdc_path`: explicit path to the `hdc` executable (recommended on Windows; use this to avoid SDK auto-discovery)
- `deps_root`: dependency root, default is `.deps`
- `hapray_python`: Python interpreter used for Hapray
- `npm_registry` / `pip_index_url`: optional mirror settings
- `windows_short_alias_root`: optional root directory for Hapray short-path aliases on Windows
- `hapray_testcases` / `hapray_round` / `hapray_devices`: Hapray runtime parameters
- `node_max_old_space_size`: Node heap size used by Homecheck

Notes:

- Native Windows paths can be written directly as `C:\\...`
- `deps_root` is important on Windows when the repository path is long. In that case, use a shorter absolute path such as `C:\\test\\.amv-deps\\migration_visualizer`
- When `windows_short_alias_root` is empty, the script will derive the Hapray short-path alias directory from `deps_root` first. In most cases, setting a short `deps_root` is enough

For manual-listening mode, make sure:

- `project_path` points to the actual application project you want to operate
- the `bundleName` in `AppScope/app.json5` matches the `--manual-package` argument

## Prepare Dependencies

Use the platform-specific dependency entrypoint:

Windows:

```bat
.\src\scripts\bootstrap.cmd --tool all --verify
```

macOS / Linux / WSL:

```bash
./src/scripts/bootstrap.sh --tool all --verify
```

If you also need Hapray's optional `gui-agent` workflow:

```bash
./src/scripts/bootstrap.sh --tool hapray --with-gui-agent
```

If the default mirrors are unstable, specify them explicitly:

```bat
.\src\scripts\bootstrap.cmd --tool all --verify ^
  --npm-registry https://<your-npm-mirror>/ ^
  --pip-index-url https://<your-pypi-mirror>/simple/ ^
  --deps-root C:\test\.amv-deps\migration_visualizer
```

Replace `<your-npm-mirror>` and `<your-pypi-mirror>` with the npm registry base URL and PyPI simple index host approved for your environment.

Notes:

- `bootstrap.sh` and `bootstrap.cmd` both read `deps_root` from `configs/config.json`
- You can also override it temporarily from the command line with `--deps-root <path>`
- `bootstrap.cmd` and `arkts-migration-visualizer` both support `--deps-root <path>`
- The rule is: **command-line arguments take precedence over `configs/config.json`**
- If you want the entire workflow to use the same temporary dependency directory, pass the same `--deps-root` value to bootstrap and to `arkts-migration-visualizer`
- Dependencies are stored under `deps_root`, including:
  - `sources/homecheck`
  - `sources/ArkAnalyzer-HapRay`
  - `runtime/homecheck`
  - `dependency-lock.json`

## Quick Start

Install the tool in editable mode first:

```bash
python3 -m pip install -e .
```

The packaged entrypoint is `arkts-migration-visualizer`. If your shell does not resolve
that command yet, use the module form instead:

- macOS / Linux / WSL:
  ```bash
  python3 -m arkts_migration_visualizer --help
  ```
- Windows:
  ```bat
  python -m arkts_migration_visualizer --help
  ```

### One-shot Run

```bash
arkts-migration-visualizer -t ".*testcase"
```

Equivalent fallback on macOS / Linux / WSL:

```bash
python3 -m arkts_migration_visualizer -t ".*testcase"
```

If you want to override the dependency directory temporarily:

```bash
arkts-migration-visualizer -t ".*testcase" --deps-root C:\test\.amv-deps\migration_visualizer
```

This command will:

1. collect Homecheck + Hapray data
2. pick the latest `artifacts/test_run_*`
3. generate `src/web/hierarchical_integrated_data.json`
4. start `http://localhost:8000/` by default

Common options:

- `--test/-t`: Hapray testcase regex
- `--skip-collect`: skip collection and only build/serve
- `--run-dir`: use an existing `artifacts/test_run_*`
- `--no-serve`: do not start the local HTTP server
- `--port`: specify the local port
- `--no-open`: do not open the browser automatically

Examples:

```bash
arkts-migration-visualizer --skip-collect
arkts-migration-visualizer --skip-collect --run-dir artifacts/test_run_YYYYMMDD_HHMMSS
arkts-migration-visualizer -t ".*TestCase_0010"
```

### Manual Performance Listening

If you do not want to run a fixed automated testcase and instead want to operate the app manually on the device:

```bash
arkts-migration-visualizer --manual-package com.example.xxx --manual-duration 60
```

Equivalent fallback on macOS / Linux / WSL:

```bash
python3 -m arkts_migration_visualizer --manual-package com.example.xxx --manual-duration 60
```

Optionally specify the launch ability explicitly:

```bash
arkts-migration-visualizer --manual-package com.example.xxx --manual-ability EntryAbility --manual-duration 60
```

Characteristics of manual-listening mode:

- It still uses the same `Homecheck + Hapray` data pipeline
- Once the console prints the "start manual load collection" message, you operate the target app manually on the device
- The same four input files are still generated after collection

### Timeline Trace Export

To export a Hapray `perf.db` as a timeline trace JSON file through the packaged entrypoint:

```bash
arkts-migration-visualizer export-timeline perf.db
```

Equivalent fallback on macOS / Linux / WSL:

```bash
python3 -m arkts_migration_visualizer export-timeline perf.db
```

Common options:

- `--step`: choose a specific `stepN` directory when the input directory contains multiple steps
- `--event-name`: export a specific perf event instead of the default preferred one
- `--thread-id`: repeat to limit export to one or more thread ids
- `--no-merge-samples`: keep adjacent samples separate instead of merging intervals

## Internal Stages

`src/arkts_migration_visualizer/collect/perf_runner.py`, `src/arkts_migration_visualizer/build/integrate_dep.py`, and `src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py` are internal pipeline modules used by the packaged tool entrypoint.

They are no longer supported as standalone user-facing entrypoints. The supported workflow entry is:

```bash
arkts-migration-visualizer
```

## Testing

The project ships package-manager-backed development targets through `pyproject.toml`.

Install development tools:

```bash
python3 -m pip install -e .[dev]
```

Run the built-in targets from the repository root:

```bash
hatch run test
hatch run lint
```

The test suite focuses on:

- `src/arkts_migration_visualizer/deps/bootstrap_impl.py` Hapray bootstrap layout and depatch behavior
- `src/arkts_migration_visualizer/build/integrate_dep.py` core data integration logic
- `src/arkts_migration_visualizer/collect/perf_runner.py` helper logic and artifact extraction
- `src/arkts_migration_visualizer/cli.py` orchestration behavior

### Start a Local Server

```bash
cd src/web
python3 -m http.server 8000
```

Open:

```text
http://localhost:8000/
```

## Page Behavior

The page is fully static and does not depend on a backend service.

Current interactions:

- The first layer shows HAR relationships
- Clicking a HAR enters file view
- Clicking blank space or pressing `Esc` clears the current pinned detail
- The detail panel shows the node name, full path, and timing
- "Non-zero only" hides zero-timing nodes and reconnects paths across hidden nodes
- The ranking panel supports direct jump-to-node actions

Current data semantics:

- HAR node timing mainly comes from HAR-level statistics in `component_timings.json` / `hapray_report.json`
- File node timing comes from `fileEvents` in `hapray_report.json`, then gets mapped back to source paths
- Artifact nodes such as `.preview`, `build cache`, and `generated`, together with the pseudo-dependencies they introduce, are filtered during integration

## Sample Data

If you only want to preview the page without real collection:

```bash
cp src/sample_data/hierarchical_integrated_data.json src/web/
cd src/web
python3 -m http.server 8000
```
