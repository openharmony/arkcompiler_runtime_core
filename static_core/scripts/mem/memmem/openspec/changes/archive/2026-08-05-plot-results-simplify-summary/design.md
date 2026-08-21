## Context

The benchmark currently generates raw evidence, metadata, `summary.csv`, and per-snapshot breakdown CSVs from received smaps files. `summary.csv` includes `pid` and `timestamp`, but those values are already represented in `app_metadata.json` and `snapshot_metadata.json`. The report path already builds in-memory `SummaryRow` objects containing all values needed for both CSV output and plotting.

Memory trend visualization needs a new plotting dependency and a headless-safe rendering approach because reports may be generated in CLI, CI, or server environments without a display server.

## Goals / Non-Goals

**Goals:**
- Simplify `summary.csv` by removing duplicated `pid` and `timestamp` columns.
- Preserve `app_label` and `snapshot_label` as join keys into metadata files.
- Generate per-app SVG trend plots for all summary memory metrics.
- Generate both linear and log-transformed companion plots.
- Keep plotting mechanics isolated in `src/plot.py`.
- Use the same in-memory rows as summary CSV generation rather than reparsing `summary.csv`.
- Keep plotting headless-safe.

**Non-Goals:**
- No aggregate cross-app plots.
- No interactive plotting.
- No CLI option for output formats in this change.
- No removal of `app_label` or `snapshot_label` from `summary.csv`.
- No changes to raw smaps, screenshot, log, app metadata, or snapshot metadata artifact formats.

## Decisions

### Use matplotlib and SVG output

Use matplotlib for static plot generation because it is the standard Python plotting library and supports SVG output through `savefig()`. SVG is the default output format because memory trend plots are line/marker graphics that benefit from vector scaling and remain easy to inspect or embed.

The implementation should centralize the extension or output format selection so later PNG/PDF support can be added without rewriting plotting logic.

### Use a headless-safe backend

Plotting should not depend on an interactive GUI backend. `src/plot.py` should select a non-interactive backend before importing pyplot so report generation works in CI, containers, and terminal-only environments.

### Keep report rows as the shared data source

`generate_reports()` should continue parsing metadata and raw smaps once, build in-memory `SummaryRow` objects, and use those rows for summary CSV and plot generation. `summary.csv` should not become an internal input to plotting, especially because it intentionally omits `pid` and `timestamp` after this change.

### Call plotting from report generation

`generate_reports()` should call `generate_plots()` after collecting rows and writing derived CSV outputs. Plots are derived reports, so this keeps report orchestration in one place while the plotting details live in `src/plot.py`.

### Use per-app relative time

For each app's plots, the x-axis should be seconds since that app's first plotted snapshot timestamp. This avoids unreadable nanosecond timestamps and makes each app's trend easy to read independently.

### Use dotted lines and point markers

Points should be connected with dotted lines and markers. The dotted line communicates trend without implying continuous exact measurement between snapshots. Snapshot labels should be associated with plotted points, for example through text annotations.

### Use transformed values for log companion plots

The companion `_log.svg` plots should plot transformed values, not a true logarithmic axis. For each metric value:

- if value is negative, report generation fails
- if value is zero, plotted y-value is `-1`
- otherwise, plotted y-value is `log10(value)`

The y-axis should make this explicit, e.g. `log10(KB), 0 KB shown as -1`.

### Keep path generation in ResultStore

`ResultStore` should provide plot directory/path helpers so output layout remains centralized with existing snapshot, screenshot, log, and breakdown path helpers.

## Risks / Trade-offs

- `matplotlib` increases dependency footprint. → Accept because static visualization is a core deliverable for this change.
- Dense snapshot labels may overlap. → Start with simple annotations; improve layout later if real flows show readability issues.
- Relative app-local x-axes make cross-app time alignment less obvious. → Aggregate/cross-app plots are out of scope, and per-app readability is the priority.
- `summary.csv` schema changes are breaking. → Downstream compatibility is not a concern; metadata files remain the source for PID and timestamps.
- SVG viewing support varies by environment. → SVG is widely supported by browsers and editors; future format expansion remains straightforward.
