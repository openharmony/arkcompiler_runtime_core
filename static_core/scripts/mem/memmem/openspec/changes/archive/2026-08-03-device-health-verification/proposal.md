## Why

Benchmark runs should start from comparable device conditions. A low battery or overheated device can skew memory and performance behavior, so the runner should reject benchmark startup when device health is outside hard default bounds.

## What Changes

- Add a device health observation API that reads battery capacity and thermal zone temperatures from device sysfs.
- Add a runner-owned startup health verification step with hard defaults: battery capacity must be at least 30%, and every readable nonzero thermal zone must be at or below 60°C.
- Run health verification immediately after device preparation and before remote setup, screen-bounds reading, and AppFlows.
- Reject benchmark startup with clear errors when health signals are unreadable or outside bounds.
- Extend fake-device test support with configurable health state.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: device preparation includes a unified startup device-health verification step before AppFlows.
- `device-hdc`: device layer exposes normalized battery and thermal health observations.
- `testing-support`: fake device models health readings for runner tests.

## Impact

- Affected code: `src/device.py`, `src/runner.py`, `test/mock/device.py`, device and runner tests.
- Affected behavior: benchmark startup may fail before AppFlows when battery capacity is below 30%, any readable nonzero thermal zone exceeds 60°C, or required health data cannot be read.
- No CLI options are added; thresholds are hard defaults owned by the runner.
