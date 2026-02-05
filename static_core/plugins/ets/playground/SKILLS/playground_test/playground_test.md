---
name: playground-test
description: Run frontend (Jest) or backend (pytest) tests and parse results
---

Uses the Bash agent to run test suites and provide parsed, actionable results.

## Usage

```bash
/playground-test [frontend|backend] [optional: specific test file]
```

## Examples

### Run All Frontend Tests
```bash
/playground-test frontend
```

### Run All Backend Tests
```bash
/playground-test backend
```

### Run Specific Frontend Test
```bash
/playground-test frontend src/components/controlPanel/__tests__/controlPanel.test.tsx
```

### Run Specific Backend Test
```bash
/playground-test backend tests/test_api.py
```

### Run Tests with Coverage
```bash
/playground-test backend --cov
```

## What It Does

### Frontend Tests
- Runs Jest test suite from `frontend/`
- 31 test files covering:
  - React components (React Testing Library)
  - Redux slices and selectors
  - API services (with mocked axios)
  - Utility functions
- Returns parsed test results with failures highlighted

### Backend Tests
- Runs pytest suite from `backend/`
- Tests include:
  - API endpoint integration tests
  - Pydantic model validation
  - Compiler/disasm runner wrapper
  - Configuration management
- Returns pytest results with failure details

## Test Coverage

**Frontend:**
- Component testing: `src/components/**/__tests__/`
- Store testing: `src/store/**/__tests__/`
- Service testing: `src/services/__tests__/`

**Backend:**
- API tests: `tests/test_api.py`
- Runner tests: `tests/test_runners.py`
- Config tests: `tests/test_config.py`

## Best Practices

- Run frontend tests first (they're faster)
- Run tests before committing changes
- Fix linting errors before running tests
- Use watch mode for active development (`npm run test:watch`)
- Use `--cov` for coverage reports on backend

## Tips

- Frontend tests use mocked Monaco Editor in `__mocks__`
- Backend tests require ArkTS binaries to be built
- Specific test files can be run for faster feedback
- Check both success and failure paths
- Use pytest fixtures for common test setup

## Related Skills

- `/add-endpoint` - Run tests after adding new endpoints
- `/plan-feature` - Include tests in feature planning
- `/shellcheck` - Validate shell scripts in test suites
