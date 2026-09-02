## ADDED Requirements

### Requirement: Device layer reads screen bounds from UITest layout
The system SHALL provide a device operation that reads root screen bounds using `uitest dumpLayout` and exposes only parsed bounds to callers.

#### Scenario: Layout dump contains root bounds
- **WHEN** the runner asks the device for screen bounds
- **THEN** the device layer dumps UITest layout, parses the root `attributes.bounds` value, and returns `left`, `top`, `right`, and `bottom` bounds

#### Scenario: Layout dump command fails
- **WHEN** the underlying layout dump command exits with failure
- **THEN** the device layer reports screen bounds discovery failure to the caller

#### Scenario: Root bounds are missing or malformed
- **WHEN** the dumped layout does not contain parseable root bounds
- **THEN** the device layer reports screen bounds discovery failure to the caller

### Requirement: Device layer supports UITest coordinate operations
The system SHALL provide device operations for tap, double tap, long tap, swipe, drag, fling, directional fling, coordinate-based text input, and focused text input using `hdc shell uitest uiInput` commands.

#### Scenario: Tap is requested
- **WHEN** the command layer delegates a tap with a pixel point
- **THEN** the device layer executes `uitest uiInput click <x> <y>`

#### Scenario: Double tap is requested
- **WHEN** the command layer delegates a double tap with a pixel point
- **THEN** the device layer executes `uitest uiInput doubleClick <x> <y>`

#### Scenario: Long tap is requested
- **WHEN** the command layer delegates a long tap with a pixel point
- **THEN** the device layer executes `uitest uiInput longClick <x> <y>`

#### Scenario: Swipe is requested
- **WHEN** the command layer delegates a swipe with start point, end point, and velocity
- **THEN** the device layer executes `uitest uiInput swipe <x1> <y1> <x2> <y2> <velocity>`

#### Scenario: Drag is requested
- **WHEN** the command layer delegates a drag with start point, end point, and velocity
- **THEN** the device layer executes `uitest uiInput drag <x1> <y1> <x2> <y2> <velocity>`

#### Scenario: Fling is requested
- **WHEN** the command layer delegates a fling with start point, end point, velocity, and step length
- **THEN** the device layer executes `uitest uiInput fling <x1> <y1> <x2> <y2> <velocity> <step_length>`

#### Scenario: Directional fling is requested
- **WHEN** the command layer delegates a directional fling with direction, velocity, and step length
- **THEN** the device layer maps the direction to UITest direction ID and executes `uitest uiInput dircFling <direction_id> <velocity> <step_length>`

#### Scenario: Coordinate text input is requested
- **WHEN** the command layer delegates text input with a pixel point and text
- **THEN** the device layer executes `uitest uiInput inputText <x> <y> <text>`

#### Scenario: Focused text input is requested
- **WHEN** the command layer delegates focused text input with text
- **THEN** the device layer executes `uitest uiInput text <text>`

## MODIFIED Requirements

### Requirement: Device layer supports key events
The system SHALL provide a named key event operation that accepts `Home`, `Back`, or `Power` and sends the corresponding UITest key event through HDC.

#### Scenario: Key command delegates to device
- **WHEN** a valid `key` command is executed
- **THEN** the device layer sends `uitest uiInput keyEvent <key>` to the device

#### Scenario: Key command fails on device
- **WHEN** the UITest key event command exits with failure
- **THEN** the device layer reports key event failure to the caller
