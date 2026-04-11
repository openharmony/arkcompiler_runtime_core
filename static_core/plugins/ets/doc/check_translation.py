#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

import argparse
import json
import subprocess
import sys
from pathlib import Path


STATUS_TRANSLATED = "translated"
STATUS_OUTDATED = "outdated"

ENGLISH_TO_CHINESE = {
    "spec_700": "zh/spec_700",
    "spec_700_concurrency": "zh/spec_700_concurrency",
    "spec_700_runtime": "zh/spec_700_runtime",
}


def run_git(repo_root: Path, args):
    return subprocess.run(
        ["git", *args],
        cwd=repo_root,
        capture_output=True,
        text=True,
        check=False,
    )


def gather_expected_files(source_root: Path):
    expected = []
    for en_dir, zh_dir in ENGLISH_TO_CHINESE.items():
        for path in sorted((source_root / en_dir).glob("*.rst")):
            rel = path.relative_to(source_root).as_posix()
            expected.append({"source_path": rel, "zh_path": f"{zh_dir}/{path.name}"})
    return expected


def load_manifest(manifest_path: Path):
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    entries = manifest.get("files", [])
    return manifest, entries, {entry["zh_path"]: entry for entry in entries}


def maybe_update_manifest_status(manifest_entry, is_outdated: bool):
    current_status = manifest_entry.get("translation_status", "")
    updated = False

    if is_outdated:
        if current_status == STATUS_TRANSLATED:
            manifest_entry["translation_status"] = STATUS_OUTDATED
            updated = True
    else:
        if current_status == STATUS_OUTDATED:
            manifest_entry["translation_status"] = STATUS_TRANSLATED
            updated = True

    return updated


def check_outdated_file(
    repo_root: Path,
    source_root: Path,
    source_path: str,
    source_commit: str,
    against_ref: str,
):
    source_file = (source_root / source_path).resolve()
    if not source_file.is_file():
        return {"status": "error", "message": f"source file not found: {source_path}"}

    try:
        repo_relative_source = source_file.relative_to(repo_root).as_posix()
    except ValueError:
        return {
            "status": "error",
            "message": f"source file is outside the git repository: {source_file}",
        }

    diff_result = run_git(
        repo_root,
        ["diff", "--quiet", source_commit, against_ref, "--", repo_relative_source],
    )
    if diff_result.returncode == 0:
        return {
            "status": "current",
            "source_path": source_path,
            "source_commit": source_commit,
        }
    if diff_result.returncode != 1:
        error_text = diff_result.stderr.strip() or diff_result.stdout.strip() or "git diff failed"
        return {"status": "error", "message": error_text}

    log_result = run_git(
        repo_root,
        [
            "log",
            "--oneline",
            "-1",
            f"{source_commit}..{against_ref}",
            "--",
            repo_relative_source,
        ],
    )
    latest_change = ""
    if log_result.returncode == 0:
        latest_change = log_result.stdout.strip()

    return {
        "status": "outdated",
        "source_path": source_path,
        "source_commit": source_commit,
        "latest_change": latest_change,
    }


def build_parser():
    parser = argparse.ArgumentParser(
        description="Check ETS Chinese translation layout and drift."
    )
    parser.add_argument("--against-ref", default="HEAD")
    parser.add_argument("--update-manifest-status", action="store_true")
    parser.add_argument("--layout-only", action="store_true")
    return parser


def check_layout(source_root: Path, zh_root: Path, manifest_path: Path):
    if not manifest_path.is_file():
        print(f"FATAL: manifest not found: {manifest_path}", file=sys.stderr)
        return 2

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    entries = manifest.get("files", [])
    manifest_map = {entry["zh_path"]: entry for entry in entries}

    expected = gather_expected_files(source_root)
    missing_files = []
    missing_manifest_entries = []

    for item in expected:
        zh_file = source_root / item["zh_path"]
        if not zh_file.is_file():
            missing_files.append(item["zh_path"])
        if item["zh_path"] not in manifest_map:
            missing_manifest_entries.append(item["zh_path"])

    extra_manifest_entries = []
    expected_paths = {item["zh_path"] for item in expected}
    for zh_path in sorted(manifest_map):
        if zh_path not in expected_paths:
            extra_manifest_entries.append(zh_path)

    if missing_files or missing_manifest_entries or extra_manifest_entries:
        if missing_files:
            print("FATAL: missing Chinese files:", file=sys.stderr)
            for item in missing_files:
                print(f"  {item}", file=sys.stderr)
        if missing_manifest_entries:
            print("FATAL: missing manifest entries:", file=sys.stderr)
            for item in missing_manifest_entries:
                print(f"  {item}", file=sys.stderr)
        if extra_manifest_entries:
            print("FATAL: extra manifest entries:", file=sys.stderr)
            for item in extra_manifest_entries:
                print(f"  {item}", file=sys.stderr)
        return 1

    print(f"translation layout is valid: {len(expected)} mirrored files tracked under {zh_root}")
    return 0


def check_outdated(args, source_root: Path, zh_root: Path, manifest_path: Path):
    repo_result = run_git(source_root, ["rev-parse", "--show-toplevel"])
    if repo_result.returncode != 0:
        print(
            "FATAL: failed to locate git repository root: {error}".format(
                error=repo_result.stderr.strip() or repo_result.stdout.strip()
            ),
            file=sys.stderr,
        )
        return 2
    repo_root = Path(repo_result.stdout.strip()).resolve()

    ref_result = run_git(repo_root, ["rev-parse", "--short", args.against_ref])
    if ref_result.returncode != 0:
        print(
            "FATAL: failed to resolve git reference {ref}: {error}".format(
                ref=args.against_ref,
                error=ref_result.stderr.strip() or ref_result.stdout.strip(),
            ),
            file=sys.stderr,
        )
        return 2
    resolved_ref = ref_result.stdout.strip()

    if not manifest_path.is_file():
        print(f"FATAL: manifest not found: {manifest_path}", file=sys.stderr)
        return 2
    manifest, entries, manifest_map = load_manifest(manifest_path)

    if not entries:
        print(f"FATAL: no files tracked in {manifest_path}", file=sys.stderr)
        return 2

    current_files = []
    outdated_files = []
    errors = []
    updated_manifest_entries = 0

    for manifest_entry in sorted(entries, key=lambda entry: entry.get("zh_path", "")):
        zh_rel = manifest_entry.get("zh_path", "")
        source_path = manifest_entry.get("source_path", "")
        source_commit = manifest_entry.get("source_commit", "")

        if not zh_rel:
            errors.append(("<manifest>", "missing zh_path"))
            continue

        missing_fields = [
            field
            for field, value in (
                ("source_path", source_path),
                ("source_commit", source_commit),
                ("translation_status", manifest_entry.get("translation_status", "")),
            )
            if not value
        ]
        if missing_fields:
            errors.append((zh_rel, "manifest entry is incomplete: missing {fields}".format(fields=", ".join(missing_fields))))
            continue

        zh_file = source_root / zh_rel
        if not zh_file.is_file():
            errors.append((zh_rel, "translated file not found"))
            continue

        result = check_outdated_file(
            repo_root=repo_root,
            source_root=source_root,
            source_path=source_path,
            source_commit=source_commit,
            against_ref=args.against_ref,
        )

        if result["status"] == "current":
            current_files.append(zh_rel)
            if args.update_manifest_status and maybe_update_manifest_status(manifest_entry, False):
                updated_manifest_entries += 1
            continue
        if result["status"] == "outdated":
            outdated_files.append((zh_rel, result))
            if args.update_manifest_status and maybe_update_manifest_status(manifest_entry, True):
                updated_manifest_entries += 1
            continue
        errors.append((zh_rel, result["message"]))

    print(f"Checked {len(entries)} Chinese .rst files under {zh_root}")
    print(f"English reference: {args.against_ref} ({resolved_ref})")
    print(f"Current: {len(current_files)}")
    print(f"Outdated: {len(outdated_files)}")
    print(f"Errors: {len(errors)}")
    if args.update_manifest_status and manifest_map:
        print(f"Manifest status updates: {updated_manifest_entries}")

    if outdated_files:
        print("\nOutdated files:")
        for zh_rel, result in outdated_files:
            print(f"- {zh_rel}")
            print(f"  source: {result['source_path']}")
            print(f"  base: {result['source_commit']}")
            if result["latest_change"]:
                print(f"  latest: {result['latest_change']}")

    if errors:
        print("\nErrors:")
        for zh_rel, message in errors:
            print(f"- {zh_rel}: {message}")

    if args.update_manifest_status and manifest_map and not errors and updated_manifest_entries:
        manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    if errors:
        return 2
    if outdated_files:
        return 1
    return 0


def main():
    parser = build_parser()
    args = parser.parse_args()

    source_root = Path(__file__).resolve().parent
    zh_root = source_root / "zh"
    manifest_path = zh_root / "translation_manifest.json"

    if args.layout_only:
        return check_layout(source_root, zh_root, manifest_path)

    layout_status = check_layout(source_root, zh_root, manifest_path)
    if layout_status != 0:
        return layout_status
    return check_outdated(args, source_root, zh_root, manifest_path)


if __name__ == "__main__":
    raise SystemExit(main())
