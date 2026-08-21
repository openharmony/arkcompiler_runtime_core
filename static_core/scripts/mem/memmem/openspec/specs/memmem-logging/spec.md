# memmem-logging Specification

## Purpose
Defines memmem application logging levels, destinations, public logger behavior, and required benchmark logging events.

## Requirements

### Requirement: Memmem logging supports configured levels
The system SHALL support memmem application log levels `info`, `warn`, and `err` through one module-level configured logger. Log filtering SHALL emit `error` messages when configured level is `err`, emit `warning` and `error` messages when configured level is `warn`, and emit `info`, `warning`, and `error` messages when configured level is `info`. Emitted log lines SHALL use full-word prefixes `info:`, `warning:`, and `error:`.

#### Scenario: Default error-only logging
- **WHEN** benchmark execution uses the default memmem log level
- **THEN** info and warning messages are suppressed

#### Scenario: Warning logging includes errors
- **WHEN** benchmark execution uses memmem log level `warn`
- **THEN** warning and error messages are emitted and info messages are suppressed

#### Scenario: Info logging includes all levels
- **WHEN** benchmark execution uses memmem log level `info`
- **THEN** info, warning, and error messages are emitted

#### Scenario: Log prefixes use full words
- **WHEN** any memmem log message is emitted
- **THEN** the line begins with `info:`, `warning:`, or `error:` according to its severity

### Requirement: Memmem logging supports stdout stderr and file destinations
The system SHALL route memmem application logs according to the configured log file path. When the log file path is unset (`None`), `info` and `warning` messages SHALL be written to stdout and `error` messages SHALL be written to stderr. When the log file path is a non-empty path, all emitted memmem log messages SHALL be written only to that file. An empty-string log file path SHALL be rejected at configuration time. Log file writes SHALL overwrite an existing file and SHALL fail if the parent directory does not exist.

#### Scenario: Unset log file routes informational output to stdout
- **WHEN** the memmem log file path is unset (`None`) and an info or warning message passes level filtering
- **THEN** the message is written to stdout

#### Scenario: Unset log file routes errors to stderr
- **WHEN** the memmem log file path is unset (`None`) and an error message passes level filtering
- **THEN** the message is written to stderr

#### Scenario: Empty string log file is rejected
- **WHEN** the memmem log file path is set to an empty string
- **THEN** the system rejects logger configuration before any benchmark execution proceeds

#### Scenario: Non-empty log file captures all emitted logs
- **WHEN** the memmem log file path is a non-empty path and messages pass level filtering
- **THEN** info, warning, and error messages are written only to the configured file

#### Scenario: Existing log file is overwritten
- **WHEN** benchmark execution starts with a configured memmem log file path that already exists
- **THEN** the system overwrites the previous file contents

#### Scenario: Missing log file parent fails
- **WHEN** benchmark execution starts with a configured memmem log file path whose parent directory does not exist
- **THEN** the system fails before benchmark execution proceeds

### Requirement: Runner path emits info transitions
The system SHALL emit `info` memmem log messages for runner path phase transitions when the configured level allows info messages. Required transitions include start benchmark pre flow, start flow verification, start benchmark flow, start application pre flow, start application flow, start application post flow, start benchmark post flow, start child-process shutdown, start metadata writing, start artifact receiving, and start remote cleanup.

#### Scenario: Benchmark pre flow transition is logged
- **WHEN** benchmark preparation begins and memmem log level is `info`
- **THEN** the system emits an info log for start benchmark pre flow

#### Scenario: Benchmark flow transition is logged
- **WHEN** benchmark execution begins running AppFlows and memmem log level is `info`
- **THEN** the system emits an info log for start benchmark flow

### Requirement: Runner path warnings use memmem logging
The system SHALL emit recoverable runner-path warnings through memmem warning logs instead of direct stderr prints. This includes report averaging warnings for missing iteration summaries and plot generation warnings for missing snapshot metadata. Warning messages SHALL obey memmem log level filtering and destination routing. Warning message inputs SHALL NOT include the `warning:` prefix because the logger adds it.

#### Scenario: Missing iteration summary warning uses logger
- **WHEN** averaged report generation skips an iteration because its `summary.csv` is missing and memmem log level is `warn` or `info`
- **THEN** the system emits a memmem warning log for the skipped iteration

#### Scenario: Missing plot metadata warning uses logger
- **WHEN** plot generation skips plots because snapshot metadata is missing and memmem log level is `warn` or `info`
- **THEN** the system emits a memmem warning log for missing snapshot metadata

### Requirement: Command execution emits diagnostic info logs
The system SHALL emit an `info` memmem log message for every benchmark command execution when the configured level allows info messages. Command execution log messages SHALL identify the current AppFlow label, command action, and command payload. Flow content, including payload values from `text` and `input_text` commands, SHALL be treated as ordinary non-secret benchmark input and MAY be reproduced in diagnostic logs.

#### Scenario: Command execution is logged
- **WHEN** a benchmark command executes and memmem log level is `info`
- **THEN** the system emits an info log containing the AppFlow label and command action

#### Scenario: Text command payload is logged
- **WHEN** a `text` command executes with a text payload and memmem log level is `info`
- **THEN** the emitted command execution log contains the text payload value

#### Scenario: Input text command payload is logged
- **WHEN** an `input_text` command executes with a text payload and memmem log level is `info`
- **THEN** the emitted command execution log contains the structured payload, including its text value

### Requirement: HDC execution emits complete diagnostic command logs
The system SHALL emit an `info` memmem log message containing the complete host-side HDC command argv before invoking HDC when the configured level allows info messages. This includes serialized remote-shell arguments originating from flow fields, because flow content is non-secret benchmark input. Logging SHALL NOT change the argv passed to the HDC subprocess.

#### Scenario: HDC text input command is logged
- **WHEN** a `text` or `input_text` flow command reaches HDC and memmem log level is `info`
- **THEN** the emitted HDC execution log contains the serialized text payload and the HDC subprocess receives the same payload

### Requirement: Fatal CLI failures use error output
The system SHALL report fatal benchmark CLI failures with an `error:`-prefixed line at the `run.py` top-level exception boundary. Library execution through `lib.run()` SHALL continue to report failures by raising exceptions and SHALL NOT convert failures to logged return statuses.

#### Scenario: CLI fatal error is logged
- **WHEN** `run.py` catches a fatal benchmark error
- **THEN** the system emits an `error:`-prefixed line containing `memmem error:` and exits with status `1`

#### Scenario: Library fatal error is raised
- **WHEN** `lib.run()` encounters a fatal benchmark or report generation error
- **THEN** the system raises the exception to the caller
