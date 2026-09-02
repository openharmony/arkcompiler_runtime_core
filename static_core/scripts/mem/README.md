# About

Memory management related scripts and fremeworks.

## memmem

The framework launches configured apps, executes flow commands, captures `/proc/<pid>/smaps` and screenshots, receives raw evidence, and writes CSV reports and SVG memory trend plots.  
See `memmem/README.md`.

## Scripts

- `gc_pause_stats.py` parses one or more GC log files, calculates pause-time statistics by GC type and cause, and writes the results as a Markdown table report.
- `gc_phase_time_stats.py` collects thread-time and CPU-time statistics for selected GC phases from a detailed GC log and writes them as a Markdown table.
- `memdump.py` analyzes a binary `memdump.bin` allocation dump, reporting total, peak, and live allocations with optional space and stack-trace filtering.
- `memusage.py` reads `/proc/<pid>/smaps` and reports Size, RSS, and PSS grouped by Ark memory region and file type, optionally averaging samples until the process exits.
