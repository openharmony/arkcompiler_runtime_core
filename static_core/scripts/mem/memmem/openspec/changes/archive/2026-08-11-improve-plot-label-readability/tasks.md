## 1. Plot Label Layout

- [x] 1.1 Replace point-local snapshot annotations with wrapped x-axis tick labels.
- [x] 1.2 Add density-aware rotation, font size, bottom margin, and figure width selection.
- [x] 1.3 Keep dotted lines, markers, relative x-axis seconds, linear plots, and log-transformed plots unchanged.
- [x] 1.4 Make policy 2 no-wrap labels the default plot layout.
- [x] 1.5 Add x-axis and y-axis grid lines.
- [x] 1.6 Increase y-axis tick density to target at least 10 tick positions.

## 2. Samples

- [x] 2.1 Generate sample plots from `memmem-out-20260807_183946_292597` with rotate 30-45 degrees and soft wrapping.
- [x] 2.2 Generate sample plots from `memmem-out-20260807_183946_292597` with rotate 60-90 degrees and wrapping.
- [x] 2.3 Generate sample plots from `memmem-out-20260807_183946_292597` with rotate 90 degrees, wider figure, and smaller font.
- [x] 2.4 Generate sample plots from `memmem-out-20260807_183946_292597` with rotate 75 degrees and no wrapping.
- [x] 2.5 Regenerate selected default-style samples after adding grid lines and denser y ticks.

## 3. Verification

- [x] 3.1 Add or update tests for x-axis tick snapshot labels, no wrapping, grid lines, and denser y-axis ticks.
- [x] 3.2 Run focused plot/report tests.
- [x] 3.3 Run type checking and full tests.
- [x] 3.4 Validate the OpenSpec change.
