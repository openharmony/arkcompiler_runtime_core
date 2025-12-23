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

import asyncio
from collections.abc import Sequence
from functools import lru_cache
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
from typing import NamedTuple, TypedDict, final

import aiofiles
import structlog
from typing_extensions import Unpack

from ..config import get_settings
from ..metrics import (AST_FAILURES, COMPILER_FAILURES, DISASM_FAILURES,
                       RUNTIME_FAILURES)

logger = structlog.stdlib.get_logger(__name__)


class Binary(NamedTuple):
    ark_bin: str
    es2panda: str
    disasm_bin: str
    verifier: str


class SubprocessExecResult(TypedDict):
    output: str
    error: str
    exit_code: int | None


class AstViewExecResult(TypedDict):
    output: str
    error: str
    exit_code: int | None
    ast: str | None


class DisasmResult(TypedDict):
    output: str
    error: str
    code: str | None
    exit_code: int | None


class CompileResult(TypedDict):
    compile: SubprocessExecResult
    disassembly: DisasmResult | None
    verifier: SubprocessExecResult | None


class CompileRunResult(TypedDict):
    compile: SubprocessExecResult
    disassembly: DisasmResult | None
    verifier: SubprocessExecResult | None
    run: SubprocessExecResult | None


class CompileRunParams(TypedDict, total=False):
    runtime_verify: bool
    verifier: bool


@final
class Runner:
    def __init__(self, build: str, timeout: int = 120, icu_data: str = ""):
        self._exec_timeout = timeout
        _build = Path(build).expanduser()
        bin_path = _build / "bin"
        self._version = {
            "ark": "",
            "playground": "",
            "es2panda": ""
        }
        self.binary = Binary(
            ark_bin=str(bin_path / "ark"),
            disasm_bin=str(bin_path / "ark_disasm"),
            es2panda=str(bin_path / "es2panda"),
            verifier=str(bin_path / "verifier")
        )
        self.stdlib_abc = str(_build / "plugins" / "ets" / "etsstdlib.abc")
        self._icu_data = icu_data if Path(icu_data).exists() else ""
        self._validate()

    @staticmethod
    def parse_compile_options(options: dict[str, str]) -> list[str]:
        res: list[str] = []
        if op := options.get("--opt-level"):
            res.append(f"--opt-level={op}")
        return res

    @staticmethod
    async def _save_code(tempdir: str, code: str) -> str:
        """Save code to temporary file in the temporary directory
        :param tempdir: 
        :param code: 
        :rtype: str
        """
        async with aiofiles.tempfile.NamedTemporaryFile(dir=tempdir,
                                                        mode="w",
                                                        suffix=".ets",
                                                        delete=False) as stsfile:
            await stsfile.write(code)
            return str(stsfile.name)

    async def get_versions(self) -> tuple[str, str, str]:
        """Get arkts_playground package and ark versions
        :rtype: Tuple[str, str]
        """
        if not self._version["ark"]:
            await self._get_arkts_version()
        if not self._version["playground"]:
            self._get_playground_version()
        if not self._version["es2panda"]:
            await self._get_es2panda_version()
        return self._version["playground"], self._version["ark"], self._version['es2panda']

    async def compile_arkts(self,
                            code: str,
                            options: list[str],
                            disasm: bool = False,
                            verifier: bool = False) -> CompileResult:
        """Compile code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="compile")
        log.info("Compilation pipeline started", stage="compile", disasm=disasm, verifier=verifier)
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile_name = await self._save_code(tempdir, code)

            compile_result = await self._compile(stsfile_name, options)
            log = log.bind(compile_ret=compile_result['exit_code'])

            res: CompileResult = {"compile": compile_result, "disassembly": None, "verifier": None}
            if compile_result["exit_code"] != 0:
                log.error("Compilation failed with nonzero exit code")
                return res
            log.info("Compilation succeeded")

            if disasm and compile_result["exit_code"] == 0:
                disassembly = await self.disassembly(f"{stsfile_name}.abc", f"{stsfile_name}.pa")
                log.debug("Disassembling completed", dis_exit_code=disassembly["exit_code"])
                res["disassembly"] = disassembly
            if verifier and compile_result["exit_code"] == 0:
                verifier_res = await self._verify(f"{stsfile_name}.abc")
                log = logger.bind(verifier_ret=verifier_res['exit_code'])
                log.debug("Verification completed", verify_exit_code=verifier_res["exit_code"])
                res["verifier"] = verifier_res

            return res

    async def compile_run_arkts(self,
                                code: str,
                                options: list[str],
                                disasm: bool = False,
                                **kwargs: Unpack[CompileRunParams]) -> CompileRunResult:
        """Compile, run code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="compile-run")
        log.info("Compile and run pipeline started", stage="compile", disasm=disasm)
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile_name = await self._save_code(tempdir, code)
            abcfile_name = f"{stsfile_name}.abc"

            compile_result = await self._compile(stsfile_name, options)
            log = log.bind(compile_ret=compile_result['exit_code'])

            res: CompileRunResult = {"compile": compile_result, "run": None, "disassembly": None, "verifier": None}
            if compile_result["exit_code"] != 0:
                log.error("Compilation failed with nonzero exit code")
                return res

            runtime_verify: bool = kwargs.get("runtime_verify", False)
            log = log.bind(runtime_verify=runtime_verify)
            run_result = await self._run(abcfile_name, runtime_verify=runtime_verify)
            res["run"] = run_result
            log = log.bind(compile_ret=compile_result['exit_code'])
            log.info("Program run completed")

            if disasm:
                disasm_res = await self.disassembly(f"{stsfile_name}.abc", f"{stsfile_name}.pa")
                log.debug("Disassembling completed", dis_exit_code=disasm_res["exit_code"])
                res["disassembly"] = disasm_res
            if kwargs.get("verifier", False):
                verifier_res = await self._verify(f"{stsfile_name}.abc")
                log.debug("Verification completed", verify_exit_code=verifier_res["exit_code"])
                res["verifier"] = verifier_res
            return res

    async def disassembly(self, input_file: str, output_file: str) -> DisasmResult:
        """Call ark_disasm and read disassembly from file
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="disasm")
        stdout, stderr, retcode = await self._execute_cmd(self.binary.disasm_bin, input_file, output_file)
        log = log.bind(disasm_ret=retcode)
        if retcode == -11:
            stderr += "disassembly: Segmentation fault"
            log.error("Disassembling failed: segmentation fault")
            DISASM_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            DISASM_FAILURES.labels("normal").inc()
        result: DisasmResult = {"output": stdout, "error": stderr, "code": None, "exit_code": 0}
        if retcode != 0:
            result["exit_code"] = retcode
            log.error("Disassembling failed with nonzero exit code")
            return result
        async with aiofiles.open(output_file, mode="r", encoding="utf-8", errors="replace") as f:
            result["code"] = await f.read()
        return result

    async def dump_ast(self, code: str, options: Sequence[str] | None = None) -> AstViewExecResult:
        """Parse ETS code and return AST dump strictly from stdout."""
        log = logger.bind(stage="astdump")
        opts = list(options) if options else []
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile_path = await self._save_code(tempdir, code)
            base = ["--dump-ast", "--extension=ets", f"--output={stsfile_path}.abc"]
            args = base + opts + [stsfile_path]

            stdout, stderr, retcode = await self._execute_cmd(self.binary.es2panda, *args)
            log = log.bind(astdump_ret=retcode)

            if retcode == -11:
                stderr += "ast: Segmentation fault"
                log.error("AST dump failed: segmentation fault")
                AST_FAILURES.labels("segfault").inc()
            elif retcode != 0:
                AST_FAILURES.labels("normal").inc()
                log.info("Ast dump completed")

            ast_out = stdout if (retcode == 0 and stdout) else None

            return {
                "ast": ast_out,
                "output": stdout,
                "error": stderr,
                "exit_code": retcode
            }

    async def _compile(self, filename: str, options: list[str]) -> SubprocessExecResult:
        """Compiles ets code stored in file
        :rtype: Dict[str, Any]
        """
        options.extend(["--extension=ets", f"--output={filename}.abc", filename])
        stdout, stderr, retcode = await self._execute_cmd(self.binary.es2panda, *options)
        if retcode == -11:
            stderr += "compilation: Segmentation fault"
            logger.error("Compilation failed: segmentation fault")
            COMPILER_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            COMPILER_FAILURES.labels("normal").inc()
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _verify(self, input_file: str) -> SubprocessExecResult:
        """Runs verifier for input abc file
        :rtype: Dict[str, Any]
        """
        stdout, stderr, retcode = await self._execute_cmd(
            self.binary.verifier,
            f"--boot-panda-files={self.stdlib_abc}",
            "--load-runtimes=ets",
            input_file
        )
        if retcode == -11:
            stderr += "compilation: Segmentation fault"
            logger.error("Verification failed: segmentation fault")
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _execute_cmd(self, cmd: str, *args: str) -> tuple[str, str, int | None]:
        """Create and executes command
        :rtype: Tuple[str, str, int | None]
        """
        proc = await asyncio.create_subprocess_exec(
            cmd,
            *args,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        try:
            stdout_b, stderr_b = await asyncio.wait_for(proc.communicate(), timeout=self._exec_timeout)
        except asyncio.exceptions.TimeoutError:
            proc.kill()
            stdout_b, stderr_b = b"", b"Timeout expired"
            logger.error("Subprocess execution timed out", timeout=self._exec_timeout)

        stdout = stdout_b.decode("utf-8", errors="replace")
        stderr = stderr_b.decode("utf-8", errors="replace")
        return stdout, stderr, proc.returncode

    async def _get_arkts_version(self) -> None:
        """Get ark version
        :rtype: None
        """
        output, _, _ = await self._execute_cmd(self.binary.ark_bin, "--version")
        self._version["ark"] = output

    async def _get_es2panda_version(self):
        """Get ark version
        :rtype: None
        """
        _, stderr, _ = await self._execute_cmd(self.binary.es2panda, "--version")
        self._version["es2panda"] = stderr

    def _get_playground_version(self) -> None:
        """Get arkts_playground package version
        :rtype: None
        """
        try:
            self._version["playground"] = version("arkts_playground")
        except PackageNotFoundError:
            self._version["playground"] = "not installed"

    async def _run(self, abcfile_name: str, runtime_verify: bool = False) -> SubprocessExecResult:
        """Runs abc file
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="run")
        entry_point = f"{Path(abcfile_name).stem[:-4]}.ETSGLOBAL::main"
        run_cmd = [f"--boot-panda-files={self.stdlib_abc}", "--load-runtimes=ets"]
        if runtime_verify:
            run_cmd += ["--verification-mode=ahead-of-time"]
        if self._icu_data:
            run_cmd.append(f"--icu-data-path={self._icu_data}")
        run_cmd.append(abcfile_name)
        run_cmd.append(entry_point)
        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_bin, *run_cmd)
        log = log.bind(run_ret=retcode)
        if retcode == -11:
            stderr += "run: Segmentation fault"
            log.error("Program run failed: segmentation fault",
                      runtime_verify=runtime_verify,
                      abcfile_name=abcfile_name)
            RUNTIME_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            RUNTIME_FAILURES.labels("normal").inc()
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    def _validate(self):
        for f in [self.binary.disasm_bin, self.binary.ark_bin, self.binary.es2panda, self.stdlib_abc]:
            if not Path(f).exists():
                raise RuntimeError(f"\"{f}\" doesn't exist")


@lru_cache
def get_runner() -> Runner:
    """Return Runner instance
    :rtype: Runner
    """
    sett = get_settings()
    return Runner(sett.binary.build, sett.binary.timeout, sett.binary.icu_data)
