---
name: run-linter
description: Running linter.sh for code quality checking (pylint, mypy, ruff, radon, yamllint)
---

# Skill: Run Linter for Code Quality Checking

## Overview

This skill covers how to run the linter suite for URunner-2.
`linter.sh` runs all static analysis tools in sequence and must exit with code `0`
before any commit. Run it after every code change.

---

## Step 1 — Run the Full Linter Suite

**Option A — via `linter.sh` (recommended, activates the virtual environment automatically):**

```bash
./linter.sh
```

**Option B — via `tox` (when the runner is installed as a package):**

```bash
source ~/.venv-panda/bin/activate
tox run -e linters
deactivate
```

The script activates the virtual environment automatically and runs the following checks in order:

| Check | Tool | Scope |
|---|---|---|
| Pylint (main code) | `pylint --rcfile .pylintrc` | `runner/`, `main.py` (tests excluded) |
| Pylint (tests) | `pylint --rcfile .pylintrc` | `runner/test/` (relaxed rules) |
| MyPy (main entry) | `mypy` | `main.py` |
| MyPy (package) | `mypy -p runner` | entire `runner/` package |
| Ruff | `ruff check .` | entire project |
| Cyclomatic complexity | `flake8 --radon-max-cc 15 --select=R701` | entire project |
| YAML lint | `check_yaml_templates.py` | `cfg/` directory |

Exit code `0` means all checks passed. Any non-zero exit code means at least one check failed;
the script prints `Linter checks failed (EXIT_CODE = N)` with the count of failed checks.

---

## Step 2 — Fix Common Issues

### Pylint errors
- **Missing type annotation** — add return type and parameter type annotations to all public functions/methods.
- **Unused import** — remove the import or use it.
- **Line too long** — break the line to stay within the PEP8 limit (configured in `.pylintrc`).
- **`Any` usage** — add a comment explaining why `Any` cannot be avoided in this case.

### MyPy errors
- **Missing return type** — annotate the function with its return type.
- **Incompatible types** — fix the type mismatch or add a cast with a comment.
- **`Any` in signature** — replace with a concrete type or document the reason.

### Ruff errors
- Ruff enforces PEP8 style and import ordering. Follow the suggested fixes in the output.

### Cyclomatic complexity (radon)
- If `flake8` reports `R701` for a function, refactor it to reduce branching (extract helper methods).
- The maximum allowed complexity is **15**.

### YAML lint errors
- Fix the reported YAML syntax or structure issues in `cfg/` files.

---

## Step 3 — Run Individual Checks

If you want to run a single tool without the full script, activate the virtual environment first:

```bash
source ~/.venv-panda/bin/activate

# Pylint on main code only
pylint --rcfile .pylintrc runner main.py --ignore=test

# Pylint on tests only (relaxed rules)
pylint --rcfile .pylintrc runner/test/ --disable=protected-access --disable=duplicate-code --disable=too-many-public-methods

# MyPy on main.py
mypy main.py

# MyPy on the runner package
mypy -p runner

# Ruff
ruff check .

# Cyclomatic complexity
flake8 --radon-max-cc 15 --select=R701 .

deactivate
```

---

## Step 4 — Key Rules Enforced

- All public functions and methods **must** have type annotations.
- `Any` usage **must** be documented with a comment explaining why it cannot be avoided.
- No unused imports.
- Only top level imports.
- PEP8 line length and formatting.
- Cyclomatic complexity per function must not exceed **15**.
- YAML files in `cfg/` must be valid and follow the project's template conventions.
