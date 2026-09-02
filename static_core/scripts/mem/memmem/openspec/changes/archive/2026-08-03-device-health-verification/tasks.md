## 1. Device Health Model and Reading

- [x] 1.1 Add device health data models for battery capacity percentage and thermal zone observations.
- [x] 1.2 Implement `Device.device_health()` using `/sys/class/power_supply/Battery/capacity` and `/sys/class/thermal/thermal_zone*/type` plus `temp`.
- [x] 1.3 Ensure health read failures or parse failures raise clear runtime errors.
- [x] 1.4 Add device-layer tests for successful battery and thermal parsing.
- [x] 1.5 Add device-layer tests for missing or malformed health data failures.

## 2. Runner Health Policy

- [x] 2.1 Add runner constants for minimum battery capacity `30` and maximum thermal temperature `60000` millidegrees Celsius.
- [x] 2.2 Add runner `_verify_device_ok()` logic that rejects battery capacity below 30%.
- [x] 2.3 Add runner `_verify_device_ok()` logic that rejects any nonzero thermal zone above 60000 millidegrees Celsius.
- [x] 2.4 Ensure zero-valued thermal zones are ignored by runner policy.
- [x] 2.5 Call device health verification immediately after device preparation and before remote setup, screen-bounds reading, or AppFlow launch.

## 3. Fake Device and Runner Tests

- [x] 3.1 Extend `test/mock/device.py` with configurable healthy default device health.
- [x] 3.2 Extend fake device support for low-battery, hot-zone, zero-zone, and health-read failure cases.
- [x] 3.3 Add runner tests showing healthy startup proceeds.
- [x] 3.4 Add runner tests showing low battery rejects startup before AppFlows.
- [x] 3.5 Add runner tests showing hot thermal zones reject startup before AppFlows.
- [x] 3.6 Add runner tests showing zero-valued thermal zones do not reject startup.
- [x] 3.7 Add runner tests asserting health verification occurs after reboot wake routine and before screen-timeout/log setup.

## 4. Documentation and Specs

- [x] 4.1 Update README run documentation to mention hard startup health checks.
- [x] 4.2 Update delta specs if implementation behavior changes from this plan.

## 5. Verification

- [x] 5.1 Run `source ".venv/bin/activate" && make test`.
- [x] 5.2 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 5.3 Run `openspec validate "device-health-verification"`.
