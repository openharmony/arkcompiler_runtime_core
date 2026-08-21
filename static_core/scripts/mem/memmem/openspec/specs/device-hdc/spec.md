# device-hdc Specification

## Purpose
Defines HDC wrapper behavior and device operations used by benchmark execution.

## Requirements

### Requirement: HDC wrapper executes generic commands
The system SHALL provide an HDC wrapper that executes `<HDC_PATH> <args...>` and returns return code, stdout, and stderr without imposing success policy. The wrapper SHALL accept a command timeout where `timeout=-1` means no timeout. If command execution times out, the wrapper SHALL return `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`.

#### Scenario: Caller runs generic HDC command
- **WHEN** a caller invokes `Hdc.run('list', 'targets')`
- **THEN** the wrapper executes `<HDC_PATH> list targets` and returns an `HdcResult`

#### Scenario: HDC command exits non-zero
- **WHEN** an HDC command returns a non-zero exit code
- **THEN** the wrapper returns the non-zero return code, stdout, and stderr to the caller

#### Scenario: HDC command times out
- **WHEN** an HDC command exceeds the non-negative timeout provided by the caller
- **THEN** the wrapper returns `HdcResult(127, "", f"memmem: TIMEOUT {cmd}")`

#### Scenario: HDC timeout is disabled
- **WHEN** a caller invokes `Hdc.run` with `timeout=-1`
- **THEN** the wrapper executes the command without a subprocess timeout

### Requirement: HDC wrapper executes shell commands
The system SHALL provide `Hdc.shell` for executing argv-style remote commands and returning return code, stdout, and stderr. For non-empty argv, the shell wrapper SHALL serialize the arguments as one POSIX-quoted command and execute `<HDC_PATH> shell <serialized_command>` so that every caller argument retains its original remote argument boundary. Calling `Hdc.shell` without arguments SHALL preserve the interactive `<HDC_PATH> shell` invocation. The system SHALL provide `Hdc.shell_raw` for commands that deliberately require remote shell syntax such as redirection, conditionals, or loops. Both shell wrappers SHALL accept the same timeout semantics as `Hdc.run`.

#### Scenario: Caller runs shell command
- **WHEN** a caller invokes `Hdc.shell('pidof', 'com.example.app')`
- **THEN** the wrapper executes `<HDC_PATH> shell <serialized_command>` where the serialized command parses to `['pidof', 'com.example.app']` and returns an `HdcResult`

#### Scenario: Shell argument contains metacharacters
- **WHEN** an argv-style shell argument contains spaces, quotes, `;`, `&&`, or `$()`
- **THEN** the serialized remote command preserves the complete value as one argument and does not interpret its contents as additional shell syntax

#### Scenario: Caller runs a deliberate raw shell command
- **WHEN** a caller invokes `Hdc.shell_raw` with a command containing redirection, a conditional, or a loop
- **THEN** the wrapper passes that pre-serialized command unchanged as the single command after `<HDC_PATH> shell`

#### Scenario: Caller starts an interactive shell
- **WHEN** a caller invokes `Hdc.shell()` without command arguments
- **THEN** the wrapper executes `<HDC_PATH> shell` without adding an empty command argument

#### Scenario: Caller runs shell command with timeout
- **WHEN** a caller invokes `Hdc.shell` with a non-negative timeout
- **THEN** the wrapper applies that timeout to the underlying HDC shell command

### Requirement: Device layer supports app lifecycle, PID checks, timestamps, and smaps capture
The system SHALL provide device operations for launching an app, resolving its current PID, checking PID validity, creating remote directories, obtaining device timestamps, capturing smaps to device-local files, capturing screenshots to device-local files, terminating an app bundle, and removing remote directories.

#### Scenario: App launch is requested
- **WHEN** the runner starts an `AppFlow`
- **THEN** the device layer launches the requested bundle and ability using HDC shell commands based on the legacy memmem implementation

#### Scenario: PID resolution returns one PID
- **WHEN** the runner needs the launched app PID and `pidof <bundle>` returns exactly one parseable PID
- **THEN** the device layer returns that PID even when it was already tracked for another app label

#### Scenario: PID resolution returns no PID
- **WHEN** the runner needs the launched app PID and `pidof <bundle>` returns no parseable PID
- **THEN** the device layer reports PID resolution failure to the caller

#### Scenario: PID resolution returns multiple PIDs
- **WHEN** the runner needs the launched app PID and `pidof <bundle>` returns more than one parseable PID
- **THEN** the device layer reports PID resolution failure to the caller without choosing a PID

#### Scenario: PID validity is checked
- **WHEN** the snapshot command checks a stored PID
- **THEN** the device layer reports whether `/proc/<pid>` exists on the device

#### Scenario: Device timestamp is requested
- **WHEN** a snapshot or screenshot command needs a timestamp for metadata
- **THEN** the device layer returns a filename-safe digit-only timestamp produced on the device

#### Scenario: Remote run directory is created
- **WHEN** the runner starts benchmark execution
- **THEN** the device layer creates the requested remote run directory under `/data/local/tmp`

#### Scenario: Smaps capture is requested
- **WHEN** the snapshot command captures smaps for a valid PID
- **THEN** the device layer executes `cat /proc/<pid>/smaps > <remote_path>` on the device and reports success or failure to the caller

#### Scenario: Screenshot capture is requested
- **WHEN** the screenshot command captures the current screen
- **THEN** the device layer executes `uitest screenCap -p <remote_path>` on the device and reports success or failure to the caller

#### Scenario: App termination is requested
- **WHEN** the runner requests termination for a bundle
- **THEN** the device layer executes `aa force-stop <bundle>` through HDC shell and reports success only when command output contains `force stop process successfully`

#### Scenario: App termination command reports an error in stdout
- **WHEN** `aa force-stop <bundle>` returns output containing an error message or no success marker
- **THEN** the device layer reports app termination failure to the caller even when the HDC return code is zero

#### Scenario: Remote run directory is cleaned up
- **WHEN** pending artifacts have been received successfully
- **THEN** the device layer removes the remote run directory and reports cleanup failure to the caller

### Requirement: Device layer supports reboot and boot readiness
The system SHALL provide device operations for rebooting the device, waiting for HDC availability, and waiting for OpenHarmony boot completion.

#### Scenario: Device reboot is requested
- **WHEN** the runner requests device reboot
- **THEN** the device layer executes `hdc target boot` and reports failure to the caller when the command fails

#### Scenario: Device availability wait is requested
- **WHEN** the runner waits for device availability
- **THEN** the device layer executes `hdc wait` with the provided timeout and reports failure to the caller when the command fails or times out

#### Scenario: Boot completion wait succeeds
- **WHEN** the runner waits for boot completion and `param get bootevent.boot.completed` returns `true` before timeout
- **THEN** the device layer reports boot completion success

#### Scenario: Boot completion wait times out
- **WHEN** `param get bootevent.boot.completed` does not return `true` before timeout
- **THEN** the device layer reports boot completion failure to the caller

### Requirement: Device layer supports benchmark environment controls
The system SHALL provide device operations for waking the device, disabling screen timeout, configuring hilog collection, and starting run-wide hilog streaming to a device-local file through a generic child process handle. The child process handle MUST allow the caller to stop the host-side HDC stream process using Python built-in cross-platform process APIs.

#### Scenario: Device is woken
- **WHEN** the runner requests device wakeup
- **THEN** the device layer executes `power-shell wakeup` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Screen timeout is disabled
- **WHEN** the runner requests screen timeout disabling
- **THEN** the device layer executes `power-shell timeout -o 60000000` through HDC shell and reports failure to the caller when the command fails

#### Scenario: Hilog is configured
- **WHEN** the runner configures hilog collection
- **THEN** the device layer executes `hilog -Q pidoff` and `hilog -p off` through HDC shell and reports failure when either command fails

#### Scenario: Hilog stream is started
- **WHEN** the runner requests hilog streaming to a remote path
- **THEN** the device layer starts command `hilog > <remote_path>` through HDC shell and returns a generic child process handle without streaming hilog into host memory

#### Scenario: Hilog stream is stopped
- **WHEN** the caller stops a generic hilog child process handle
- **THEN** the system terminates the direct host-side HDC stream process, waits for bounded shutdown, and kills the process if it remains alive using Python built-in cross-platform process APIs

#### Scenario: Hilog stream path is required
- **WHEN** the runner requests hilog streaming
- **THEN** the caller MUST provide a device-local remote path and the device layer does not stream hilog into host memory

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

### Requirement: Device layer supports key events
The system SHALL provide a named key event operation that accepts `Home`, `Back`, or `Power` and sends the corresponding UITest key event through HDC.

#### Scenario: Key command delegates to device
- **WHEN** a valid `key` command is executed
- **THEN** the device layer sends `uitest uiInput keyEvent <key>` to the device

#### Scenario: Key command fails on device
- **WHEN** the UITest key event command exits with failure
- **THEN** the device layer reports key event failure to the caller

### Requirement: Device layer supports file transfer utilities
The system SHALL provide file send and receive operations using HDC file transfer commands for device-side snapshot artifacts and future device-side workflows.

#### Scenario: File send is requested
- **WHEN** a caller invokes `send_file(local_path, remote_path)`
- **THEN** the device layer executes `hdc file send <local_path> <remote_path>`

#### Scenario: File receive is requested
- **WHEN** a caller invokes `recv_file(remote_path, local_path)`
- **THEN** the device layer creates the local parent directory and executes `hdc file recv <remote_path> <local_path>`
