#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

# flake8: noqa
# pylint: skip-file

import os
import pytest  # type: ignore
from unittest import TestCase
from vmb.shell import ShellUnix

here = os.path.realpath(os.path.dirname(__file__))
sh = ShellUnix()


def posix_only(f):
    def s(*args):
        if 'posix' != os.name:
            pytest.skip("POSIX-only test")
        else:
            f(*args)
    return s


@posix_only
def test_unix_ls() -> None:
    this_file = os.path.basename(__file__)
    res = sh.run(f'ls -1 {here}')
    TestCase().assertTrue(res.grep(this_file) == this_file)


@posix_only
def test_unix_cwd() -> None:
    this_file = os.path.basename(__file__)
    this_dir = os.path.dirname(__file__)
    res = sh.run(f'ls -1 .', cwd=this_dir)
    TestCase().assertTrue(res.grep(this_file) == this_file)


@posix_only
def test_unix_measure_time() -> None:
    res = sh.run('sleep 1', measure_time=True, timeout=3)
    test = TestCase()
    test.assertTrue(res.tm > 0.9)
    test.assertTrue(res.rss > 0)
