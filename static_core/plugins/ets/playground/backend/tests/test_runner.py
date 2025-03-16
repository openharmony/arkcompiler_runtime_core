#!/usr/bin/env python
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


import pytest
from src.arkts_playground.deps.runner import Runner
from tests.fixtures.mock_subprocess import FakeCommand, MockAsyncSubprocess


@pytest.fixture(scope="session", name="RunnerSample")
def runner_fixture(ark_build):
    return Runner(ark_build[0])


@pytest.mark.asyncio
@pytest.mark.parametrize("compile_opts, disasm, verifier", [
    (
            [],
            False,
            True
    ),
    (
            [],
            True,
            False
    ),
    (
            ["--opt-level=2"],
            True,
            False
    )
])
async def test_compile(monkeypatch, ark_build, compile_opts, disasm, verifier):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets", *compile_opts],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stdout=b"disassembly",
                        return_code=0),
            FakeCommand(expected=str(ark_build[5]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets"],
                        stdout=b"verified",
                        return_code=0)

        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_arkts("code", compile_opts, disasm, verifier)
    expected = {"compile": {"error": "", "exit_code": 0, "output": "executed"}, "disassembly": None, "verifier": None}
    if disasm:
        expected["disassembly"] = {
            "code": "disassembly output",
            "error": "",
            "exit_code": 0,
            "output": "disassembly"
        }
    if verifier:
        expected["verifier"] = {
            "output": "verified",
            "error": "",
            "exit_code": 0
        }
    assert res == expected


def _get_mocker_compile_and_run(ark_build, compile_opts: list) -> MockAsyncSubprocess:
    return MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets", *compile_opts],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}",
                              "--load-runtimes=ets",
                              f"--icu-data-path={ark_build[6]}",
                              "ETSGLOBAL::main"],
                        stdout=b"run and compile",
                        return_code=0),
            FakeCommand(expected=str(ark_build[2]),
                        opts=[],
                        stdout=b"disassembly",
                        return_code=0),
            FakeCommand(expected=str(ark_build[5]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets"],
                        stdout=b"verified",
                        return_code=0)
        ]
    )


@pytest.mark.asyncio
@pytest.mark.parametrize("compile_opts, disasm, verifier", [
    (
            [],
            False,
            False,
    ),
    (
            [],
            True,
            True
    ),
    (
            ["--opt-level=2"],
            True,
            False
    )
])
async def test_compile_and_run(monkeypatch, ark_build, compile_opts: list, disasm: bool, verifier: bool):
    mocker = _get_mocker_compile_and_run(ark_build, compile_opts)
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0], icu_data=ark_build[6])
    res = await r.compile_run_arkts("code", compile_opts, disasm, verifier=verifier)
    expected = {
        "compile": {
            "error": "",
            "exit_code": 0,
            "output": "executed"
        },
        "disassembly": None,
        "verifier": None,
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
    if verifier:
        expected["verifier"] = {
            "output": "verified",
            "error": "",
            "exit_code": 0
        }
    assert res == expected


@pytest.mark.asyncio
async def test_run_when_compile_failed(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
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
        "verifier": None,
        "run": None
    }
    assert res == expected


@pytest.mark.asyncio
async def test_run_when_compile_segfault(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
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
        "verifier": None,
        "run": None
    }
    assert res == expected


async def test_run_disasm_failed(monkeypatch, ark_build, ):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
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
        },
        "verifier": None
    }
    assert res == expected


async def test_compile_failed_with_disasm(monkeypatch, ark_build):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"compile error",
                        return_code=1),
        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_arkts("code", [], True, False)
    expected = {
        "compile": {
            "error": "",
            "exit_code": 1,
            "output": "compile error"
        },
        "disassembly": None,
        "verifier": None
    }
    assert res == expected


async def test_run_with_verify_on_the_fly(monkeypatch, ark_build, ):
    mocker = MockAsyncSubprocess(
        [
            FakeCommand(opts=["--extension=ets"],
                        expected=str(ark_build[1]),
                        stdout=b"executed",
                        return_code=0),
            FakeCommand(expected=str(ark_build[3]),
                        opts=[f"--boot-panda-files={ark_build[4]}", "--load-runtimes=ets",
                              "--verification-enabled=true", "--verification-mode=on-the-fly", "ETSGLOBAL::main"],
                        stderr=b"runtime verify error",
                        return_code=1),

        ]
    )
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_run_arkts("code", [], False, runtime_verify=True)
    expected = {
        "compile": {
            "error": "",
            "exit_code": 0,
            "output": "executed"
        },
        "run": {
            "error": "runtime verify error",
            "exit_code": 1,
            "output": ""
        },
        "disassembly": None,
        "verifier": None
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
