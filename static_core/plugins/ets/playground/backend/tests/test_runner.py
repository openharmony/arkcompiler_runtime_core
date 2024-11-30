#!/usr/bin/env python
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


import pytest
from src.arkts_playground.deps.runner import Runner
from tests.fixtures.mock_subprocess import FakeCommand, MockAsyncSubprocess


@pytest.fixture(scope="session", name="RunnerSample")
def runner_fixture(ark_build):
    return Runner(ark_build[0])


@pytest.mark.asyncio
@pytest.mark.parametrize("compile_opts, disasm", [
    (
            [],
            False
    ),
    (
            [],
            True
    ),
    (
            ["--opt-level=2"],
            True
    )
])
async def test_compile(monkeypatch, ark_build, compile_opts, disasm):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=sts", *compile_opts],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stdout=b"disassembly",
                        return_code=0)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_arkts("code", compile_opts, disasm)
    expected = {"compile": {"error": "", "exit_code": 0, "output": "executed"}, "disassembly": None}
    if disasm:
        expected["disassembly"] = {
            "code": "disassembly output",
            "error": "",
            "exit_code": 0,
            "output": "disassembly"
        }
    assert res == expected


@pytest.mark.asyncio
@pytest.mark.parametrize("compile_opts, disasm", [
    (
            [],
            False
    ),
    (
            [],
            True
    ),
    (
            ["--opt-level=2"],
            True
    )
])
async def test_compile_and_run(monkeypatch, ark_build, compile_opts: list, disasm):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=sts", *compile_opts],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stdout=b"run and compile",
                        return_code=0),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stdout=b"disassembly",
                        return_code=0)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", compile_opts, disasm)
    expected = {
        "compile": {
            "error": "",
            "exit_code": 0,
            "output": "executed"
        },
        "disassembly": None,
        "run": {
            "error": "", "exit_code": 0, "output": "run and compile"
        }
    }
    if disasm:
        expected["disassembly"] = {
            "code": "disassembly output",
            "error": "",
            "exit_code": 0,
            "output": "disassembly"
        }
    assert res == expected


@pytest.mark.asyncio
async def test_run_when_compile_failed(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=sts"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=-1)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False)
    expected = {
        "compile": {
            "error": "",
            "exit_code": -1,
            "output": "compile error"
        },
        "disassembly": None,
        "run": None
    }
    assert res == expected


@pytest.mark.asyncio
async def test_run_when_compile_segfault(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=sts"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=-11)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False)
    expected = {
        "compile": {
            "error": "compilation: Segmentation fault",
            "exit_code": -11,
            "output": "compile error"
        },
        "disassembly": None,
        "run": None
    }
    assert res == expected


async def test_run_disasm_failed(monkeypatch, ark_build, ):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=sts"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets", "ETSGLOBAL::main"],
                        stderr=b"runtime error",
                        return_code=1),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stderr=b"disassembly failed",
                        return_code=-2)
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], True)
    expected = {
        "compile": {
            "error": "",
            "exit_code": 0,
            "output": "executed"
        },
        "run": {
            "error": "runtime error",
            "exit_code": 1,
            "output": ""
        },
        "disassembly": {
            "code": None,
            "exit_code": -2,
            "error": "disassembly failed",
            "output": ""
        }
    }
    assert res == expected


async def test_compile_failed_with_disasm(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=sts"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_arkts("code", [], True)
    expected = {
        "compile": {
            "error": "",
            "exit_code": 1,
            "output": "compile error"
        },
        "disassembly": None
    }
    assert res == expected


async def test_get_versions(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--version"],
                        expected=str(ark_build[3]),
                        stdout=b"arkts ver",
                        return_code=1),
            FakeCommand(opts=["--version"],
                        expected=str(ark_build[1]),
                        stderr=b"es2panda ver",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())
    monkeypatch.setattr("src.arkts_playground.deps.runner.version", lambda _: "1.2.3")

    r = Runner(ark_build[0])
    res = await r.get_versions()
    expected = "1.2.3", "arkts ver", "es2panda ver"
    assert res == expected
