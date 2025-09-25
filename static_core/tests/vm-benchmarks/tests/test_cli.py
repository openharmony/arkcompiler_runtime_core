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
from unittest.mock import patch
from vmb.cli import Args, Command
from vmb.target import Target
from vmb.tool import OptFlags, ToolBase
from vmb.unit import BenchUnit


class FakeTool(ToolBase):
    def __init__(self, *a):
        super().__init__(*a)

    @property
    def name(self) -> str:
        return ''

    def exec(self, bu: BenchUnit) -> None:
        pass


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


def test_comma_separated_set():
    test = TestCase()
    with cmdline('all -p fake --hooks one,two --hooks zero,,three foo/bar'):
        args = Args()
        test.assertTrue(args.command == Command.ALL)
        test.assertTrue(args.hooks == {'one', 'two', 'zero', 'three'})


def test_wrong_opts():
    with cmdline('blah blah'):
        with pytest.raises(SystemExit):
            Args()


def test_optflags():
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
    test = TestCase()
    with (cmdline('all --lang=blah --platform=xxx '
                  '--node-custom-option="--a=b" '
                  '--other-custom-option=--a=A '
                  "--node-custom-option='--c=d' "
                  "--node-custom-option=--e=f "
                  '--node-custom-option=-s=short '
                  'foo/bar')):
        args = Args()
        tool = FakeTool(Target.HOST,
                        args.get_opts_flags(),
                        args.custom_opts.get('node', []))
        expected = {'a': 'b', 'c': 'd', 'e': 'f', 's': 'short'}
        actual = tool.custom_opts_obj()
        test.assertDictEqual(expected, actual)
        test.assertEqual('''"--a=b" '--c=d' --e=f -s=short''', tool.custom)


def test_custom_paths():
    test = TestCase()
    with (cmdline('all --lang=blah --platform=xxx '
                  '--node-custom-option="--a=b" '
                  '--node-path=/some/custom/path/to/node '
                  '--node-custom-option=--c=d '
                  'foo/bar')):
        args = Args()
        tool = FakeTool(Target.HOST,
                        args.get_opts_flags(),
                        args.custom_opts.get('node', []),
                        args.custom_paths.get('node', ''))
        expected = {'a': 'b', 'c': 'd'}
        actual = tool.custom_opts_obj()
        test.assertDictEqual(expected, actual)
        test.assertEqual('''"--a=b" --c=d''', tool.custom)
        test.assertEqual("/some/custom/path/to/node", tool.custom_path)
