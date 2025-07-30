#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import os
import stat
import sys
import shutil
import json
import re
from pathlib import Path


def load_json_file(file_path: Path) -> dict:
    """Load JSON file; return parsed data"""
    try:
        with os.fdopen(os.open(file_path, os.O_RDONLY, stat.S_IRUSR), 'r', encoding='utf-8') as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error: Hiddable Interface file format is invalid in ({file_path}): {e}")
        return {}


def read_hidden_interfaces(std_dir: Path) -> dict:
    json_files = [
        "hiddable_APIs.json"
    ]
    hidden_interfaces = {}
    for json_file in json_files:
        hidden_interfaces_file = std_dir / json_file

        if not hidden_interfaces_file.exists():
            print("Error: Hiddable Interface file not found. Will Copy All files.")
            return hidden_interfaces

        data = load_json_file(hidden_interfaces_file)
        for item in data:
            file = item.get('file')
            interfaces = item.get('interfaces', [])
            if file:
                hidden_interfaces[file] = set(interfaces)

    return hidden_interfaces


def process_file_with_hidden_interfaces(
    source_file: Path,
    target_file: Path,
    hidden_ifaces: set,
    interface_pattern: re.Pattern) -> None:
    """Replace export keyword with empty string if interface is hiddable"""
    with os.fdopen(os.open(source_file, os.O_RDONLY, stat.S_IRUSR), 'r', encoding='utf-8') as f:
        lines = f.readlines()

    modified_lines = []

    for line in lines:
        match = interface_pattern.search(line)
        if match:
            interface_type, interface_name = match.groups()
            if interface_name in hidden_ifaces:
                modified_line = re.sub(r'\bexport \b', '', line)
                modified_lines.append(modified_line)
                continue

        modified_lines.append(line)

    with os.fdopen(
        os.open(target_file, os.O_RDWR | os.O_CREAT | os.O_TRUNC, stat.S_IWUSR | stat.S_IRUSR), 'w') as fout:
        fout.writelines(modified_lines)


def copy_files_with_filter(std_dir: Path, target_dir: Path, hidden_interfaces: dict) -> None:
    """Copy stdlib files; filter interfaces by hiddable_APIs.json"""
    copied_count = 0
    skipped_count = 0
    filtered_files = 0

    interface_pattern = re.compile(
        r'^\s*export\s+(?:final\s+|abstract\s+)?(class|interface|@interface|function)\s+(\w+)'
    )

    for root, _, files in os.walk(std_dir):
        rel_path = Path(root).relative_to(std_dir)
        target_subdir = target_dir / 'std' / rel_path

        target_subdir.mkdir(parents=True, exist_ok=True)

        for file in files:
            source_file = Path(root) / file
            rel_file_path = rel_path / file
            rel_file_str = str(rel_file_path)

            if rel_file_str not in hidden_interfaces:
                shutil.copy2(source_file, target_subdir)
                copied_count += 1
                continue

            hidden_ifaces = hidden_interfaces[rel_file_str]

            if not hidden_ifaces:
                skipped_count += 1
                continue
            filtered_files += 1
            target_file = target_subdir / file
            process_file_with_hidden_interfaces(source_file, target_file, hidden_ifaces, interface_pattern)
            copied_count += 1


def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <STD_DIR> <ESCOMPACT_DIR> <TARGET_DIR>")
        sys.exit(1)

    std_dir = Path(sys.argv[1])
    escompact_dir = Path(sys.argv[2])
    target_dir = Path(sys.argv[3])

    target_dir.mkdir(parents=True, exist_ok=True)
    hidden_interfaces = read_hidden_interfaces(std_dir)
    copy_files_with_filter(std_dir, target_dir, hidden_interfaces)

    escompat_target = target_dir / escompact_dir.name
    shutil.copytree(escompact_dir, escompat_target, dirs_exist_ok=True)


if __name__ == "__main__":
    main()
