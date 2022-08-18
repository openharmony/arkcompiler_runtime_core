#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2022 Huawei Device Co., Ltd.
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
import argparse
import subprocess


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--testcase-list', help='the file path of testcase list', required=True)
    parser.add_argument(
        '--build-dir', help='the build dir of ts2abc', required=True)
    parser.add_argument(
        '--dst-dir', help='the output dst path', required=True)
    return parser.parse_args()


def create_dir(out_dir, mode=0o775):
    if os.path.isabs(out_dir) and not os.path.exists(out_dir):
        os.makedirs(out_dir, mode)


def run_command(cmd):
    cmd = ' '.join(cmd)
    print(cmd)
    subprocess.Popen(cmd, shell=True)


def get_command(s, ts2js, tc_dir, dst_dir):
    testcase, m_flag = s.split(' ')
    tc_name = os.path.basename(testcase)
    tc_name = tc_name[0: tc_name.rfind('.')]
    cmd = ['node', '--expose-gc', ts2js, '--opt-level=0']
    if m_flag == '1':
        cmd.append('-m')
    out_dir = os.path.join(dst_dir, 'defectscanaux_tests',
                           testcase[0: testcase.rfind('/')])
    create_dir(out_dir)
    cmd.append('-o')
    cmd.append(os.path.join(out_dir, tc_name + '.abc'))
    tc_path = os.path.join(tc_dir, testcase)
    cmd.append(tc_path)
    return cmd


def generate_abc(args):
    ts2js = os.path.join(args.build_dir, 'src/index.js')
    tc_dir = os.path.dirname(args.testcase_list)
    with open(args.testcase_list) as f:
        for line in f.readlines():
            line = line.strip()
            if len(line) == 0:
                continue
            cmd = get_command(line, ts2js, tc_dir, args.dst_dir)
            run_command(cmd)


if __name__ == '__main__':
    input_args = parse_args()
    generate_abc(input_args)
