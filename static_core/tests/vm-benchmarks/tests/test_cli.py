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

import sys
import pytest  # type: ignore

from vmb.cli import Args, Command
from vmb.tool import OptFlags
from unittest.mock import patch


def cmdline(line):
    return patch.object(sys, 'argv', ['vmb'] + line.split())


def test_run_command():
    with cmdline('gen --langs blah /foo/bar'):
        args = Args()
        assert args.command == Command.GEN
        assert args.langs == {'blah'}
        assert args.get('platform') is None
    with cmdline('all -L ,,this,that -l blah,,foo -p fake /foo/bar'):
        args = Args()
        assert args.command == Command.ALL
        assert args.langs == {'blah', 'foo'}
        assert args.src_langs == {'.this', '.that'}
        assert args.tests == set()
        assert args.tags == set()
        assert args.get('platform') == 'fake'


def test_wrong_opts():
    with cmdline('blah blah'):
        with pytest.raises(SystemExit):
            Args()


def test_optfalgs():
    with cmdline('all --lang=blah --platform=xxx '
                 '--mode=jit --mode=int /foo/bar'):
        args = Args()
        flags = args.get_opts_flags()
        assert OptFlags.INT in flags
        assert OptFlags.JIT not in flags
        assert OptFlags.AOT not in flags
        assert OptFlags.GC_STATS not in flags


def test_custom_opts():
    with cmdline('all --lang=blah --platform=xxx '
                 '--node-custom-option="--a=b" '
                 '--node-custom-option="--c=d" '
                 '/foo/bar'):
        args = Args()
        assert '"--a=b" "--c=d"' == \
            ' '.join(args.get('node_custom_option'))
