#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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
import os
import shutil
import subprocess
import zipfile


def parse_args():
    parser = argparse.ArgumentParser(description="Verify abc files in system app.")
    parser.add_argument(
        "--hap-dir", required=True, help="Path to the HAP files directory.")
    parser.add_argument(
        "--verifier-dir", required=True, help="Path to the ark_verifier directory.")
    return parser.parse_args()


def copy_and_rename_hap_files(hap_folder, out_folder):
    for file_path in os.listdir(hap_folder):
        if file_path.endswith(".hap"):
            destination_path = os.path.join(out_folder, file_path.replace(".hap", ".zip"))
            shutil.copy(os.path.join(hap_folder, file_path), destination_path)


def extract_zip(zip_path, extract_folder):
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_folder)
    except zipfile.BadZipFile as e:
        print(f"Error extracting {zip_path}: {e}")


def verify_file(file_path, ark_verifier_path):
    verification_command = [ark_verifier_path, "--input_file", file_path]
    result = subprocess.run(verification_command, capture_output=True, text=True)
    status = 'pass' if result.returncode == 0 else 'fail'
    print(f"Verifying: {file_path} {status}")
    return result.returncode == 0


def process_directory(directory, ark_verifier_path):
    total_count = 0
    passed_count = 0
    failed_abc_list = []

    for root, dirs, files in os.walk(directory):
        for file in files:
            if not file.endswith(".abc"):
                continue
            abc_path = os.path.join(root, file)
            if verify_file(abc_path, ark_verifier_path):
                passed_count += 1
            else:
                failed_abc_list.append(os.path.relpath(abc_path, hap_folder))
            total_count += 1

    return total_count, passed_count, failed_abc_list

def verify_hap(hap_folder, ark_verifier_path):
    failed_abc_list = []
    passed_count = 0
    total_count = 0

    for file in os.listdir(hap_folder):
        if not file.endswith(".zip"):
            continue

        zip_path = os.path.join(hap_folder, file)
        extract_folder = os.path.join(hap_folder, file.replace(".zip", ""))
        extract_zip(zip_path, extract_folder)

        ets_path = os.path.join(extract_folder, "ets")
        if not os.path.exists(ets_path):
            continue

        modules_abc_path = os.path.join(ets_path, "modules.abc")
        if os.path.isfile(modules_abc_path):
            if verify_file(modules_abc_path, ark_verifier_path):
                passed_count += 1
            else:
                failed_abc_list.append(os.path.relpath(modules_abc_path, hap_folder))
            total_count += 1
        else:
            total_inc, passed_inc, failed_abc_inc = process_directory(ets_path, ark_verifier_path)
            total_count += total_inc
            passed_count += passed_inc
            failed_abc_list.extend(failed_abc_inc)

    return total_count, passed_count, len(failed_abc_list), failed_abc_list


def main():
    args = parse_args()

    hap_folder_path = os.path.abspath(args.hap_dir)
    ark_verifier_path = os.path.abspath(os.path.join(args.verifier_dir, "ark_verifier"))

    out_folder = os.path.join(os.path.dirname(__file__), "out")
    os.makedirs(out_folder, exist_ok=True)

    copy_and_rename_hap_files(hap_folder_path, out_folder)

    total_count, passed_count, failed_count, failed_abc_list = verify_hap(out_folder, ark_verifier_path)

    print("Summary(abc verification):")
    print(f"Total: {total_count}")
    print(f"Passed: {passed_count}")
    print(f"Failed: {failed_count}")

    if failed_count > 0:
        print("\nFailed abc files:")
        for failed_abc in failed_abc_list:
            print(f"  - {failed_abc}")


if __name__ == "__main__":
    main()
