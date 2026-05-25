#!/usr/bin/env python3
# -- coding: utf-8 --
#
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
#
# Verify that one test file under 24.build_system/metadata/ is identical to
# its counterpart under 24.build_system/binary-metadata/ except for the
# `es2panda_options` field in the test metadata block (the YAML between
# `/*---` and `---*/` comments — same notion as runner.suites.test_metadata
# TestMetadata), which must change in a known way (--emit-declaration ↔
# --emit-metadata + --read-metadata).
#
# Invoked per-test by cfg/workflows/panda-metadata-parity.yaml. The workflow
# passes the LHS source file path (absolute, under .../metadata/...); this
# script derives the RHS path by swapping the "metadata" path segment with
# "binary-metadata". Exit 0 → parity OK; exit 1 → drift detected (with a
# diagnostic printed to stderr).

import sys
from pathlib import Path

import yaml

# For each accepted `es2panda_options` LHS pattern (in metadata/), the set
# of acceptable RHS patterns (in binary-metadata/). The same LHS can map to
# multiple valid RHS forms because a file may or may not be a metadata
# consumer (with-or-without `read-metadata`).
EXPECTED_OPTIONS_MAP: dict[frozenset[str], set[frozenset[str]]] = {
    frozenset():
        {frozenset(),
         frozenset({"read-metadata"})},
    frozenset({"emit-declaration"}):
        {frozenset({"emit-metadata"}),
         frozenset({"emit-metadata", "read-metadata"})},
    frozenset({"emit-declaration", "simultaneous"}):
        {frozenset({"emit-metadata", "simultaneous"}),
         frozenset({"emit-metadata", "simultaneous", "read-metadata"})},
    frozenset({"simultaneous"}):
        {frozenset({"simultaneous"})},
}

LHS_SEGMENT = "metadata"
RHS_SEGMENT = "binary-metadata"
SIBLING_SUFFIXES = (".json", ".yaml")
# Delimiters of the per-test metadata block 
METADATA_BLOCK_START = "/*---"
METADATA_BLOCK_END = "---*/"


def derive_rhs(lhs_path: Path) -> Path:
    """Swap the FIRST 'metadata' path segment to 'binary-metadata'."""
    parts = lhs_path.parts
    try:
        idx = parts.index(LHS_SEGMENT)
    except ValueError as exc:
        raise SystemExit(
            f"cannot derive RHS: '{lhs_path}' does not contain a '{LHS_SEGMENT}' path segment"
        ) from exc
    return Path(*parts[:idx], RHS_SEGMENT, *parts[idx + 1:])


def extract_metadata(text: str) -> tuple[dict | None, str]:
    """Extract the test-metadata block delimited by `/*--- ... ---*/`.

    Returns (metadata_dict, body_without_metadata). metadata_dict is None
    when the file has no metadata block (e.g. arktsconfig*.json).
    """
    start = text.find(METADATA_BLOCK_START)
    if start == -1:
        return None, text
    inner_start = start + len(METADATA_BLOCK_START)
    end = text.find(METADATA_BLOCK_END, inner_start)
    if end == -1:
        return None, text
    try:
        metadata = yaml.safe_load(text[inner_start:end]) or {}
    except yaml.YAMLError as exc:
        raise SystemExit(f"failed to parse test metadata YAML: {exc}") from exc
    body = text[:start] + text[end + len(METADATA_BLOCK_END):]
    return metadata, body


def _opts(value) -> frozenset[str]:
    """Coerce a YAML `es2panda_options` value into a frozenset[str]."""
    return frozenset(value or [])


def _check_options_swap(lhs_meta: dict, rhs_meta: dict) -> str | None:
    """Validate the es2panda_options swap rule. Mutates both metadata dicts —
    pops the `es2panda_options` key so the caller can compare the remaining
    fields byte-identical."""
    lhs_opts = _opts(lhs_meta.pop("es2panda_options", None))
    rhs_opts = _opts(rhs_meta.pop("es2panda_options", None))
    accepted_rhs = EXPECTED_OPTIONS_MAP.get(lhs_opts)
    if accepted_rhs is None:
        return (f"unknown LHS es2panda_options pattern {sorted(lhs_opts)} — "
                "add to EXPECTED_OPTIONS_MAP if intentional")
    if rhs_opts not in accepted_rhs:
        accepted_pretty = sorted(sorted(s) for s in accepted_rhs)
        return (f"es2panda_options swap mismatch: LHS={sorted(lhs_opts)} → "
                f"RHS={sorted(rhs_opts)}, accepted RHS forms: {accepted_pretty}")
    return None


def _check_remaining_metadata(lhs_meta: dict, rhs_meta: dict) -> str | None:
    """All non-options test-metadata fields must be byte-identical."""
    if lhs_meta == rhs_meta:
        return None
    diff_keys = sorted(
        k for k in set(lhs_meta) | set(rhs_meta)
        if lhs_meta.get(k) != rhs_meta.get(k)
    )
    return f"test metadata differs on non-options field(s): {diff_keys}"


def _check_metadata(lhs_meta: dict | None, rhs_meta: dict | None) -> str | None:
    """Dispatch on metadata-block presence and run metadata-specific checks.

    Single exit point — accumulates into `result` and returns once. Required
    by the project style rule "return value types of all branches must be
    the same": with multiple `return None` / `return str` statements the
    linter sees them as differently-typed returns even though both satisfy
    the declared `str | None` signature.
    """
    # Non-test files (.json/.yaml) have no metadata block — body equality
    # (already verified by the caller) is sufficient.
    result: str | None = None
    if lhs_meta is None and rhs_meta is None:
        pass
    elif lhs_meta is None or rhs_meta is None:
        result = "test metadata block present on one side only"
    else:
        result = _check_options_swap(lhs_meta, rhs_meta)
        if result is None:
            result = _check_remaining_metadata(lhs_meta, rhs_meta)
    return result


def compare(lhs_path: Path, rhs_path: Path) -> str | None:
    """Return None if files are parity-equivalent, else a one-line diff reason."""
    if not lhs_path.exists():
        return f"LHS missing: {lhs_path}"
    if not rhs_path.exists():
        return f"RHS missing (binary-metadata counterpart absent): {rhs_path}"

    lhs_text = lhs_path.read_text(encoding="utf-8")
    # Normalize the only expected text-level delta — path segment swap inside
    # arktsconfig*.json and similar config files referencing the test tree.
    rhs_text = rhs_path.read_text(encoding="utf-8").replace(
        f"/{RHS_SEGMENT}/", f"/{LHS_SEGMENT}/"
    )

    lhs_meta, lhs_body = extract_metadata(lhs_text)
    rhs_meta, rhs_body = extract_metadata(rhs_text)

    if lhs_body.strip() != rhs_body.strip():
        return f"code-body diff between {lhs_path.name} and its binary-metadata counterpart"
    return _check_metadata(lhs_meta, rhs_meta)


def _report(lhs_path: Path, rhs_path: Path, reason: str) -> None:
    """Emit a uniform `metadata-parity FAIL:` diagnostic to stderr."""
    print(f"metadata-parity FAIL: {reason}", file=sys.stderr)
    print(f"  LHS: {lhs_path}", file=sys.stderr)
    print(f"  RHS: {rhs_path}", file=sys.stderr)


def _is_visible_sibling(sib: Path, lhs: Path) -> bool:
    return (
        sib != lhs
        and sib.is_file()
        and sib.suffix.lower() in SIBLING_SUFFIXES
    )


def _check_pair(lhs_path: Path) -> int:
    """Compare lhs_path to its derived RHS; return 1 if drift, 0 otherwise."""
    rhs_path = derive_rhs(lhs_path)
    diff = compare(lhs_path, rhs_path)
    if diff is None:
        return 0
    _report(lhs_path, rhs_path, diff)
    return 1


def _check_siblings(lhs: Path) -> int:
    """Diff every sibling *.json / *.yaml in the test's directory. Runner
    discovery is .ets-only, so these would otherwise escape per-test checks."""
    if not lhs.is_file():
        return 0
    failures = 0
    for sib in sorted(lhs.parent.iterdir()):
        if not _is_visible_sibling(sib, lhs):
            continue
        failures += _check_pair(sib)
    return failures


def _check_orphans(lhs: Path) -> int:
    """Report files present in binary-metadata/<dir> with no metadata/<dir>
    counterpart. Discovery walks LHS only, so RHS-only files would otherwise
    never trigger compare()."""
    lhs_dir = lhs.parent if lhs.is_file() else lhs
    rhs_dir = derive_rhs(lhs_dir)
    if not rhs_dir.is_dir():
        return 0
    lhs_names = {p.name for p in lhs_dir.iterdir() if not p.name.startswith(".")}
    failures = 0
    for p in sorted(rhs_dir.iterdir()):
        if p.name.startswith(".") or p.name in lhs_names:
            continue
        print(
            "metadata-parity FAIL: orphan file in binary-metadata/ "
            f"(no metadata/ counterpart): {p}",
            file=sys.stderr,
        )
        failures += 1
    return failures


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <metadata-source-file>", file=sys.stderr)
        return 2
    lhs = Path(sys.argv[1])

    failures = _check_pair(lhs) + _check_siblings(lhs) + _check_orphans(lhs)
    return 1 if failures > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
