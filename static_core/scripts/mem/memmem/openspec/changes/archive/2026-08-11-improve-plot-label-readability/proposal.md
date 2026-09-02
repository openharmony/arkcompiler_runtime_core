## Why

Dense benchmark flows can generate many snapshot points, and point-local text annotations currently cover the plotted memory trend or overlap each other. Moving snapshot labels onto the x-axis improves readability while preserving the association between points and snapshot labels.

## What Changes

- Replace in-plot snapshot text annotations with x-axis tick labels for memory trend plots.
- Render snapshot labels as unwrapped rotated x-axis tick labels, using the selected policy-2 sample as the default.
- Adjust figure layout so dense flows have enough horizontal and bottom margin for unwrapped labels.
- Add x-axis and y-axis grid lines to make dense plots easier to read.
- Use denser y-axis ticks so plots show at least 10 y-axis tick positions when possible.
- Generate experimental sample outputs from an existing result directory for visual comparison before choosing the final policy.

## Capabilities

### New Capabilities

### Modified Capabilities
- `result-evidence`: memory trend plots identify snapshot points using readable x-axis labels instead of labels that can cover plotted data.

## Impact

- Affected code: `src/plot.py`, plot/report tests, and possibly README output examples if the final behavior needs documentation.
- No public API changes.
- No new dependency is expected for the initial implementation.
