## 1. Schema tests

- [x] 1.1 Add test asserting a `wait` command with non-negative float payload (e.g. `2.5`) loads successfully
- [x] 1.2 Add test asserting a `wait` command with negative float payload fails validation
- [x] 1.3 Add test asserting a `text` command with legacy struct payload `{"text": "..."}` fails validation

## 2. Verification

- [x] 2.1 Run full test suite and confirm all tests pass
- [x] 2.2 Run mypy and confirm no new issues
- [x] 2.3 Run `openspec validate` and confirm the change validates