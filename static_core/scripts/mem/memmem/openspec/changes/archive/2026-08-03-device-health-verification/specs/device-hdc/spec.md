## ADDED Requirements

### Requirement: Device layer reports device health observations
The system SHALL provide a device health operation that reads battery capacity and thermal zone observations from the device and returns normalized values without applying benchmark acceptance policy.

#### Scenario: Battery capacity is read
- **WHEN** the runner requests device health
- **THEN** the device layer reads `/sys/class/power_supply/Battery/capacity` through HDC shell and returns the parsed integer percentage as battery capacity

#### Scenario: Thermal zones are read
- **WHEN** the runner requests device health
- **THEN** the device layer reads thermal zone names and temperatures from `/sys/class/thermal/thermal_zone*/type` and `/sys/class/thermal/thermal_zone*/temp` through HDC shell and returns thermal temperatures in millidegrees Celsius

#### Scenario: Device health read fails
- **WHEN** required battery or thermal health data cannot be read or parsed
- **THEN** the device layer reports device health discovery failure to the caller

#### Scenario: Device health contains inactive thermal zones
- **WHEN** a thermal zone reports temperature `0`
- **THEN** the device layer preserves that zero-valued thermal observation for the runner to interpret
