## Context

Memory trend plotting currently draws labels as annotations at each data point with a fixed offset. That keeps the label close to the point, but with many snapshots the text overlaps other labels and can cover the line, markers, and trend shape.

The plot output is SVG and generated headlessly with matplotlib. That makes wider dynamic figures practical because users can zoom or open the result in a browser/editor.

## Goals / Non-Goals

**Goals:**
- Keep every snapshot point associated with its label.
- Move labels away from the plotted data region when flows have many snapshots.
- Keep the solution deterministic and dependency-free for the first iteration.
- Generate comparison samples from a real output directory before settling the final default policy.

**Non-Goals:**
- Interactive labels, tooltips, or browser-side plotting.
- Cross-app aggregate visualization.
- Adding automatic collision solvers such as `adjustText` in this iteration.

## Decisions

### Use x-axis tick labels instead of point annotations

Snapshot labels identify x positions more than y values, so they fit naturally as tick labels. This keeps the plotted memory line and markers unobstructed.

Alternative considered: continue using annotations with alternating offsets. This reduces collisions but still allows text to cover data when snapshots are dense.

### Use unwrapped rotated x-axis labels by default

The visual samples showed that policy 2 with `rotation=75` and no wrapping is the most readable. Labels should remain intact rather than splitting across lines, with extra horizontal width and bottom margin providing space.

Alternative considered: wrapped x-axis labels. Wrapping avoids long horizontal extents but makes repeated-prefix labels harder to scan and visually busier.

### Keep a density-aware fallback for figure size

The selected default should use a steep rotation and no wrapping while sizing the SVG wide enough for dense flows. Font size and bottom margin can remain tuned for readability without using point annotations.

Alternative considered: one small fixed figure. This preserves current dimensions but recreates crowding for dense flows.

### Add grid lines and denser y ticks

Both x-axis and y-axis grid lines should be enabled so points can be traced to time and memory values. The y-axis should target at least 10 ticks, making changes easier to estimate than the current sparse four-tick layout.

Alternative considered: keep matplotlib defaults. The defaults are clean but too coarse for memory inspection in dense plots.

### Generate samples as explicit experiment artifacts

Produce sample SVGs for candidate layouts from `/home/kostr2010/memmem_clean/memmem-out-20260807_183946_292597` so the policy can be chosen visually. After selection, generate a no-wrap policy-2 sample for confirmation.

## Risks / Trade-offs

- Unwrapped labels can require more horizontal space. → Use wider SVG figures and keep output zoomable.
- Grid lines can make plots visually heavier. → Use light dashed grid styling behind the plotted data.
- Denser y ticks can crowd small plots. → Use a locator target rather than manual labels, allowing matplotlib to choose readable values.
- Dynamic figure sizing changes plot dimensions from current output. → Keep per-app/per-metric paths and SVG format unchanged; only layout changes.
