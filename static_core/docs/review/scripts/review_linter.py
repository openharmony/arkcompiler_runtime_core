#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Deterministic linter for arkcompiler review rules.

Checks a subset of REVIEW_RULES that can be verified via regex/grep
without semantic analysis. Supports two modes:
  --mode=diff   Analyze only changed files/lines from git diff (default)
  --mode=full   Scan all files under a given directory
"""

import argparse
import dataclasses
import fnmatch
import os
import re
import subprocess
import sys
from typing import Callable, Dict, List, Optional, Tuple

# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Violation:
    rule_id: str
    severity: str
    file: str
    line: int
    message: str

    def __str__(self) -> str:
        return f"{self.rule_id}:{self.severity}:{self.file}:{self.line}: {self.message}"


CheckerFn = Callable[[str, str], List[Tuple[int, str]]]


@dataclasses.dataclass
class Rule:
    rule_id: str
    severity: str
    file_globs: List[str]
    message: str
    pattern: Optional["re.Pattern[str]"] = None
    checker: Optional[CheckerFn] = None
    diff_only: bool = False
    diff_removed_only: bool = False
    diff_added_only: bool = False
    path_filter: Optional[Callable[[str], bool]] = None


# ---------------------------------------------------------------------------
# Diff parser
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class DiffFile:
    file_path: str
    added_lines: Dict[int, str]
    removed_lines: Dict[int, str]


def parse_unified_diff(diff_text: str) -> List[DiffFile]:
    """Parse unified diff output into structured per-file data."""
    files: List[DiffFile] = []
    current: Optional[DiffFile] = None
    old_line_no = 0
    new_line_no = 0

    for line in diff_text.splitlines():
        if line.startswith("diff --git"):
            current = None
            continue
        if line.startswith("+++ b/"):
            path = line[6:]
            current = DiffFile(file_path=path, added_lines={}, removed_lines={})
            files.append(current)
            continue
        if line.startswith("--- "):
            continue
        if line.startswith("@@ "):
            m = re.search(r"-(\d+)", line)
            old_line_no = int(m.group(1)) if m else 1
            m = re.search(r"\+(\d+)", line)
            new_line_no = int(m.group(1)) if m else 1
            continue
        if current is None:
            continue
        if line.startswith("+"):
            current.added_lines[new_line_no] = line[1:]
            new_line_no += 1
        elif line.startswith("-"):
            current.removed_lines[old_line_no] = line[1:]
            old_line_no += 1
        else:
            old_line_no += 1
            new_line_no += 1

    return files


# ---------------------------------------------------------------------------
# Naming convention helpers
# ---------------------------------------------------------------------------

def is_camel_case(name: str) -> bool:
    return bool(re.match(r"^_?[a-z][a-zA-Z0-9]*$", name))


def is_pascal_case(name: str) -> bool:
    return bool(re.match(r"^[A-Z][a-zA-Z0-9]*$", name))


def is_screaming_snake(name: str) -> bool:
    return bool(re.match(r"^[A-Z][A-Z0-9]*(_[A-Z0-9]+)*$", name))


# ---------------------------------------------------------------------------
# Path helpers
# ---------------------------------------------------------------------------

SKIP_DIRS = frozenset({
    ".git", "out", "build", "third_party", "node_modules",
    "__pycache__", ".cache", "REVIEW_RULES",
})

SKIP_PATHS = frozenset({
    "review/scripts",
})


def is_test_path(filepath: str) -> bool:
    parts = filepath.replace("\\", "/").split("/")
    return "test" in parts or "tests" in parts


def matches_glob(filepath: str, globs: List[str]) -> bool:
    basename = os.path.basename(filepath)
    return any(fnmatch.fnmatch(basename, g) for g in globs)


def read_file_safe(filepath: str) -> Optional[str]:
    try:
        with open(filepath, "r", encoding="utf-8", errors="replace") as f:
            return f.read()
    except (OSError, IOError):
        return None


# ---------------------------------------------------------------------------
# Checker functions for complex rules
# ---------------------------------------------------------------------------

def check_ets099(filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS099: verifier config must have all check stages."""
    required = {"cflow", "resolve-id", "typing", "absint"}
    results: List[Tuple[int, str]] = []
    in_check = False
    check_start = 0
    check_content = ""

    for i, line in enumerate(content.splitlines(), 1):
        stripped = line.strip()
        if "check" in stripped and "{" in stripped:
            in_check = True
            check_start = i
            check_content = stripped
        elif in_check:
            check_content += " " + stripped
            if "}" in stripped:
                in_check = False
                found = set(re.findall(r"[\w-]+", check_content)) & required
                missing = required - found
                if missing:
                    results.append((
                        check_start,
                        f"Verifier config missing stages: {', '.join(sorted(missing))}"
                    ))
    return results


def check_ets801(filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS801: exported functions must be camelCase."""
    results: List[Tuple[int, str]] = []
    pat = re.compile(r"\bexport\s+function\s+(\w+)")
    for i, line in enumerate(content.splitlines(), 1):
        m = pat.search(line)
        if m:
            name = m.group(1)
            if not is_camel_case(name):
                results.append((i, f"Exported function '{name}' must use camelCase naming"))
    return results


def check_ets802(filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS802: public methods must be camelCase."""
    results: List[Tuple[int, str]] = []
    pat = re.compile(r"\bpublic\s+(?:static\s+)?(?:native\s+)?(?:override\s+)?(\w+)\s*[(<]")
    skip = {"constructor"}
    for i, line in enumerate(content.splitlines(), 1):
        m = pat.search(line)
        if m:
            name = m.group(1)
            if name not in skip and not is_camel_case(name):
                results.append((i, f"Public method '{name}' must use camelCase naming"))
    return results


def check_ets803(filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS803: exported types must be PascalCase."""
    results: List[Tuple[int, str]] = []
    pat = re.compile(r"\bexport\s+(?:abstract\s+)?(?:final\s+)?(class|interface|enum|type)\s+(\w+)")
    for i, line in enumerate(content.splitlines(), 1):
        m = pat.search(line)
        if m:
            name = m.group(2)
            if not is_pascal_case(name):
                results.append((i, f"Exported {m.group(1)} '{name}' must use PascalCase naming"))
    return results


ETS804_WHITELIST = frozenset({"NaN", "Infinity"})


def check_ets804(filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS804: top-level constants must be SCREAMING_SNAKE_CASE."""
    results: List[Tuple[int, str]] = []
    pat = re.compile(r"^(?:export\s+)?const\s+(\w+)")
    for i, line in enumerate(content.splitlines(), 1):
        m = pat.match(line)
        if m:
            name = m.group(1)
            if name in ETS804_WHITELIST:
                continue
            if not is_screaming_snake(name) and not is_camel_case(name):
                results.append((i, f"Global constant '{name}' should use SCREAMING_SNAKE_CASE naming"))
    return results


def strip_line_for_pattern(line: str) -> str:
    """Remove string literals and comments to avoid false positives."""
    stripped = line.lstrip()
    # Skip block comment lines (JSDoc /** */ and /* */ continuations)
    if stripped.startswith("*") or stripped.startswith("/*"):
        return ""
    result = re.sub(r'"[^"]*"', '""', line)
    result = re.sub(r"'[^']*'", "''", result)
    result = re.sub(r"//.*$", "", result)
    return result


def check_ets813(_filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS813: integer values must not use exponential notation."""
    pat = re.compile(r"(?<!\.)\b\d+[eE][+]?\d+\b")
    results: List[Tuple[int, str]] = []
    for i, line in enumerate(content.splitlines(), 1):
        if pat.search(strip_line_for_pattern(line)):
            results.append((i, "Do not use exponential notation for integer values; write in standard form"))
    return results


def check_ets011_removed(_filepath: str, _content: str) -> List[Tuple[int, str]]:
    """ETS011: stub — actual check done per-line in diff mode."""
    return []


def check_ets490_added(filepath: str, content: str) -> List[Tuple[int, str]]:
    """ETS490: new AST nodes in astNodeMapping.h need spec verification."""
    if "astNodeMapping" not in os.path.basename(filepath):
        return []
    results: List[Tuple[int, str]] = []
    pat = re.compile(r"^\s*\w+\s*\(")
    for i, line in enumerate(content.splitlines(), 1):
        if pat.match(line):
            results.append((i, "New AST node added — verify it matches the language spec grammar"))
    return results


# ---------------------------------------------------------------------------
# Rule definitions
# ---------------------------------------------------------------------------

def _not_test(path: str) -> bool:
    return not is_test_path(path)


RULES: List[Rule] = [
    # --- Simple grep / regex rules ---
    Rule(
        rule_id="ETS097", severity="Error",
        file_globs=["*"],
        message="Do not disable bytecode verification (--verification-mode=disabled)",
        pattern=re.compile(r"--verification-mode=disabled"),
    ),
    Rule(
        rule_id="ETS098", severity="Error",
        file_globs=["*.cpp", "*.h"],
        message="Do not disable bytecode verification in gtest runtime options",
        pattern=re.compile(r'verification.mode.*disabled|SetDefaultVerificationMode.*false', re.IGNORECASE),
    ),
    Rule(
        rule_id="ETS402", severity="Error",
        file_globs=["*.cpp", "*.h"],
        message="Do not use EAllocator type identifier directly",
        pattern=re.compile(r"\bEAllocator\b"),
    ),
    Rule(
        rule_id="ETS403", severity="Error",
        file_globs=["*.cpp", "*.h"],
        message="Do not manually create scoped allocators (CreateScopedAllocator/NewScopedAllocator)",
        pattern=re.compile(r"(CreateScopedAllocator|NewScopedAllocator)\s*\("),
    ),
    Rule(
        rule_id="ETS807", severity="Warning",
        file_globs=["*.ets", "*.sts"],
        message="Avoid 'typeof' operator; use 'instanceof' instead",
        pattern=re.compile(r"\btypeof\b"),
        path_filter=_not_test,
    ),
    Rule(
        rule_id="ETS502", severity="Error",
        file_globs=["*.ets", "*.sts"],
        message="Use 'instanceof' instead of 'typeof' in tests",
        pattern=re.compile(r"\btypeof\b"),
        path_filter=is_test_path,
    ),
    Rule(
        rule_id="ETS814", severity="Warning",
        file_globs=["*.ets", "*.sts"],
        message="Do not use 'overload' set syntax; declare overloaded functions explicitly",
        pattern=re.compile(r"\boverload\s+\w+\s*\{"),
    ),
    Rule(
        rule_id="ETS805", severity="Warning",
        file_globs=["*.ets", "*.sts"],
        message="Avoid 'for..of' loop; use traditional index-based 'for' loop",
        pattern=re.compile(r"\bfor\s*\([^)]*\bof\b"),
    ),
    Rule(
        rule_id="ETS806", severity="Warning",
        file_globs=["*.ets", "*.sts"],
        message="Do not use labeled statements on loops",
        pattern=re.compile(r"^\s*\w+\s*:\s*(for|while|do)\b"),
    ),
    Rule(
        rule_id="ETS812", severity="Warning",
        file_globs=["*.ets", "*.sts"],
        message="Use StringBuilder instead of '+' for string concatenation",
        pattern=re.compile(r'''(?:"[^"]*"|'[^']*')\s*\+|\+\s*(?:"[^"]*"|'[^']*')'''),
    ),
    Rule(
        rule_id="ETS813", severity="Warning",
        file_globs=["*.ets", "*.sts"],
        message="Do not use exponential notation for integer values; write in standard form",
        checker=check_ets813,
    ),
    # --- Checker-based rules ---
    Rule(
        rule_id="ETS099", severity="Error",
        file_globs=["*.verifier.config"],
        message="Verifier config must include all required check stages",
        checker=check_ets099,
    ),
    Rule(
        rule_id="ETS801", severity="Error",
        file_globs=["*.ets", "*.sts"],
        message="Exported function must use camelCase naming",
        checker=check_ets801,
    ),
    Rule(
        rule_id="ETS802", severity="Error",
        file_globs=["*.ets", "*.sts"],
        message="Public method must use camelCase naming",
        checker=check_ets802,
    ),
    Rule(
        rule_id="ETS803", severity="Error",
        file_globs=["*.ets", "*.sts"],
        message="Exported type must use PascalCase naming",
        checker=check_ets803,
    ),
    Rule(
        rule_id="ETS804", severity="Error",
        file_globs=["*.ets", "*.sts"],
        message="Global constant should use SCREAMING_SNAKE_CASE naming",
        checker=check_ets804,
    ),
    # --- Diff-only rules ---
    Rule(
        rule_id="ETS011", severity="Error",
        file_globs=["*"],
        message="Do not remove assertions from the codebase",
        pattern=re.compile(r"\b(ASSERT|ES2PANDA_ASSERT)\b|arktest\.assert"),
        diff_only=True,
        diff_removed_only=True,
    ),
    Rule(
        rule_id="ETS490", severity="Error",
        file_globs=["*.h"],
        message="New AST node added — verify it matches the language spec grammar",
        checker=check_ets490_added,
        diff_only=True,
        diff_added_only=True,
    ),
]


# ---------------------------------------------------------------------------
# Runners
# ---------------------------------------------------------------------------

def check_line(filepath: str, line_no: int, line_text: str, rule: Rule) -> Optional[Violation]:
    """Check a single line against a pattern-based rule."""
    if rule.pattern and rule.pattern.search(line_text):
        return Violation(rule.rule_id, rule.severity, filepath, line_no, rule.message)
    return None


def check_file_with_rule(filepath: str, content: str, rule: Rule) -> List[Violation]:
    """Run a single rule against a file's content."""
    if rule.path_filter and not rule.path_filter(filepath):
        return []

    if rule.checker:
        results = rule.checker(filepath, content)
        return [Violation(rule.rule_id, rule.severity, filepath, ln, msg) for ln, msg in results]

    violations: List[Violation] = []
    for i, line in enumerate(content.splitlines(), 1):
        v = check_line(filepath, i, line, rule)
        if v:
            violations.append(v)
    return violations


def run_full_scan(path: str, rules: List[Rule]) -> List[Violation]:
    """Scan all files under path."""
    violations: List[Violation] = []
    active_rules = [r for r in rules if not r.diff_only]

    for root, dirs, files in os.walk(path):
        dirs[:] = sorted(
            d for d in dirs
            if d not in SKIP_DIRS
            and not d.startswith(".")
            and not any(
                os.path.join(root, d).replace("\\", "/").endswith(p)
                for p in SKIP_PATHS
            )
        )
        for fname in sorted(files):
            filepath = os.path.join(root, fname)
            rel_path = os.path.relpath(filepath, path)
            applicable = [r for r in active_rules if matches_glob(filepath, r.file_globs)]
            if not applicable:
                continue
            content = read_file_safe(filepath)
            if content is None:
                continue
            for rule in applicable:
                violations.extend(check_file_with_rule(rel_path, content, rule))
    return violations


def run_diff_scan(diff_text: str, rules: List[Rule]) -> List[Violation]:
    """Analyze a unified diff."""
    violations: List[Violation] = []
    diff_files = parse_unified_diff(diff_text)

    for df in diff_files:
        applicable = [r for r in rules if matches_glob(df.file_path, r.file_globs)]
        for rule in applicable:
            if rule.path_filter and not rule.path_filter(df.file_path):
                continue

            if rule.diff_removed_only:
                lines = df.removed_lines
            elif rule.diff_added_only:
                lines = df.added_lines
            elif rule.diff_only:
                lines = df.added_lines
            else:
                lines = df.added_lines

            if rule.checker:
                combined = "\n".join(lines.values())
                results = rule.checker(df.file_path, combined)
                for ln, msg in results:
                    violations.append(Violation(rule.rule_id, rule.severity, df.file_path, ln, msg))
            else:
                for line_no, line_text in lines.items():
                    v = check_line(df.file_path, line_no, line_text, rule)
                    if v:
                        violations.append(v)
    return violations


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def get_git_diff(cached: bool = True) -> str:
    """Run git diff and return its output."""
    cmd = ["git", "diff", "--unified=0"]
    if cached:
        cmd.append("--cached")
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.stdout


def filter_rules(rules: List[Rule], include: Optional[str], exclude: Optional[str]) -> List[Rule]:
    """Filter rules by include/exclude CLI options."""
    if include:
        ids = set(include.split(","))
        rules = [r for r in rules if r.rule_id in ids]
    if exclude:
        ids = set(exclude.split(","))
        rules = [r for r in rules if r.rule_id not in ids]
    return rules


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Deterministic linter for arkcompiler review rules",
    )
    parser.add_argument(
        "--mode", choices=["diff", "full"], default="diff",
        help="Analysis mode (default: diff)",
    )
    parser.add_argument(
        "--path",
        help="Directory to scan (required for --mode=full)",
    )
    parser.add_argument(
        "--diff-input",
        help="Read diff from file (use - for stdin). Default: run 'git diff --cached'",
    )
    parser.add_argument(
        "--rules",
        help="Comma-separated rule IDs to run (default: all)",
    )
    parser.add_argument(
        "--exclude-rules",
        help="Comma-separated rule IDs to skip",
    )
    parser.add_argument(
        "--verbose", action="store_true",
        help="Print progress to stderr",
    )

    args = parser.parse_args()

    if args.mode == "full" and not args.path:
        parser.error("--path is required when --mode=full")
        return 2

    rules = filter_rules(list(RULES), args.rules, args.exclude_rules)

    if args.mode == "full":
        if args.verbose:
            print(f"Scanning {args.path} with {len(rules)} rules...", file=sys.stderr)
        violations = run_full_scan(args.path, rules)
    else:
        if args.diff_input == "-":
            diff_text = sys.stdin.read()
        elif args.diff_input:
            with open(args.diff_input, "r") as f:
                diff_text = f.read()
        else:
            diff_text = get_git_diff()
        if args.verbose:
            print(f"Analyzing diff with {len(rules)} rules...", file=sys.stderr)
        violations = run_diff_scan(diff_text, rules)

    violations.sort(key=lambda v: (v.file, v.line, v.rule_id))

    for v in violations:
        print(v)

    if violations:
        errors = sum(1 for v in violations if v.severity == "Error")
        warnings = sum(1 for v in violations if v.severity == "Warning")
        parts = []
        if errors:
            parts.append(f"{errors} error(s)")
        if warnings:
            parts.append(f"{warnings} warning(s)")
        print(f"\nFound {', '.join(parts)}.", file=sys.stderr)
        return 1

    if args.verbose:
        print("No violations found.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
