#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2025 Huawei Device Co., Ltd.
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
import sys
from jinja2 import Environment, FileSystemLoader

OUTPUT_DIR = sys.argv[1]
TEST_DIR = sys.argv[2]
ARM_32 = sys.argv[3]

MIN_NUM_ARGS = 1
MAX_NUM_ARGS = 11

types = ['boolean', 'byte', 'char', 'double',
         'float', 'int', 'long', 'short', 'string']


def test_to_file(output_ets, output_cpp, file_name):
    file_dir = f'{OUTPUT_DIR}/{file_name}'
    if not os.path.exists(file_dir):
        os.makedirs(file_dir)
    path_ets = f'{file_dir}/{file_name}.ets'
    fd_ets = os.open(path_ets, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    with os.fdopen(fd_ets, "w", encoding="utf-8") as f:
        f.write(output_ets)

    path_cpp = f'{file_dir}/{file_name}.cpp'
    fd_cpp = os.open(path_cpp, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
    with os.fdopen(fd_cpp, "w", encoding="utf-8") as f:
        f.write(output_cpp)


def main() -> None:
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    env = Environment(loader=FileSystemLoader(TEST_DIR))

    types_ets = env.get_template('types_ets.j2')
    types_cpp = env.get_template('types_lib.j2')

    for typee in types:
        for i in range(MIN_NUM_ARGS, MAX_NUM_ARGS + 1):
            types_ets_out = types_ets.render(
                Type=typee.capitalize(), type=typee, count=i, arm32=ARM_32)
            types_cpp_out = types_cpp.render(
                Type=typee.capitalize(), type=typee, count=i, arm32=ARM_32)
            file_name = f'types_{typee}_{i}'
            test_to_file(types_ets_out, types_cpp_out, file_name)

    arrays_ets = env.get_template('arrays_ets.j2')
    arrays_cpp = env.get_template('arrays_lib.j2')

    for typee in types:
        arrays_ets_out = arrays_ets.render(Type=typee.capitalize(), type=typee)
        arrays_cpp_out = arrays_cpp.render(Type=typee.capitalize(), type=typee)
        file_name = f'arrays_{typee}'
        test_to_file(arrays_ets_out, arrays_cpp_out, file_name)

    returns_ets = env.get_template('returns_ets.j2')
    returns_cpp = env.get_template('returns_lib.j2')

    for typee in types:
        returns_ets_out = returns_ets.render(
            Type=typee.capitalize(), type=typee, arm32=ARM_32)
        returns_cpp_out = returns_cpp.render(
            Type=typee.capitalize(), type=typee, arm32=ARM_32)
        file_name = f'returns_{typee}'
        test_to_file(returns_ets_out, returns_cpp_out, file_name)


if __name__ == "__main__":
    main()
