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

import os

import pytest
from structlog.testing import capture_logs
from structlog.typing import EventDict

from src.arkts_playground.deps.runner import Runner
from tests.fixtures.mock_subprocess import FakeCommand, MockAsyncSubprocess


@pytest.fixture(scope="session", name="RunnerSample")
def runner_fixture(ark_build):
    return Runner(ark_build[0])


def log_entry_assert(
    entry: EventDict,
    stage_expected: str | None = None,
    event_expected: str | None = None,
    level_expected: str | None = None,
):
    if stage_expected is not None:
        assert entry["stage"] == stage_expected
    if event_expected is not None:
        assert entry["event"] == event_expected
    if level_expected is not None:
        assert entry["log_level"] == level_expected


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
    with capture_logs() as cap_logs:
        res = await r.compile_arkts("code", compile_opts, disasm, verifier)

    log_entry_assert(cap_logs[0], event_expected="Compilation pipeline started", stage_expected="compile")
    log_entry_assert(cap_logs[1], event_expected="Compilation succeeded", stage_expected="compile")
    if disasm and verifier:
        log_entry_assert(cap_logs[2], event_expected="Disassemblying completed")
        log_entry_assert(cap_logs[3], event_expected="Verification completed")
    elif disasm:
        log_entry_assert(cap_logs[2], event_expected="Disassembling completed")
    elif verifier:
        log_entry_assert(cap_logs[2], event_expected="Verification completed")

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
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", compile_opts, disasm, verifier=verifier)

    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started", stage_expected="compile")
    log_entry_assert(cap_logs[1], event_expected="Program run completed", stage_expected="compile-run")
    if disasm and verifier:
        log_entry_assert(cap_logs[2], event_expected="Disassembling completed")
        log_entry_assert(cap_logs[3], event_expected="Verification completed")
    elif disasm:
        log_entry_assert(cap_logs[2], event_expected="Disassembling completed")
    elif verifier:
        log_entry_assert(cap_logs[2], event_expected="Verification completed")

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
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], False)
    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started")
    log_entry_assert(cap_logs[-1], event_expected="Compilation failed with nonzero exit code", level_expected="error")

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
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], False)
    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started")
    log_entry_assert(cap_logs[1], event_expected="Compilation failed: segmentation fault", level_expected="error")
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
    with capture_logs() as cap_logs:
        res = await r.compile_run_arkts("code", [], True)
    log_entry_assert(cap_logs[0], event_expected="Compile and run pipeline started")
    log_entry_assert(cap_logs[2], event_expected="Disassembling failed with nonzero exit code")

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
    with capture_logs() as cap_logs:
        res = await r.compile_arkts("code", [], True, False)
    log_entry_assert(cap_logs[0], event_expected="Compilation pipeline started")
    log_entry_assert(cap_logs[-1], event_expected="Compilation failed with nonzero exit code", level_expected="error")
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
                              "--verification-mode=ahead-of-time", "ETSGLOBAL::main"],
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


@pytest.mark.asyncio
async def test_dump_ast_ok(monkeypatch, ark_build, fake_saved_stsfile):
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),  # es2panda
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"AST TREE",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    ast_result = await r.dump_ast("code")
    assert ast_result == {
        "ast": "AST TREE",
        "output": "AST TREE",
        "error": "",
        "exit_code": 0,
    }


@pytest.mark.asyncio
async def test_dump_ast_with_opts(monkeypatch, ark_build, fake_saved_stsfile):
    compile_opts = ["--opt-level=2"]
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc", *compile_opts],
            stdout=b"AST",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    ast_result = await r.dump_ast("code", compile_opts)

    assert ast_result == {
        "ast": "AST",
        "output": "AST",
        "error": "",
        "exit_code": 0,
    }


@pytest.mark.asyncio
async def test_dump_ast_stdout_empty_not_use_stderr(monkeypatch, ark_build, fake_saved_stsfile):
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"",
            stderr=b"some log in stderr",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())
    r = Runner(ark_build[0])
    ast_result = await r.dump_ast("code")
    assert ast_result == {
        "ast": None,
        "output": "",
        "error": "some log in stderr",
        "exit_code": 0,
    }


@pytest.mark.asyncio
async def test_dump_ast_segfault(monkeypatch, ark_build, fake_saved_stsfile):
    subproc_mock = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),
            opts=["--dump-ast", "--extension=ets", f"--output={fake_saved_stsfile}.abc"],
            stdout=b"",
            stderr=b"oops",
            return_code=-11,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", subproc_mock.create_subprocess_exec())

    r = Runner(ark_build[0])
    with capture_logs() as cap_logs:
        ast_result = await r.dump_ast("code")
    log_entry_assert(cap_logs[-1], event_expected="AST dump failed: segmentation fault", level_expected="error")
    assert ast_result == {
        "ast": None,
        "output": "",
        "error": "oopsast: Segmentation fault",
        "exit_code": -11,
    }


@pytest.mark.asyncio
async def test_compile_utf8_decode_with_replace(monkeypatch, ark_build):
    """
    Ensure _execute_cmd decodes stdout/stderr with errors='replace'
    and does not crash on non-UTF8 bytes.
    """
    mocker = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[1]),         # es2panda
            opts=["--extension=ets"],
            stdout=b"\xf7\xfa non-utf8 \xa0",
            stderr=b"\xff\xfe error bytes",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    r = Runner(ark_build[0])
    res = await r.compile_arkts("code", [], disasm=False, verifier=False)

    assert res["compile"]["exit_code"] == 0
    # Replacement characters should be present instead of raising UnicodeDecodeError
    assert "\ufffd" in res["compile"]["output"]
    assert "\ufffd" in res["compile"]["error"]


@pytest.fixture(name="out_pa_file")
def out_file_with_cleanup():
    out_file = "out.pa"
    yield out_file
    if os.path.exists(out_file):
        os.remove(out_file)


@pytest.mark.asyncio
async def test_disassembly_opens_file_with_errors_replace(monkeypatch, ark_build, out_pa_file):
    """
    Ensure disassembly opens the .pa file with encoding='utf-8' and errors='replace'.
    """
    mocker = MockAsyncSubprocess([
        FakeCommand(
            expected=str(ark_build[2]),  # ark_disasm
            opts=[],
            stdout=b"",
            return_code=0,
        ),
    ])
    monkeypatch.setattr("asyncio.create_subprocess_exec", mocker.create_subprocess_exec())

    captured: dict[str, object] = {}

    class _DummyFile:
        async def read(self) -> str:
            return "X"

    class _DummyCtx:
        async def __aenter__(self):
            return _DummyFile()

        async def __aexit__(self, exc_type, exc, tb):
            return False

    def open_spy(path, mode="r", encoding=None, errors=None):
        captured["path"] = path
        captured["mode"] = mode
        captured["encoding"] = encoding
        captured["errors"] = errors
        return _DummyCtx()

    monkeypatch.setattr("src.arkts_playground.deps.runner.aiofiles.open", open_spy)

    r = Runner(ark_build[0])
    res = await r.disassembly("in.abc", out_pa_file)

    assert res["exit_code"] == 0
    assert res["code"] == "X"

    assert captured.get("encoding") == "utf-8"
    assert captured.get("errors") == "replace"
