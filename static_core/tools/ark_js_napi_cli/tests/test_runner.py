#!/usr/bin/env python3
#coding: utf-8

# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import logging
import os
import shutil
import subprocess
import sys

logger = logging.getLogger(__name__)


def parse_args():
    """parse arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--tests-list', help='path to the tests list file')
    parser.add_argument('--build-dir', help='path to build dir')
    parser.add_argument('--tests-dir', help='path to tests dir')
    return parser.parse_args()


def check_command_result(command, change_dir=None):
    try:
        if change_dir:
            subprocess.check_output(command.split(), stderr=subprocess.STDOUT, cwd=change_dir)
        else:
            subprocess.check_output(command.split(), stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        if not e.stdout:
            log_msg = f'Note: missed output for {command}'
            logger.warning(log_msg)
            return True
        log_msg = f'Failed: {command}'
        logger.warning(log_msg)
        logger.warning(e.stdout.decode())
        if e.stderr:
            logger.warning(e.stderr.decode())
        return False
    return True


def prepare_test_files(arguments):
    build_root = arguments.build_dir
    test_space = os.path.join(build_root, "hybrid_test")
    try:
        os.makedirs(f'{test_space}/bin', exist_ok=True)
        os.makedirs(f'{test_space}/lib', exist_ok=True)
        os.makedirs(f'{test_space}/module', exist_ok=True)
        # Copy binaries
        shutil.copy(f'{build_root}/arkcompiler/ets_frontend/es2abc', f'{test_space}/bin')
        shutil.copy(f'{build_root}/arkcompiler/ets_frontend/es2panda', f'{test_space}/bin')
        shutil.copy(f'{build_root}/arkcompiler/runtime_core/ark_js_napi_cli', f'{test_space}/bin')
        shutil.copy(f'{build_root}/gen/arkcompiler/ets_runtime/stub.an', f'{test_space}/lib')
        # Copy STS specific files
        shutil.copy(f'{build_root}/arkcompiler/runtime_core/libets_interop_js_napi.so', f'{test_space}/module')
        shutil.copy(f'{build_root}/gen/arkcompiler/runtime_core/static_core/plugins/ets/etsstdlib.abc', f'{test_space}')
        shutil.copy(f'{build_root}/arkcompiler/ets_frontend/arktsconfig.json', f'{test_space}')
    except OSError as error:
        logger.warning(error)
        return False
    
    # Compile necessary .ts files
    ts_gc_common_name = "gc_test_common"
    ts_gc_common_path = ts_gc_common_name + ".ts"
    compile_cmd = f'{test_space}/bin/es2abc --merge-abc --extension=ts \
                  --module {arguments.tests_dir}/{ts_gc_common_path} \
                  --output {test_space}/{ts_gc_common_name}_ts.abc'
    return check_command_result(compile_cmd)


def judge_output(arguments):
    if not prepare_test_files(arguments):
        return False
    try:
        with open(arguments.tests_list, 'r') as f:
            lines = f.read().splitlines()
    except OSError:
        log_msg = f'Could not open/read tests file: {arguments.tests_list}'
        logger.warning(log_msg)
        return False
    result = True
    build_root = arguments.build_dir
    test_space = os.path.join(build_root, "hybrid_test")
    for line in lines:
        test_names = line.strip()
        if test_names and test_names[0] != '#':
            file_names = test_names.split(',')
            assert len(file_names) == 2, "Expected 2 files: .ts and .sts"
            ts_test_name, sts_test_name = file_names[0].strip()[:-3], file_names[1].strip()[:-4]
            # Compile test files
            compile_cmd_1 = f'{test_space}/bin/es2panda --extension=sts --opt-level=2 \
                                 --arktsconfig={test_space}/arktsconfig.json \
                                 --output={test_space}/{sts_test_name}.abc \
                                 {arguments.tests_dir}/sts/{sts_test_name}.sts'
            compile_cmd_2 = f'{test_space}/bin/es2abc --merge-abc --extension=ts \
                              --module {arguments.tests_dir}/{ts_test_name}.ts \
                              --output {test_space}/{ts_test_name}.abc'
            if not check_command_result(compile_cmd_1) or not check_command_result(compile_cmd_2):
                result = False
                continue
            # Run test
            libs_path = f'{build_root}/arkcompiler/runtime_core/'
            libs_path += f':{build_root}/arkcompiler/ets_runtime/'
            libs_path += f':{build_root}/arkui/napi/'
            libs_path += f':{build_root}/thirdparty/icu/'
            libs_path += f':{build_root}/thirdparty/libuv/'
            libs_path += f':{build_root}/thirdparty/bounds_checking_function/'
            os.environ['LD_LIBRARY_PATH'] = libs_path
            check_cmd = f'{test_space}/bin/ark_js_napi_cli --stub-file \
                          {test_space}/lib/stub.an --entry-point \
                          {ts_test_name} {test_space}/{ts_test_name}.abc'
            result &= check_command_result(check_cmd, f'{test_space}')
    return result

if __name__ == '__main__':
    input_args = parse_args()
    if not judge_output(input_args):
        sys.exit("Failed: hybrid tests runner got errors")
    logger.warning("Hybrid tests runner was passed successfully!")
