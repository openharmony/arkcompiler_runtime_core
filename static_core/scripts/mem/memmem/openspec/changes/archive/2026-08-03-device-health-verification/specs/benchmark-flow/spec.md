## MODIFIED Requirements

### Requirement: Benchmark execution prepares device environment before AppFlows
The benchmark runner SHALL prepare the device environment before launching any `AppFlow`. When reboot is enabled, it SHALL reboot the device, wait for HDC availability, wait for boot completion, wake the device, perform an upward directional fling, and send Back. Regardless of reboot setting, it SHALL disable screen timeout before AppFlows. When logs are enabled, it SHALL configure hilog before AppFlows. After device preparation completes, it SHALL verify that device health is acceptable before remote setup, screen-bounds reading, or AppFlow launch.

#### Scenario: Reboot is enabled
- **WHEN** benchmark options have `reboot` equal to `True`
- **THEN** the runner reboots the device, waits for device availability, waits for boot completion, performs the post-reboot wake routine, completes device preparation, verifies device health, and then reads screen bounds before launching AppFlows

#### Scenario: Reboot is disabled
- **WHEN** benchmark options have `reboot` equal to `False`
- **THEN** the runner skips reboot, device availability wait, boot-complete wait, and post-reboot wake routine while still verifying device health before launching AppFlows

#### Scenario: Post-reboot wake routine runs
- **WHEN** reboot preparation reaches boot completion
- **THEN** the runner wakes the device, performs an upward directional fling, and sends Back before disabling screen timeout

#### Scenario: Device health is acceptable
- **WHEN** benchmark execution prepares the device environment and device health reports battery capacity at least 30% and every readable nonzero thermal zone at or below 60000 millidegrees Celsius
- **THEN** the runner continues preparation and may launch AppFlows

#### Scenario: Battery capacity is too low
- **WHEN** benchmark execution prepares the device environment and device health reports battery capacity below 30%
- **THEN** the runner rejects benchmark startup before remote setup, reading screen bounds, or launching AppFlows

#### Scenario: Thermal zone is too hot
- **WHEN** benchmark execution prepares the device environment and any readable nonzero thermal zone reports a temperature above 60000 millidegrees Celsius
- **THEN** the runner rejects benchmark startup before remote setup, reading screen bounds, or launching AppFlows

#### Scenario: Screen timeout is disabled
- **WHEN** benchmark execution prepares the device environment
- **THEN** the runner disables screen timeout before device health verification and before launching AppFlows independent of reboot and log options

#### Scenario: Logs are enabled during preparation
- **WHEN** benchmark options have `logs` equal to `True`
- **THEN** the runner configures hilog before device health verification and before launching AppFlows
