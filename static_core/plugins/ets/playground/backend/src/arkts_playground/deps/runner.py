#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import os
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
from ..models.common import IrDumpFileItem, VerificationMode

logger = structlog.stdlib.get_logger(__name__)


class Binary(NamedTuple):
    ark_bin: str
    es2panda: str
    disasm_bin: str
    verifier: str
    ark_aot: str


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


class IrDumpResult(TypedDict):
    output: str
    error: str
    compiler_dump: list[IrDumpFileItem]
    disasm_dump: str | None
    exit_code: int | None


class IrDumpOptions(TypedDict, total=False):
    enabled: bool
    compiler_dump: bool
    disasm_dump: bool


class IrDumpPaths(NamedTuple):
    output_an: Path
    dump_folder: Path
    disasm_file: Path


class CompileOptions(TypedDict, total=False):
    disasm: bool
    verifier: bool
    ir_dump: IrDumpOptions | None
    aot_mode: bool


class CompileResult(TypedDict):
    compile: SubprocessExecResult
    disassembly: DisasmResult | None
    verifier: SubprocessExecResult | None
    aot_compile: SubprocessExecResult | None
    ir_dump: IrDumpResult | None


class AotCompileRunResult(TypedDict):
    aot_compile: SubprocessExecResult
    aot_run: SubprocessExecResult


class CompileRunResult(TypedDict):
    compile: SubprocessExecResult
    disassembly: DisasmResult | None
    verifier: SubprocessExecResult | None
    run: SubprocessExecResult | None
    aot_compile: SubprocessExecResult | None
    aot_run: SubprocessExecResult | None
    ir_dump: IrDumpResult | None


class CompileRunParams(TypedDict, total=False):
    verification_mode: VerificationMode
    verifier: bool
    ir_dump: IrDumpOptions | None
    aot_mode: bool


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
            verifier=str(bin_path / "verifier"),
            ark_aot=str(bin_path / "ark_aot")
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

    @staticmethod
    def _empty_ir_dump(error: str = "", exit_code: int | None = None) -> IrDumpResult:
        """Create empty IR dump result"""
        return {"output": "", "error": error, "compiler_dump": [], "disasm_dump": None, "exit_code": exit_code}

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
                            **kwargs: Unpack[CompileOptions]) -> CompileResult:
        """Compile code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="compile")
        log.info(
            "Compilation pipeline started",
            disasm=kwargs.get("disasm"),
            verifier=kwargs.get("verifier"),
            aot_mode=kwargs.get("aot_mode", False)
        )
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile = await self._save_code(tempdir, code)
            res: CompileResult = {
                "compile": await self._compile(stsfile, options),
                "disassembly": None, "verifier": None, "ir_dump": None, "aot_compile": None
            }
            log = log.bind(compile_ret=res["compile"]['exit_code'])

            if res["compile"]["exit_code"] != 0:
                log.error("Compilation failed with nonzero exit code")
                return res
            log.info("Compilation succeeded")

            abcfile = f"{stsfile}.abc"
            if kwargs.get("disasm", False):
                disasm_res = await self.disassembly(abcfile, f"{stsfile}.pa")
                res["disassembly"] = disasm_res
                log.debug("Disassembling completed", dis_exit_code=disasm_res["exit_code"])
            if kwargs.get("verifier", False):
                verify_res = await self._verify(abcfile)
                res["verifier"] = verify_res
                log.debug("Verification completed", verify_exit_code=verify_res["exit_code"])
            if kwargs.get("aot_mode", False):
                aot_res = await self._compile_aot_only(abcfile, tempdir)
                res["aot_compile"] = {
                    "output": aot_res.get("aot_compile_output"),
                    "error": aot_res.get("aot_compile_error"),
                    "exit_code": aot_res.get("aot_compile_exit_code")
                }
                log.debug("AOT compilation completed", aot_compile_exit_code=aot_res.get("aot_compile_exit_code"))
            ir_dump = kwargs.get("ir_dump")
            if ir_dump and (ir_dump.get("compiler_dump", False) or ir_dump.get("disasm_dump", False)):
                dump_res = await self.generate_ir_dump(abcfile, tempdir, ir_dump)
                res["ir_dump"] = dump_res
                log.debug("IR dump completed", ir_dump_exit_code=dump_res["exit_code"])

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
            stsfile = await self._save_code(tempdir, code)
            abcfile = f"{stsfile}.abc"
            res: CompileRunResult = {
                "compile": await self._compile(stsfile, options),
                "run": None,
                "aot_compile": None,
                "aot_run": None,
                "disassembly": None,
                "verifier": None,
                "ir_dump": None
            }
            log = log.bind(compile_ret=res["compile"]['exit_code'])

            if res["compile"]["exit_code"] != 0:
                log.error("Compilation failed with nonzero exit code")
                return res

            log = log.bind(
                verification_mode=kwargs.get("verification_mode", VerificationMode.AHEAD_OF_TIME).value,
                aot_mode=kwargs.get("aot_mode", False)
            )

            res["run"] = await self._run(
                abcfile,
                verification_mode=kwargs.get("verification_mode", VerificationMode.AHEAD_OF_TIME)
            )
            if kwargs.get("aot_mode", False):
                aot_res = await self._run_aot(
                    abcfile,
                    tempdir,
                    verification_mode=kwargs.get("verification_mode", VerificationMode.AHEAD_OF_TIME)
                )
                res["aot_compile"] = aot_res["aot_compile"]
                res["aot_run"] = aot_res["aot_run"]
            log.info("Program run completed")

            if disasm:
                res["disassembly"] = dis_res = await self.disassembly(abcfile, f"{stsfile}.pa")
                log.debug("Disassembling completed", dis_exit_code=dis_res["exit_code"])
            if kwargs.get("verifier", False):
                res["verifier"] = ver_res = await self._verify(abcfile)
                log.debug("Verification completed", verify_exit_code=ver_res["exit_code"])
            if (ir_opts := kwargs.get("ir_dump")) and (
                ir_opts.get("compiler_dump") or ir_opts.get("disasm_dump")
            ):
                res["ir_dump"] = ir_res = await self.generate_ir_dump(abcfile, tempdir, ir_opts)
                log.debug("IR dump completed", ir_dump_exit_code=ir_res["exit_code"])
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

    async def generate_ir_dump(self, abc_file: str, tempdir: str,
                               ir_dump_options: IrDumpOptions | None = None) -> IrDumpResult:
        """Generate IR dump using ark_aot compiler"""
        log = logger.bind(stage="ir_dump")

        if not ir_dump_options or not (
            ir_dump_options.get("compiler_dump") or ir_dump_options.get("disasm_dump")
        ):
            return self._empty_ir_dump()

        if not Path(self.binary.ark_aot).exists():
            log.error("ark_aot binary not found", ark_aot_path=self.binary.ark_aot)
            return self._empty_ir_dump("Internal error", 1)

        if not Path(abc_file).exists():
            log.error("ABC file not found", abc_file=abc_file)
            return self._empty_ir_dump("Internal error", 1)

        dump_folder = Path(tempdir) / "ir_dump"
        dump_folder.mkdir(exist_ok=True)
        paths = IrDumpPaths(Path(tempdir) / "output.an", dump_folder, dump_folder / "ir_disasm.txt")

        cmd_args = self._build_ir_dump_cmd(abc_file, paths, ir_dump_options)
        log.info("Starting IR dump generation", options=ir_dump_options)
        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_aot, *cmd_args)

        result: IrDumpResult = {
            "output": stdout, "error": stderr,
            "compiler_dump": [], "disasm_dump": None, "exit_code": retcode
        }

        if retcode == -11:
            result["error"] += "\nir_dump: Segmentation fault"
            log.error("IR dump failed: segmentation fault")
            return result
        if retcode != 0:
            log.error("IR dump failed with nonzero exit code")
            return result

        if ir_dump_options.get("disasm_dump") and paths.disasm_file.exists():
            async with aiofiles.open(paths.disasm_file, mode="r", encoding="utf-8", errors="replace") as f:
                result["disasm_dump"] = await f.read()
        if ir_dump_options.get("compiler_dump"):
            result["compiler_dump"] = await self._read_compiler_dump_files(paths.dump_folder)

        log.info("IR dump completed successfully",
                 has_compiler_dump=len(result["compiler_dump"]) > 0,
                 has_disasm_dump=result["disasm_dump"] is not None)
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

    async def _read_compiler_dump_files(self, dump_folder: Path) -> list[IrDumpFileItem]:
        """Read compiler dump files from folder and return as list of file items"""
        if not dump_folder.exists():
            return []
        dump_files = list(dump_folder.glob("*.ir"))
        if not dump_files:
            return []
        contents: list[IrDumpFileItem] = []
        for dump_file in sorted(dump_files):
            async with aiofiles.open(dump_file, mode="r", encoding="utf-8", errors="replace") as f:
                contents.append({
                    "name": dump_file.name,
                    "content": await f.read()
                })
        return contents

    def _build_ir_dump_cmd(self, abc_file: str, paths: IrDumpPaths,
                           ir_dump_options: IrDumpOptions) -> list[str]:
        """Build command arguments for IR dump generation"""
        cmd_args = [
            f"--boot-panda-files={self.stdlib_abc}",
            "--paoc-mode=aot", "--load-runtimes=ets",
            f"--paoc-panda-files={abc_file}", f"--paoc-output={paths.output_an}",
            "--compiler-ignore-failures=false",
        ]
        if ir_dump_options.get("compiler_dump", False):
            cmd_args.extend(["--compiler-dump", f"--compiler-dump:folder={paths.dump_folder}"])
        if ir_dump_options.get("disasm_dump", False):
            cmd_args.append(
                f"--compiler-disasm-dump:single-file=true,code-info=true,code=true,file-name={paths.disasm_file}"
            )
        return cmd_args

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
        env = os.environ.copy()
        build_lib = str(Path(self.binary.ark_bin).parent.parent / "lib")
        system_lib_paths = (
            "/usr/lib/aarch64-linux-gnu:/usr/lib/x86_64-linux-gnu:"
            "/lib/aarch64-linux-gnu:/lib/x86_64-linux-gnu"
        )

        if "LD_LIBRARY_PATH" in env:
            env["LD_LIBRARY_PATH"] = f"{build_lib}:{system_lib_paths}:{env['LD_LIBRARY_PATH']}"
        else:
            env["LD_LIBRARY_PATH"] = f"{build_lib}:{system_lib_paths}"

        if "ark_aot" in cmd:
            logger.debug("Executing ark_aot", source="aot", ld_library_path=env.get("LD_LIBRARY_PATH"))

        proc = await asyncio.create_subprocess_exec(
            cmd,
            *args,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            env=env
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

    async def _run(
        self,
        abcfile_name: str,
        verification_mode: VerificationMode = VerificationMode.AHEAD_OF_TIME
    ) -> SubprocessExecResult:
        """Runs abc file
        :rtype: Dict[str, Any]
        """
        log = logger.bind(stage="run")
        entry_point = f"{Path(abcfile_name).with_suffix('').stem}.ETSGLOBAL::main"
        run_cmd = [f"--boot-panda-files={self.stdlib_abc}", "--load-runtimes=ets"]
        run_cmd += [f"--verification-mode={verification_mode.value}"]
        if self._icu_data:
            run_cmd.append(f"--icu-data-path={self._icu_data}")
        run_cmd.append(abcfile_name)
        run_cmd.append(entry_point)

        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_bin, *run_cmd)

        log = log.bind(run_ret=retcode)
        if retcode == -11:
            stderr += "run: Segmentation fault"
            log.error("Program run failed: segmentation fault",
                      verification_mode=verification_mode,
                      abcfile_name=abcfile_name)
            RUNTIME_FAILURES.labels("segfault").inc()
        elif retcode != 0:
            RUNTIME_FAILURES.labels("normal").inc()

        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _aot_compile(self, abcfile: str, output_an: Path) -> tuple[str, str, int | None]:
        """Compile ABC to native code using ark_aot"""
        aot_cmd = [
            f"--boot-panda-files={self.stdlib_abc}", "--paoc-mode=aot", "--load-runtimes=ets",
            f"--paoc-panda-files={abcfile}", f"--paoc-output={output_an}", "--compiler-ignore-failures=false",
        ]
        stdout, stderr, retcode = await self._execute_cmd(self.binary.ark_aot, *aot_cmd)
        return stdout, stderr, retcode

    def _build_aot_run_cmd(
        self, abcfile: str,
        verification_mode: VerificationMode = VerificationMode.AHEAD_OF_TIME
    ) -> list[str]:
        """Build command for running AOT compiled code"""
        entry_point = f"{Path(abcfile).with_suffix('').stem}.ETSGLOBAL::main"
        cmd = [f"--boot-panda-files={self.stdlib_abc}", "--load-runtimes=ets", "--enable-an:force"]
        cmd.append(f"--verification-mode={verification_mode.value}")
        if self._icu_data:
            cmd.append(f"--icu-data-path={self._icu_data}")
        cmd.extend([abcfile, entry_point])
        return cmd

    async def _compile_aot_only(self, abcfile: str, tempdir: str) -> SubprocessExecResult:
        """Compiles ABC to native code using AOT (compilation only, no execution)"""
        log = logger.bind(source="aot_compile")

        if not Path(self.binary.ark_aot).exists():
            log.error("ark_aot binary not found", ark_aot_path=self.binary.ark_aot)
            return {
                "aot_compile_output": "",
                "aot_compile_error": "Internal error",
                "aot_compile_exit_code": 1,
                "run_output": "",
                "run_error": "",
                "exit_code": None,
            }

        output_an = Path(tempdir) / "output.an"
        log.info("Starting AOT compilation (compile-only mode)", abc_file=abcfile)
        aot_out, aot_err, aot_ret = await self._aot_compile(abcfile, output_an)

        log = log.bind(aot_compile_ret=aot_ret)
        if aot_ret == -11:
            aot_err += "\nAOT compilation: Segmentation fault"
            log.error("AOT compilation failed: segmentation fault")
            RUNTIME_FAILURES.labels("aot_segfault").inc()
        elif aot_ret != 0:
            log.error("AOT compilation failed", exit_code=aot_ret)
            RUNTIME_FAILURES.labels("aot_compile_failure").inc()
        else:
            log.info("AOT compilation succeeded")

        return {
            "aot_compile_output": aot_out,
            "aot_compile_error": aot_err,
            "aot_compile_exit_code": aot_ret,
            "run_output": "",
            "run_error": "",
            "exit_code": None,
        }

    async def _run_aot(
        self, abcfile: str, tempdir: str,
        verification_mode: VerificationMode = VerificationMode.AHEAD_OF_TIME
    ) -> AotCompileRunResult:
        """Compiles ABC to native code using AOT and runs it"""
        log = logger.bind(source="aot")

        if not Path(self.binary.ark_aot).exists():
            log.error("ark_aot binary not found", ark_aot_path=self.binary.ark_aot)
            return {
                "aot_compile": {
                    "output": "",
                    "error": "Internal error",
                    "exit_code": 1
                },
                "aot_run": {
                    "output": "",
                    "error": "",
                    "exit_code": 1
                }
            }

        output_an = Path(tempdir) / "output.an"
        log.info("Starting AOT compilation", abc_file=abcfile)
        aot_out, aot_err, aot_ret = await self._aot_compile(abcfile, output_an)

        log = log.bind(aot_compile_ret=aot_ret)
        if aot_ret == -11:
            aot_err += "\nAOT compilation: Segmentation fault"
            log.error("AOT compilation failed: segmentation fault")
            RUNTIME_FAILURES.labels("aot_segfault").inc()
        elif aot_ret != 0:
            log.error("AOT compilation failed", exit_code=aot_ret)
            RUNTIME_FAILURES.labels("aot_compile_failure").inc()

        aot_compile_result: SubprocessExecResult = {
            "output": aot_out,
            "error": aot_err,
            "exit_code": aot_ret
        }

        if aot_ret != 0:
            log.info("Skipping AOT execution due to compilation failure")
            return {
                "aot_compile": aot_compile_result,
                "aot_run": {
                    "output": "",
                    "error": "",
                    "exit_code": None
                }
            }

        log.info("AOT compilation succeeded, starting execution")
        stdout, stderr, retcode = await self._execute_cmd(
            self.binary.ark_bin, *self._build_aot_run_cmd(abcfile, verification_mode)
        )

        if retcode == -11:
            stderr += "\nrun (AOT): Segmentation fault"
            log.error("AOT program run failed: segmentation fault")
            RUNTIME_FAILURES.labels("aot_run_segfault").inc()
        elif retcode != 0:
            RUNTIME_FAILURES.labels("aot_run_failure").inc()

        aot_run_result: SubprocessExecResult = {
            "output": stdout,
            "error": stderr,
            "exit_code": retcode
        }

        return {
            "aot_compile": aot_compile_result,
            "aot_run": aot_run_result
        }

    def _validate(self):
        for f in [self.binary.disasm_bin, self.binary.ark_bin, self.binary.es2panda, self.stdlib_abc]:
            if not Path(f).exists():
                raise RuntimeError(f"\"{f}\" doesn't exist")
        if not Path(self.binary.ark_aot).exists():
            logger.warning("ark_aot binary not found - IR dump feature will not work",
                           source="aot", ark_aot_path=self.binary.ark_aot)


@lru_cache
def get_runner() -> Runner:
    """Return Runner instance
    :rtype: Runner
    """
    sett = get_settings()
    return Runner(sett.binary.build, sett.binary.timeout, sett.binary.icu_data)
