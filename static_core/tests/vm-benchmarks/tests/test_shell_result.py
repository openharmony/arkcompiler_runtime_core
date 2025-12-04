#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Huawei Device Co., Ltd.
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

import logging

import pytest
from vmb.shell import ShellResult

@pytest.mark.parametrize('ret, out, err, log', [
    (0, 'hello', '', [[logging.DEBUG, 'hello']]),
    (0, 'Hello\nBye', '', [[logging.DEBUG, 'Hello\nBye']]),
    (0, '', 'hello', []),
    (0, '', 'Hello\nBye', []),
    (0, 'hello', 'Bye', [[logging.DEBUG, 'hello']]),
    (0, 'Hello\nBye', 'Hola\nAdios', [[logging.DEBUG, 'Hello\nBye']]),
    (1, 'hello', '', [[logging.ERROR, 'hello']]),
    (1, 'Hello\nBye', '', [[logging.ERROR, 'Hello\nBye']]),
    (1, '', 'hello', [[logging.ERROR, 'hello']]),
    (1, '', 'Hello\nBye', [[logging.ERROR, 'Hello'], [logging.ERROR, 'Bye']]),
    (1, 'hello', 'Bye', [[logging.ERROR, 'hello'], [logging.ERROR, 'Bye']]),
    (1, 'Hello\nBye', 'Hola\nAdios', [[logging.ERROR, 'Hello\nBye'], [logging.ERROR, 'Hola'], [logging.ERROR, 'Adios']])
])
def test_log_output(caplog, ret, out, err, log):
    caplog.set_level(logging.DEBUG)
    shell_result = ShellResult(ret, out, err)

    shell_result.log_output()
    records = caplog.records

    assert len(records) == len(log)

    for i in range(len(records)):
        assert records[i].levelno == log[i][0]
        assert records[i].message == log[i][1]


@pytest.mark.parametrize('line, log_level, message', [
    ('hello', logging.DEBUG, 'hello'),
    ('info: hello', logging.INFO, 'hello'),
    ('warning:hello', logging.WARNING, 'hello'),
    ('error: hello ', logging.ERROR, 'hello'),
    ('INFO:hello  ', logging.INFO, 'hello'),
    ('WARNING:   hello', logging.WARNING, 'hello'),
    ('ERROR: hello  ', logging.ERROR, 'hello')
])
def test_prepare_log_level_message(caplog, line, log_level, message):
    caplog.set_level(logging.DEBUG)
    shell_result = ShellResult(0, '', '')

    level, text = shell_result.prepare_log_level_message(line)

    assert level == log_level
    assert text == message


@pytest.mark.parametrize('out, err, log', [
    ('hello', '', [[logging.DEBUG, 'hello']]),
    ('info: hello', '', [[logging.INFO, 'hello']]),
    ('Warning:hello', '', [[logging.WARNING, 'hello']]),
    ('ERROR: hello ', '', [[logging.ERROR, 'hello']]),
    ('', 'hello', [[logging.DEBUG, 'hello']]),
    ('', 'info: hello', [[logging.INFO, 'hello']]),
    ('', 'Warning:hello  ', [[logging.WARNING, 'hello']]),
    ('', 'ERROR:   hello', [[logging.ERROR, 'hello']]),
    ('INFO: Hello\nWarning: Bye', '', [[logging.INFO, 'Hello'], [logging.WARNING, 'Bye']]),
    ('Error: hello', 'info: Bye', [[logging.ERROR, 'hello'], [logging.INFO, 'Bye']]),
    ('Warning: Hello\nError:Bye', 'error:Hola\ninfo: Adios',
        [[logging.WARNING, 'Hello'], [logging.ERROR, 'Bye'],
            [logging.ERROR, 'Hola'], [logging.INFO, 'Adios']])
])
def test_log_output_with_levels(caplog, out, err, log):
    caplog.set_level(logging.DEBUG)
    shell_result = ShellResult(0, out, err)

    shell_result.log_output_with_levels()
    records = caplog.records

    assert len(records) == len(log)

    for i in range(len(records)):
        assert records[i].levelno == log[i][0]
        assert records[i].message == log[i][1]


@pytest.mark.parametrize('out, err, message, log', [
    ('hello', '', 'Unknown error', [[logging.DEBUG, 'hello']]),
    ('', 'hello', 'Unknown error', [[logging.DEBUG, 'hello']]),
    ('info:hello', '', 'Unknown error', [[logging.INFO, 'hello']]),
    ('', 'info:hello', 'Unknown error', [[logging.INFO, 'hello']]),
    ('warning:hello', '', 'Unknown error', [[logging.WARNING, 'hello']]),
    ('', 'warning:hello', 'Unknown error', [[logging.WARNING, 'hello']]),
    ('error:hello', '', 'hello', [[logging.ERROR, 'hello']]),
    ('', 'error:hello', 'hello', [[logging.ERROR, 'hello']]),
    ('hello', 'info:', 'Unknown error', [[logging.DEBUG, 'hello']]),
    ('warning:\n', 'hello', 'Unknown error', [[logging.DEBUG, 'hello']]),
    ('info:hello', 'error:', 'Unknown error', [[logging.INFO, 'hello']]),
    ('error:\n\n', 'warning:hello', 'Unknown error', [[logging.WARNING, 'hello']]),
    ('warning:hello', '', 'Unknown error', [[logging.WARNING, 'hello']]),
    ('error:Hello\nBye', 'Warning: Hola\nAdios', 'Hello',
        [[logging.ERROR, 'Hello'], [logging.DEBUG, 'Bye'], [logging.WARNING, 'Hola'], [logging.DEBUG, 'Adios']]),
    ('Hello\nerror:Bye', 'Hola\ninfo:Adios', 'Bye',
        [[logging.DEBUG, 'Hello'], [logging.ERROR, 'Bye'], [logging.DEBUG, 'Hola'], [logging.INFO, 'Adios']]),
    ('WARNING:Hello\nBye', 'error:Hola\ninfo:Adios', 'Hola',
        [[logging.WARNING, 'Hello'], [logging.DEBUG, 'Bye'], [logging.ERROR, 'Hola'], [logging.INFO, 'Adios']]),
    ('error:Hello\nBye', 'Hola\nerror:Adios', 'Adios',
        [[logging.ERROR, 'Hello'], [logging.DEBUG, 'Bye'], [logging.DEBUG, 'Hola'], [logging.ERROR, 'Adios']]),
    ('info: Hello\nWARNING: Bye', 'error:Hola\nAdios', 'Hola',
        [[logging.INFO, 'Hello'], [logging.WARNING, 'Bye'], [logging.ERROR, 'Hola'], [logging.DEBUG, 'Adios']]),
    ('Hello\nBye', 'error:Hola\nerror:Adios', 'Adios',
        [[logging.DEBUG, 'Hello'], [logging.DEBUG, 'Bye'], [logging.ERROR, 'Hola'], [logging.ERROR, 'Adios']]),
    ('', '', 'Unknown error', []),
    ('Hello\n', 'error:Hola\nerror:', 'Hola', [[logging.DEBUG, 'Hello'], [logging.ERROR, 'Hola']])
])
def test_log_output_with_levels_exception(caplog, out, err, message, log):
    caplog.set_level(logging.DEBUG)
    shell_result = ShellResult(1, out, err)

    # Catch exception
    with pytest.raises(RuntimeError) as exception:
        shell_result.log_output_with_levels()

    assert message in str(exception.value)

    records = caplog.records
    assert len(records) == len(log)

    for i in range(len(records)):
        assert records[i].levelno == log[i][0]
        assert records[i].message == log[i][1]
