## Why

The UI recorder implementation has accumulated extra lifecycle wrappers and conversion branching that make the script harder to read and maintain. The recorder output filename also now follows the same timestamp convention as `run.py`, so the specs and tests should align with that behavior.

## What Changes

- Simplify `record.py` control flow by removing redundant recorder startup/process dataclasses and consolidating startup readiness handling into one focused function.
- Keep named dataclasses where structured return values remain useful, rather than replacing them with nameless tuples.
- Refactor UI event conversion to use separate pointer and key dispatch paths, with pointer handlers selected from an `OP_TYPE` handler map.
- Print recorder conversion warnings immediately instead of accumulating warnings for later emission.
- Treat malformed supported records as fatal conversion errors; unsupported/skipped records emit warnings and produce no commands.
- Remove the low-value filename-format unit test while keeping behavior covered by implementation/spec alignment.
- Update recorder filename requirements to `flow-YYYYMMDD_HHMMSS_microseconds.json` for consistency with `run.py` timestamp formatting.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-input-recording`: Update generated recorder flow filename format and clarify warning/error behavior for skipped versus malformed records.
- `testing-support`: Update recorder test expectations to focus on generated command structure rather than warning text, and remove the filename-format unit test requirement.

## Impact

- Affects `record.py` recorder lifecycle, conversion helpers, and output path helper.
- Affects `test/record_test.py` recorder tests.
- Affects README and OpenSpec specs that describe recorder output filenames.
- No changes to flow schema, command schema, device execution behavior, or dependencies.
