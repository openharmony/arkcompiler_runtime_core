#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

from unittest import TestCase
from vmb.cli import Args, Command
from vmb.tool import OptFlags, ToolBase
from vmb.unit import BenchUnit
from unittest.mock import patch


def cmdline(line):
    return patch.object(sys, 'argv', ['vmb'] + line.split())


def test_run_command():
    test = TestCase()
    with cmdline('gen --langs blah foo/bar'):
        args = Args()
        test.assertTrue(args.command == Command.GEN)
        test.assertTrue(args.langs == {'blah'})
        test.assertTrue(args.get('platform') is None)
    with cmdline('all -L ,,this,that -l blah,,foo -p fake foo/bar'):
        args = Args()
        test.assertTrue(args.command == Command.ALL)
        test.assertTrue(args.langs == {'blah', 'foo'})
        test.assertTrue(args.src_langs == {'.this', '.that'})
        test.assertTrue(args.tests == set())
        test.assertTrue(args.tags == set())
        test.assertTrue(args.get('platform') == 'fake')


def test_wrong_opts():
    with cmdline('blah blah'):
        with pytest.raises(SystemExit):
            Args()


def test_optfalgs():
    with cmdline('all --lang=blah --platform=xxx '
                 '--mode=jit --mode=int foo/bar'):
        args = Args()
        flags = args.get_opts_flags()
        test = TestCase()
        test.assertTrue(OptFlags.INT in flags)
        test.assertTrue(OptFlags.JIT not in flags)
        test.assertTrue(OptFlags.AOT not in flags)
        test.assertTrue(OptFlags.GC_STATS not in flags)


def test_custom_opts():
    class FakeTool(ToolBase):
        def __init__(self, **a):
            super().__init__(**a)

        @property
        def name(self) -> str:
            return ''

        def exec(self, bu: BenchUnit) -> None:
            pass

    with (cmdline('all --lang=blah --platform=xxx '
                  '--node-custom-option="--a=b" '
                  '--other-custom-option=--a=A '
                  "--node-custom-option='--c=d' "
                  "--node-custom-option=--e=f "
                  '--node-custom-option=-s=short '
                  'foo/bar')):
        args = Args()
        opts = args.custom_opts.get('node', [])
        tool = FakeTool(custom_opts=opts)
        expected = {'a': 'b', 'c': 'd', 'e': 'f', 's': 'short'}
        actual = tool.custom_opts_obj()
        test = TestCase()
        test.assertDictEqual(expected, actual)
        test.assertEqual('''"--a=b" '--c=d' --e=f -s=short''', tool.custom)
