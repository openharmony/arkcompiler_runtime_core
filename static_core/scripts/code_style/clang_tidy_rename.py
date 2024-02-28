#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

#!/usr/bin/env python3

import argparse
import json
import multiprocessing
import os
import shutil
import sys
import subprocess
import time
import re
import tempfile


def get_args():
    parser = argparse.ArgumentParser(
        description="Runner for clang-tidy for panda project.")
    parser.add_argument(
        'panda_dir', help='panda sources directory.')
    parser.add_argument(
        'build_dir', help='panda build directory.')
    parser.add_argument(
        'fix_dir', help='directory with fixes.')
    parser.add_argument(
        'clang_rules_autofix', help='one or several rules for applying fixies')
    parser.add_argument(
        '--filename-filter', type=str, action='store', dest='filename_filter',
        required=False, default="*",
        help='Regexp for filename with path to it. If missed all source files will be checked.')

    return parser.parse_args()


def run_clang_tidy(src_path, panda_dir, build_dir, fix_dir, compile_args, msg, clang_rules_autofix):
    fname_prefix = os.path.basename(src_path) + '_'
    fname_fix_patch = tempfile.mkstemp(
        suffix='.yaml', prefix=fname_prefix, dir=fix_dir)[1]

    cmd = ['clang-tidy-14']
    cmd += ['--header-filter=.*']
    cmd += ['-checks=-*,'+ clang_rules_autofix]
    cmd += ['--format-style=file --fix-errors --fix-notes']
    cmd += ['--config-file=' + os.path.join(panda_dir, '.clang-tidy')]
    cmd += ['--export-fixes=' + fname_fix_patch + '']
    cmd += [src_path]
    cmd += ['--']
    cmd += ['-ferror-limit=0']
    cmd += compile_args.split(' ')

    print(msg)

    try:
        subprocess.check_output(cmd, cwd=build_dir, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        # Skip error for some invalid release configurations.
        if e.stdout:
            print('Failed: ', ' '.join(cmd))
            print(e.stdout.decode())
        else:
            print("Note: missed output for ", src_path)

        if e.stderr:
            print(e.stderr.decode())

    if os.path.getsize(fname_fix_patch) == 0:
        os.remove(fname_fix_patch)


def check_file_list(file_list, panda_dir, build_dir, fix_dir, clang_rules_autofix):
    pool = multiprocessing.Pool(multiprocessing.cpu_count())
    jobs = []
    total_count = str(len(file_list))
    idx = 0
    for src, file_args in file_list:
        idx += 1

        msg = "[%s/%s] Running clang-tidy-rename: %s" % (
            str(idx), total_count, src)
        proc = pool.apply_async(func=run_clang_tidy, args=(
            src, panda_dir, build_dir, fix_dir, file_args, msg, clang_rules_autofix))
        jobs.append(proc)

    # Wait for jobs to complete before exiting
    while(not all([p.ready() for p in jobs])):
        time.sleep(5)

    # Safely terminate the pool
    pool.close()
    pool.join()


def need_to_ignore_file(file_path):
    src_exts = (".c", '.cc', ".cp", ".cxx", ".cpp", ".CPP", ".c++", ".C")
    if not file_path.endswith(src_exts):
        return True

    skip_dirs = ['third_party', 'unix', 'windows']
    for skip_dir in skip_dirs:
        if skip_dir in file_path:
            return True
        
    return False


def get_file_list(panda_dir, build_dir, filename_filter):
    json_cmds_path = os.path.join(build_dir, 'compile_commands.json')
    cmds_json = []
    with open(json_cmds_path, 'r') as f:
        cmds_json = json.load(f)

    if not cmds_json:
        return []

    regexp = None
    if filename_filter != '*':
        regexp = re.compile(filename_filter)

    file_list = []
    for cmd in cmds_json:
        file_path = str(os.path.realpath(cmd["file"]))
        if need_to_ignore_file(file_path):
            continue

        if file_path.startswith(build_dir):
            continue

        if regexp is not None and not regexp.search(file_path):
            continue

        compile_args = cmd["command"]
        args_pos = compile_args.find(' ')
        compile_args = compile_args[args_pos:]
        compile_args = compile_args.replace('\\', '')
        file_list.append((file_path, compile_args))

    file_list.sort(key=lambda tup: tup[0])
    return file_list


def apply_fixies(fix_dir, panda_dir):
    print("Running apply fixies ... ")

    # clang-apply-replacements-9 or clang-apply-replacements-14
    cmd = ['clang-apply-replacements-14']
    cmd += ['--style=file']
    cmd += ['--style-config=' + panda_dir]
    cmd += [fix_dir]

    try:
        subprocess.check_output(cmd, cwd=fix_dir, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print('Failed: ', ' '.join(cmd))
        if e.stdout:
            print(e.stdout.decode())
        if e.stderr:
            print(e.stderr.decode())
        return False

    return True


if __name__ == "__main__":
    args = get_args()
    main_file_list = []

    args.build_dir = str(os.path.abspath(args.build_dir))
    args.panda_dir = str(os.path.abspath(args.panda_dir))
    args.fix_dir = str(os.path.abspath(args.fix_dir))

    args.fix_dir = tempfile.mkdtemp(prefix='renamer_fixies_', dir=args.fix_dir)

    if not os.path.exists(os.path.join(args.build_dir, 'compile_commands.json')):
        shutil.rmtree(args.fix_dir)
        sys.exit("Error: Missing file `compile_commands.json` in build directory")

    main_file_list = get_file_list(
        args.panda_dir, args.build_dir, args.filename_filter)

    if not main_file_list:
        shutil.rmtree(args.fix_dir)
        sys.exit("Can't be prepaired source list. Please check availble in build `dir compile_commands.json`"
                 " and correcting of parameter `--filename-filter` if you use it.")

    check_file_list(main_file_list, args.panda_dir, args.build_dir, args.fix_dir, args.clang_rules_autofix)

    res = apply_fixies(args.fix_dir, args.panda_dir)

    if res:
        print("Clang-tidy renamer was applyed successfully! Please review changes")
    else:
        print("Clang-tidy renamer has errors, please check it")
    shutil.rmtree(args.fix_dir)
