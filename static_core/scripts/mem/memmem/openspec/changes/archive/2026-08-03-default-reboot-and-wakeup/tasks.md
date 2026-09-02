## 1. CLI and Documentation

- [x] 1.1 Change CLI reboot default to enabled while keeping `--no-reboot` as the opt-out.
- [x] 1.2 Update CLI tests so omitted reboot controls produce `reboot=True` and explicit `--no-reboot` produces `False`.
- [x] 1.3 Update README run-option documentation to state reboot defaults to enabled.

## 2. Device Wakeup Operation

- [x] 2.1 Add `Device.wakeup()` using `power-shell wakeup` through HDC shell.
- [x] 2.2 Add device-layer tests for successful wakeup command translation and wakeup failure reporting.

## 3. Reboot Preparation Flow

- [x] 3.1 Extend reboot-enabled environment preparation to call wakeup after boot completion.
- [x] 3.2 Extend reboot-enabled environment preparation to perform `directional_fling("up", 20000, 20)` after wakeup.
- [x] 3.3 Extend reboot-enabled environment preparation to send `Back` after the fling and before disabling screen timeout.
- [x] 3.4 Ensure the post-reboot wake routine is skipped when reboot is disabled.

## 4. Test Fakes and Runner Tests

- [x] 4.1 Add fake-device wakeup state and failure knob.
- [x] 4.2 Update runner tests to assert reboot preparation includes reboot, availability wait, boot-complete wait, wakeup, upward fling, Back, and screen-timeout disabling.
- [x] 4.3 Update runner tests to assert no-reboot runs skip wakeup and wake gestures.

## 5. Spec and Verification

- [x] 5.1 Update main or delta specs as needed during implementation if behavior changes from this plan.
- [x] 5.2 Run `source ".venv/bin/activate" && make test`.
- [x] 5.3 Run `source ".venv/bin/activate" && make tests_full`.
- [x] 5.4 Run `openspec validate "default-reboot-and-wakeup"`.
