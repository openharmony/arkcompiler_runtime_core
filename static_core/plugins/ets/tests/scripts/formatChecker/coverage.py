#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

from math import fabs
from pathlib import Path
import argparse
from typing import Tuple

from utils.constants import TEMPLATE_EXTENSION
from utils.test_case import is_negative, strip_template
from utils.file_structure import TestDirectory, walk_test_subdirs, build_directory_tree
from utils.file_structure import print_tree, denormalize_section_name
from utils.spec import build_spec_tree

def count_tests(folder: TestDirectory):
    pos_tests = 0
    neg_tests = 0
    templates = set()

    for testfile in folder.iter_files(allowed_ext=[TEMPLATE_EXTENSION]):
        if is_negative(testfile):
            neg_tests += 1
        else: 
            pos_tests += 1
        template_name, _ = strip_template(testfile)
        templates.add(template_name)
    return pos_tests, neg_tests, len(templates)


def print_table_of_contents(testpath: Path):
    total_templates = 0
    total_positive_tests = 0
    total_negative_tests = 0

    print("Table of contents:\n")
    for dir in walk_test_subdirs(testpath):
        full_index = dir.full_index()
        depth = len(full_index)
        assert depth >= 1
        # Count 
        pos_tests, neg_tests, n_templates = count_tests(dir)
        # Save
        total_positive_tests += pos_tests
        total_negative_tests += neg_tests
        total_templates += n_templates
        # Print
        left_space = " " * 2 * depth
        section_index = ".".join([str(i) for i in full_index])
        section_name = denormalize_section_name(dir.name)
        right_space = 90 - len(left_space) - len(section_index) - len(section_name)
        if not dir.is_empty():
            print(left_space, section_index, section_name, "." * right_space,
                  f"templates: {n_templates}; tests: {pos_tests} pos, {neg_tests} neg.\n")
    
    print("\n=====")
    print(f"TOTAL TESTS {total_positive_tests + total_negative_tests} | "
          f"TOTAL TEMPLATES {total_templates} | "
          f"TOTAL POSITIVE TESTS {total_positive_tests} | "
          f"TOTAL NEGATIVE TESTS {total_negative_tests}")


def find_diffs(test: TestDirectory, ethalon: TestDirectory, strict: bool = False, silent: bool = False):
    diff_detected = False

    for esd in ethalon.subdirs:
        tsd = test.find_subdir_by_name(esd.name)

        esd_index = esd.full_index()
        esd_section_index = ".".join([str(i) for i in esd_index][1:])
        esd_section_name = denormalize_section_name(esd.name)

        if tsd is None:
            diff_detected = True
            if not strict:
                tsd = test.find_subdir_by_id(esd.id)
                if not tsd is None and not find_diffs(tsd, esd, strict=True, silent=True):
                    tsd_section_name = denormalize_section_name(tsd.name)
                    if not silent:
                        print(f"[Missmath]: Subsection's name missmath [{esd_section_index}]: "
                              f"'{esd_section_name}' (spec) != '{tsd_section_name}' (tests).")
                else:
                    if not silent:    
                        print(f"[NotFound]: Subsection [{esd_section_index}.{esd_section_name}] not fould in tests.")
            
            continue

        if tsd.id != esd.id:
            diff_detected = True

            tsd_index = tsd.full_index()
            tsd_section_index = ".".join([str(i) for i in tsd_index][1:])
            tsd_section_name = denormalize_section_name(tsd.name)

            if not strict:
                if not find_diffs(tsd, esd, strict=True, silent=True):
                    if not silent:    
                        print(f"[Missmath]: Index missmath for [{esd_section_name}]: "
                              f"{esd_section_index} (spec) != {tsd_section_index} (tests) but content is equal.")
                else:
                    if not silent:
                        print(f"[Missmath]: Subsection [{tsd_section_index}.{tsd_section_name}] from tests has"
                              f"a different index and content in compare with [{esd_section_index}.{esd_section_name}]"
                              f"from spec.")
            continue

        find_diffs(tsd, esd)

    return diff_detected


def calc_tests_and_spec_stat(test: TestDirectory, spec: TestDirectory, tnum: int = 0, snum: int = 0)-> Tuple[int, int]:
    for esd in spec.subdirs:
        snum += 1
        tsd = test.find_subdir_by_name(esd.name)

        if not tsd is None and tsd.id == esd.id:
            tnum += 1
            tnum, snum = calc_tests_and_spec_stat(tsd, esd, tnum, snum)   

    return tnum, snum
    

def coverage(testpath: Path, specpath: Path):
    ethalon_tree = TestDirectory(id=-1, name="root", path="", parent=None)
    build_spec_tree(specpath=specpath, root=ethalon_tree)

    test_tree = TestDirectory(id=-1, name="root", path=testpath, parent=None)
    build_directory_tree(test_tree)
    print(f"=====\nDIFF tests='{str(testpath)}'; spec='{str(specpath)}'")
    find_diffs(test_tree, ethalon_tree)

    testsNum, specNum = calc_tests_and_spec_stat(test_tree, ethalon_tree)
    coverage_p = int(float(testsNum) / float(specNum) * 100)
    print(f"=====\nCOVEARGE: {coverage_p}%")
    


parser = argparse.ArgumentParser(description='Run CTS tests.')
parser.add_argument('tests_dir', help='Path to directory that contains the tests.')
parser.add_argument('--specpath', help='Path to file with STS specification in YAML format.', required=False)


def main():
    args = parser.parse_args()

    testpath = Path(args.tests_dir)

    # Verify that CTS dir exists
    if not testpath.is_dir():
        print(f"ERROR: Path '{testpath}' must point to a directory")
        exit(1)
   
    print_table_of_contents(testpath)
    
    if not args.specpath is None:
        specpath = Path(args.specpath)

        if not specpath.is_file():
            print(f"ERROR: Path '{specpath}' must point to a file with STS spec")
            exit(1)

        coverage(testpath, specpath)


if __name__ == "__main__":
    main()
