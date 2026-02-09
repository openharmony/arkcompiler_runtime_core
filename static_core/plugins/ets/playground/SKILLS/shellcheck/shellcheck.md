---
name: shellcheck
description: Verify all bash scripts using ShellCheck static analysis
---

Runs ShellCheck to analyze bash scripts for common errors, security issues, and portability problems in the ArkTS Playground project.

## Usage

```bash
/shellcheck [path] [options]
```

## Examples

### Check All Shell Scripts
```bash
/shellcheck
```

### Check Specific Directory
```bash
/shellcheck backend/docker/
/shellcheck scripts/
```

### Check Specific File
```bash
/shellcheck backend/docker/entrypoint.sh
/shellcheck scripts/build.sh
```

### Check with Specific Shellcheck Options
```bash
/shellcheck frontend/ --severity=warning
/shellcheck backend/ --shell=bash
```

## What It Does

ShellCheck performs static analysis on shell scripts to detect:
- **Syntax errors** before runtime
- **Security vulnerabilities** (unquoted variables, command injection risks)
- **Portability issues** (bashisms in POSIX scripts)
- **Code smells** (unused variables, deprecated features)
- **Best practices violations** (missing error handling, quoting)

### ShellCheck Severity Levels

| Level | Description | Example |
|-------|-------------|---------|
| **error** | Critical issues that break scripts | Unmatched `if`, syntax errors |
| **warning** | Serious problems that may cause issues | Unquoted variables, missing quotes |
| **info** | Suggestions for improvement | Unused variables, missing shebang |
| **style** | Stylistic improvements | Quoting conventions, readability |

### ShellCheck Exit Codes

- **0** - All scripts passed (no issues found)
- **1** - All scripts passed but some have issues
- **>1** - Command failed or script errors detected

## How It Works

1. **Discovery**: Finds all `.sh` files in the specified path (or project root if none specified)
2. **Analysis**: Runs ShellCheck on each file with appropriate options
3. **Aggregation**: Collects all findings and organizes by severity
4. **Reporting**: Displays summary with file-by-file breakdown
5. **Suggestions**: Provides specific recommendations for each issue

## Shell Files in This Project

Based on the repository structure, shell scripts may be found in:

```
backend/docker/
└── entrypoint.sh             # Backend container entry point (if exists)

backend/scripts/              # Build/utility scripts (if exists)
└── *.sh

frontend/
└── scripts/                  # Frontend build scripts (if exists)
    └── *.sh

scripts/                      # Project-wide utility scripts (if exists)
└── *.sh
```

## Common Workflows

### Pre-Commit Verification
Check all scripts before committing:
```bash
/shellcheck
```

### CI/CD Integration
Add to CI pipeline to ensure code quality:
```bash
/shellcheck --severity=error
```

### New Script Validation
Validate newly created scripts:
```bash
/shellcheck path/to/new-script.sh
```

### Security Audit
Focus on security-related issues:
```bash
/shellcheck --severity=warning --exclude=SC2034
```

## Best Practices

- **Fix All Errors**: Never commit scripts with ShellCheck errors
- **Address Warnings**: Review and fix or justify warnings
- **Consider Style**: Style suggestions improve maintainability
- **Use Function Keyword**: Always define functions with `function name()` syntax for better readability and POSIX compatibility
- **Be Specific**: Use paths when you know which scripts changed
- **Continuous Integration**: Run in CI/CD pipelines for automated checks

## ShellCheck Rules Reference

### Common SC Codes

| Code | Severity | Issue | Fix |
|------|----------|-------|-----|
| SC2002 | warning | Useless cat | Use `$(< file)` instead of `$(cat file)` |
| SC2086 | warning | Unquoted variable | Quote variables: `"$VAR"` |
| SC2034 | info | Unused variable | Remove or use variable |
| SC2164 | warning | Use `cd ... || exit` | Handle cd failure |
| SC2207 | warning | Don't use `ls` output | Use globs instead |
| SC1001 | error | Usage error | Fix syntax |

### Security-Related Checks

- **SC2086**: Double-quote to prevent globbing and word splitting
- **SC2046**: Quote to prevent word splitting on `$()`
- **SC2091**: Remove `$` from `$(cmd)`
- **SC2154**: Variable is referenced but not assigned

### POSIX Compliance

- **SC2039**: Use POSIX-compatible constructs
- **SC3000**: Remove bashisms for POSIX scripts
- **SC3010**: In POSIX sh, `[[ ]]` is undefined

## Tips

- **Install ShellCheck**: Ensure `shellcheck` is in your PATH
  ```bash
  # Ubuntu/Debian
  sudo apt-get install shellcheck

  # macOS
  brew install shellcheck

  # Verify installation
  shellcheck --version
  ```

- **Shebang Matters**: ShellCheck analyzes based on the shebang (`#!/usr/bin/env bash` vs `#!/usr/bin/env sh`)
- Use portable shebangs: `#!/usr/bin/env bash` instead of `#!/bin/bash` for better cross-platform compatibility

- **Function Keyword**: Always use the `function` keyword for better readability and POSIX compatibility:
  ```bash
  # Good
  function cleanup() {
      echo "Cleaning up..."
  }

  # Avoid
  cleanup() {
      echo "Cleaning up..."
  }
  ```

- **Suppress Cautiously**: Only suppress specific rules with justification:
  ```bash
  # shellcheck disable=SC2086
  ```

- **Online ShellCheck**: Can use https://www.shellcheck.net for quick checks

- **Integration**: Works with editors (VSCode, Vim, Emacs) via extensions

- **Version**: Use latest ShellCheck for most accurate results

## Troubleshooting

**ShellCheck not found:**
- Install ShellCheck via package manager
- Ensure it's in your PATH
- Check with `shellcheck --version`

**Too many warnings:**
- Start with `--severity=error` to focus on critical issues
- Fix incrementally: errors first, then warnings, then style
- Use `--exclude` to suppress specific rules if justified

**False positives:**
- Add inline suppressions: `# shellcheck disable=SC CODE`
- Review the rule documentation to understand the warning
- Consider if the warning reveals a real issue

**POSIX vs Bash:**
- Ensure shebang matches intent: `#!/usr/bin/env sh` vs `#!/usr/bin/env bash`
- Use portable shebangs with `/usr/bin/env` for cross-platform compatibility
- Use `--shell=bash` if shebang is missing
- POSIX scripts have more restrictions

**CI Failures:**
- Run locally first: `/shellcheck --severity=error`
- Check for new issues introduced by changes
- Review and fix or suppress with justification

## ShellCheck Options Reference

```bash
# Format options
-s bash          # Specify dialect (bash, sh, zsh)
-f json          # Output format (json, gcc, tty)

# Filter options
-S error         # Minimum severity (error, warning, info, style)
-i CODE          # Include specific rule
-e CODE          # Exclude specific rule

# Output options
-a               # Output all checks
-C               # Colorize output (default for TTY)
-V               # Verbose mode

# Example combinations
shellcheck -s bash -f json script.sh
shellcheck -S warning -e SC2034 script.sh
```

## Integration Examples

### Makefile Target
```makefile
.PHONY: shellcheck
shellcheck:
	@echo "Running ShellCheck..."
	@find . -name "*.sh" -exec shellcheck {} +
```

### GitHub Actions
```yaml
- name: Run ShellCheck
  run: |
    find . -name "*.sh" -exec shellcheck -x {} +
```

### Pre-commit Hook
```bash
#!/usr/bin/env bash
# .git/hooks/pre-commit
git diff --cached --name-only | grep '\.sh$' | xargs shellcheck
```

## Related Skills

- `/docker` - Docker shell scripts may need validation
- `/playground-test` - Test scripts should pass ShellCheck

## Additional Resources

- [ShellCheck Wiki](https://www.shellcheck.net/wiki/) - Full rule documentation
- [SC Source Code](https://github.com/koalaman/shellcheck) - GitHub repository
- [ShellCheck Online](https://www.shellcheck.net/) - Web-based checker

## Output Example

```
Running ShellCheck on ArkTS Playground scripts...

Checking 2 shell scripts...

✓ scripts/build.sh
  - No issues found

✗ backend/docker/entrypoint.sh (2 warnings)
  line 15: SC2086: Double quote to prevent globbing and word splitting
  line 23: SC2164: Use 'cd ... || exit' in case cd fails

Summary:
  Total files: 2
  Passed: 1
  Issues: 2 warnings

Recommendations:
  - Quote variables on line 15
  - Add error handling for cd on line 23
```

Regular ShellCheck use ensures reliable, portable, and secure shell scripts throughout the ArkTS Playground project.
