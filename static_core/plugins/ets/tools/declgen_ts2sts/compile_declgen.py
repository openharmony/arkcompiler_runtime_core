#!/usr/bin/env python
# coding: utf-8
# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import subprocess
import shutil
import tarfile
import os
import shutil


def extract(package_path, dest_path, package_name):
    try:
        with tarfile.open(package_path, 'r:gz') as tar:
            tar.extractall(path=dest_path)
    except tarfile.TarError as e:
        print(f'Error extracting files: {e}')
    dest_package_path = os.path.join(dest_path, package_name)
    if (os.path.exists(dest_package_path)):
        shutil.rmtree(dest_package_path)
    os.rename(os.path.join(dest_path, 'package'), dest_package_path)


def copy_dir(source_path, dest_path):
    try:
        run_cmd(["rm", "-rf", dest_path])
        shutil.copytree(source_path, dest_path, dirs_exist_ok=True, symlinks=True)
    except Exception as err:
        raise Exception(err.decode())


def run_cmd(cmd, execution_ath=None):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                           stdin=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           cwd=execution_ath)
    stdout, stderr = proc.communicate(timeout=300)
    if proc.returncode != 0:
        raise Exception(stderr.decode())


def run_build(execution_path):
    cmd = ["npm", "run", "build"]
    run_cmd(cmd, execution_path)


def run_pack(execution_path):
    cmd = ["npm", "pack"]
    run_cmd(cmd, execution_path)


# Note:
# The original script built and packed directly inside dest_and_exec_path.
# However, the ohos_copy GN action also writes temporary files into the same
# directory. If we run `rm -rf dest_and_exec_path`, it can cause race conditions
# with ohos_copy writing files, leading to build failures or missing files.
#
# Solution:
# Create a build_tmp subdirectory inside dest_and_exec_path, copy the source
# files into it, and perform the build/pack steps there. When cleaning, only
# remove build_tmp to avoid touching ohos_copy’s temporary files. Finally,
# move the generated panda-declgen-1.0.0.tgz back to dest_and_exec_path so the
# GN outputs path remains unchanged.
def main(args):
    source_path = args[0]
    dest_and_exec_path = args[1]
    tsc_package_path = args[2]
    tgz_name = args[3]
    tmp_build_dir = os.path.join(dest_and_exec_path, "build_tmp")

    node_modules_path = os.path.join(source_path, "node_modules")
    extract(tsc_package_path, node_modules_path, "typescript")
    copy_dir(source_path, tmp_build_dir)
    run_build(tmp_build_dir)
    run_pack(tmp_build_dir)

    src_file = os.path.join(tmp_build_dir, tgz_name)
    dst_file = os.path.join(dest_and_exec_path, tgz_name)
    run_cmd(["cp", src_file, dst_file])



if __name__ == '__main__':
    main(sys.argv[1:])
