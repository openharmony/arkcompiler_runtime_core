## Context

Benchmark preparation currently reboots by default, waits for boot completion, wakes the device, disables screen timeout, and configures hilog before AppFlows. It does not check whether the device is in a comparable physical state. Real-device exploration showed stable health sources under sysfs: battery capacity at `/sys/class/power_supply/Battery/capacity` and thermal zone names/temperatures under `/sys/class/thermal/thermal_zone*/type` and `temp`.

## Goals / Non-Goals

**Goals:**
- Expose a device-level `device_health()` observation API for battery capacity and thermal zones.
- Keep benchmark acceptance policy in the runner, not the device layer.
- Reject benchmark startup when battery capacity is below 30%.
- Reject benchmark startup when any readable nonzero thermal zone exceeds 60°C.
- Run the health gate immediately after device preparation and before remote setup, screen-bounds reading, and AppFlows.
- Produce clear rejection errors identifying the failing signal and threshold.

**Non-Goals:**
- No CLI flags or user-configurable thresholds.
- No automatic waiting for cooling or charging.
- No reboot retry loop for unhealthy devices.
- No health metadata output in this change.

## Decisions

### Device API returns observations, runner owns policy

Add a `Device.device_health()` API that returns normalized observations such as `DeviceHealth` and `ThermalZoneHealth`. The device layer reads and parses sysfs but does not decide whether values are acceptable. The runner adds a `_verify_device_ok()` helper, called immediately after `_prepare_device()`, with hard threshold constants.

Alternative considered: `Device.verify_device_ok()`. Rejected because acceptable battery and temperature thresholds are benchmark policy, not device transport behavior.

### Use sysfs as the primary source

Use `/sys/class/power_supply/Battery/capacity` for battery capacity and `/sys/class/thermal/thermal_zone*/type` plus `temp` for thermal zones. This avoids service-specific formatting, and worked on the connected OpenHarmony device.


### Normalize thermal temperatures in millidegrees Celsius

Thermal zone `temp` values are represented as millidegrees Celsius on the tested device, so `60000` represents 60°C. Battery sysfs `temp` may use a different scale, but thermal verification uses thermal zone data rather than battery power-supply temperature.

### Ignore inactive zero thermal readings

Many thermal zones on the tested device report `0`, representing inactive or unavailable sensors rather than a real 0°C device temperature. The runner health policy ignores zero-valued thermal zones and rejects only readable nonzero zones above 60000.

### Fail fast instead of recovery

Health verification rejects startup immediately. This keeps benchmark behavior predictable and avoids long, surprising waits or repeated reboots.

## Risks / Trade-offs

- Sysfs paths may differ across devices. → Fail with a clear health-read error when required battery or thermal data cannot be read.
- Some thermal zones may report values in an unexpected scale. → Use millidegrees Celsius because the connected target reports values like `30000` for 30°C; keep the representation explicit in names and errors.
- Ignoring zero thermal zones may hide a broken sensor. → This avoids false failures from inactive zones observed on the target; nonzero readable zones still gate startup.
- Hard thresholds may be too strict or too loose for some devices. → Start with agreed defaults and keep them runner constants for future adjustment.
