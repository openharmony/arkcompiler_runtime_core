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
#
import argparse
import json
import multiprocessing
import os
import subprocess as spp
import tempfile
from collections import OrderedDict
from typing import List
import logging

NPROC = multiprocessing.cpu_count()
TMP_DIR = tempfile.gettempdir()

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

STRESS = os.path.join(SCRIPT_DIR, '..', '..', 'scripts', 'stress.sh')

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


def get_args():
    parser = argparse.ArgumentParser(description="Abckit stress test")
    parser.add_argument('--build-dir', type=str, required=True, help=f'Path to build dir')
    parser.add_argument('--update-fail-list', action='store_true', default=False, help=f'Update fail list')
    parser.add_argument("--ark-path", type=str, default=None, help=f'ARK runtime path')
    parser.add_argument("--repeats", type=int, default=None, help=f'VM test retry attempts')
    return parser.parse_args()


def collect_from(rd, func) -> List[str]:
    tmp: List[str] = []
    for root, _, file_names in os.walk(rd):
        for file_name in file_names:
            tmp.append(os.path.join(root, file_name))
    return filter(func, tmp)


class ExecRes:
    def __init__(self, returncode, stdout, stderr):
        self.returncode = returncode
        self.stdout = stdout
        self.stderr = stderr


def stress_exec(cmd, cwd=os.getcwd(), allow_error=False, timeout=600, print_output=False, print_command=True, env=None,
         repeats: int = 1):
    if print_command:
        logging.debug('$ {0}> %s', cwd, " ".join(cmd))

    ct = timeout
    for loop in range(repeats):
        return_code, stdout, stderr = __exec_impl(cmd, cwd=cwd, timeout=ct, print_output=print_output, env=env)
        if return_code != -1:
            break
        ct = ct + 60

    if return_code != 0 and not allow_error:
        raise Exception(f"Error: Non-zero return code\nstdout: {stdout}\nstderr: {stderr}")
    return ExecRes(return_code, stdout, stderr)


def __exec_impl(cmd, cwd=os.getcwd(), timeout=600, print_output=False, env=None):
    proc = spp.Popen(cmd, cwd=cwd, stdout=spp.PIPE, stderr=spp.STDOUT, encoding='ISO-8859-1', env=env)
    while True and print_output:
        line = proc.stdout.readline()
        logging.debug(line.strip())
        if not line:
            break
    try:
        stdout, stderr = proc.communicate(timeout=timeout)
        return_code = proc.wait()
    except spp.TimeoutExpired:
        stdout, stderr = "timeout", "timeout"
        return_code = -1

    return return_code, stdout, stderr


def parse_stdout(error: str, stdout):
    if stdout is not None and 'ASSERTION FAILED:' in stdout:
        for line in stdout.split('\n'):
            if 'ASSERTION FAILED:' in line:
                error = line
    if stdout is not None and 'ERROR: plugin returned non-zero' in stdout:
        for line in stdout.split('\n'):
            if 'failed!' in line:
                error = line.split(' ')[-2] + " " + line.split(' ')[-1]
    return error


def get_fail_list(result):
    fail_list = OrderedDict()
    for src_path in result:
        if result[src_path]['error'] != '0':
            fail_list[src_path] = result[src_path]['error']
    return fail_list


def check_regression_errors(kfl_path: str, fail_list):
    with open(kfl_path, 'r') as f:
        old_fail_list = json.load(f)
    regression_errors = []
    for fail in fail_list:
        if fail not in old_fail_list:
            regression_errors.append(f'REGRESSION ERROR: new fail "{fail}"')
    if regression_errors:
        logging.debug('\n'.join(regression_errors))
        return False
    return True


def get_options_list(options_path: str):
    options_list = OrderedDict()
    with open(options_path, 'r') as f:
        options_list = json.load(f)
    return options_list


def update_fail_list(kfl_path: str, fail_list):
    with os.fdopen(os.open(kfl_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o755), 'w') as f:
        json.dump(fail_list, f, indent=2, sort_keys=True)


def check_fail_list(kfl_path: str, fail_list):
    with open(kfl_path, 'r') as f:
        old_fail_list = json.load(f)

    errors = []
    for old_fail in old_fail_list:
        if old_fail not in fail_list:
            errors.append(f'ERROR: no file in new fail list "{old_fail}"')
    if errors:
        logging.debug('\n'.join(errors))
        logging.debug('Please update fail list, rerun this script with "--update-fail-list"')
        return False
    return True
