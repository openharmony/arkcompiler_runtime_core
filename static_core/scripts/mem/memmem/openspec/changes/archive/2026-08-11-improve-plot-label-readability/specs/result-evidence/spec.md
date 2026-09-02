## MODIFIED Requirements

### Requirement: Memory trend plots are generated
The system SHALL generate per-app SVG memory trend plots for Size, Rss, Pss, Referenced, Shared, Private, Swap, and Anonymous metrics using the same in-memory report rows used to write `summary.csv`.

#### Scenario: Linear metric plot is generated
- **WHEN** report generation has rows for an app
- **THEN** the system writes one dotted-line SVG plot per metric under `plots/<app_label>/<metric>.svg`

#### Scenario: Log-transformed metric plot is generated
- **WHEN** report generation has rows for an app
- **THEN** the system writes one log-transformed SVG plot per metric under `plots/<app_label>/<metric>_log.svg`

#### Scenario: Zero appears in log-transformed plot
- **WHEN** a metric value is zero
- **THEN** the log-transformed plot shows that point at y = -1

#### Scenario: Negative metric value is encountered
- **WHEN** a metric value is negative
- **THEN** report generation fails because memory metrics must be non-negative

#### Scenario: Plot x-axis uses relative time
- **WHEN** plotting metric rows for an app
- **THEN** the x-axis uses seconds elapsed since that app's first plotted snapshot timestamp

#### Scenario: Plot points identify snapshots
- **WHEN** plotting metric rows
- **THEN** each point is associated with its snapshot label using unwrapped rotated x-axis tick labels and connected with a dotted line

#### Scenario: Dense snapshot labels avoid plotted data
- **WHEN** plotting metric rows for a flow with many snapshot labels
- **THEN** snapshot labels are rendered outside the plotted data region using rotation and layout sizing so labels do not cover the trend line or markers

#### Scenario: Plot grid lines are shown
- **WHEN** plotting metric rows
- **THEN** the system renders both x-axis and y-axis grid lines behind the plotted data

#### Scenario: Y-axis has denser tick marks
- **WHEN** plotting metric rows
- **THEN** the y-axis uses at least 10 target tick positions when possible

#### Scenario: Plotting runs without a display server
- **WHEN** reports are generated in a headless environment
- **THEN** plotting writes SVG files without requiring a GUI display

#### Scenario: No aggregate plots are generated
- **WHEN** report generation writes plot artifacts
- **THEN** it writes per-app per-metric plots only and does not write cross-app aggregate plot files
