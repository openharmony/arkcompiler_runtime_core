## ADDED Requirements

### Requirement: Programmatic API exports public type names
The system SHALL expose through `lib.py` the type names needed to write mypy-checked programmatic user scripts without importing documented APIs from `src.*`. Public type exports SHALL include flow model types, command model types used by public builders, and device/HDC handle types used by public helper and run signatures. Payload structs and internal runner orchestration types SHALL NOT be public exports.

#### Scenario: Typed user script annotates flow helpers
- **WHEN** a user script annotates helper functions or variables with `lib.Flow`, `lib.AppFlow`, or `lib.Command`
- **THEN** mypy can resolve those names from `lib.py` without `src.*` imports

#### Scenario: Typed user script annotates device handles
- **WHEN** a user script annotates HDC or device variables with `lib.Hdc` or `lib.Device`
- **THEN** mypy can resolve those names from `lib.py` without `src.*` imports

#### Scenario: Precise command types are available
- **WHEN** a user script needs precise builder return annotations such as `lib.TapCommand` or `lib.WaitCommand`
- **THEN** those command model type names are available from the public `lib.py` facade

#### Scenario: Payload structs remain private
- **WHEN** a user imports documented public helpers from `lib.py`
- **THEN** payload struct names such as `TapPayload` are not part of the public facade

#### Scenario: Internal runner options remain private
- **WHEN** a user imports documented public helpers from `lib.py`
- **THEN** internal orchestration types such as `BenchmarkOptions` are not part of the public facade
