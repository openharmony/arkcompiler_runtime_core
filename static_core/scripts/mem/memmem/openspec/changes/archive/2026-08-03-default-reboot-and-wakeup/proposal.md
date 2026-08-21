## Why

Users expect stable benchmark startup without manually remembering to reboot first, and recent device wakeup fixes need to be captured as explicit behavior. Reboot should become the default preparation path, and the post-reboot wake routine should be specified, documented, and covered by tests.

## What Changes

- Change CLI reboot default from disabled to enabled; `--no-reboot` remains the opt-out.
- Update benchmark options/tests so omitted reboot controls produce `reboot=True`.
- Add a device wakeup operation using `power-shell wakeup`.
- Extend reboot preparation to wake the device after boot completion, perform an upward unlock-style fling, and send Back before disabling screen timeout.
- Update fakes and tests to cover wakeup state/failures and the post-reboot wake routine.
- Update README and OpenSpec specs to describe reboot-on-by-default and post-reboot wake behavior.

## Capabilities

### New Capabilities

### Modified Capabilities
- `benchmark-flow`: CLI reboot default and reboot preparation sequence change.
- `device-hdc`: device wakeup operation is added to benchmark environment controls.
- `testing-support`: fake device support for wakeup state and failure simulation.

## Impact

- Affected code: `run.py`, `src/device.py`, `src/runner.py`, `test/mock/device.py`, `test/device_test.py`, `test/run_cli_test.py`, `test/runner_test.py`, and README.
- Behavior change: benchmark runs reboot by default unless users pass `--no-reboot`.
- No new external dependencies.
