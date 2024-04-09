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
from subprocess import TimeoutExpired
from vmb.shell import ShellUnix, ShellAdb

here = os.path.realpath(os.path.dirname(__file__))
sh = ShellUnix()


def only_posix() -> None:
    if 'posix' != os.name:
        pytest.skip("Only POSIX")


def test_unix_ls() -> None:
    only_posix()
    this_file = os.path.basename(__file__)
    res = sh.run(f'ls -1 {here}')
    assert res.grep(this_file) == this_file


def test_unix_cwd() -> None:
    only_posix()
    this_file = os.path.basename(__file__)
    this_dir = os.path.dirname(__file__)
    res = sh.run(f'ls -1 .', cwd=this_dir)
    assert res.grep(this_file) == this_file


def test_unix_measure_time() -> None:
    only_posix()
    res = sh.run('sleep 1', measure_time=True, timeout=3)
    assert res.tm > 0.9
    assert res.rss > 0


def test_unix_timeout() -> None:
    only_posix()
    with pytest.raises(TimeoutExpired):
        sh.run('sleep 2', timeout=1)
