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


import asyncio
from pathlib import Path
from functools import lru_cache
from importlib.metadata import version, PackageNotFoundError
from typing import Tuple, Dict, Any

import aiofiles

from ..config import get_settings


class Runner:
    def __init__(self, build: str, timeout: int = 120):
        self._exec_timeout = timeout
        _build = Path(build).expanduser()
        bin_path = _build / "bin"
        self._version = {
            "ark": "",
            "playground": "",
            "es2panda": ""
        }
        self.disasm_bin = str(bin_path / "ark_disasm")
        self.ark_bin = str(bin_path / "ark")
        self.es2panda = str(bin_path / "es2panda")
        self.stdlib_abc = str(_build / "plugins" / "ets" / "etsstdlib.abc")
        self._validate()

    @staticmethod
    def parse_compile_options(options: dict) -> list:
        res = []
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
                                                        suffix=".sts",
                                                        delete=False) as stsfile:
            await stsfile.write(code)
            return str(stsfile.name)

    async def get_versions(self) -> Tuple[str, str, str]:
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

    async def compile_arkts(self, code: str, options: list, disasm: bool = False) -> Dict[str, Any]:
        """Compile code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile_name = await self._save_code(tempdir, code)
            compile_result = await self._compile(stsfile_name, options)
            res = {"compile": compile_result, "disassembly": None}
            if disasm and compile_result["exit_code"] == 0:
                disassembly = await self.disassembly(f"{stsfile_name}.abc", f"{stsfile_name}.pa")
                res["disassembly"] = disassembly
            return res

    async def compile_run_arkts(self, code: str, options: list, disasm: bool = False) -> Dict[str, Any]:
        """Compile, run code and make disassemble if disasm argument is True
        :rtype: Dict[str, Any]
        """
        async with aiofiles.tempfile.TemporaryDirectory(prefix="arkts_playground") as tempdir:
            stsfile_name = await self._save_code(tempdir, code)
            abcfile_name = f"{stsfile_name}.abc"
            compile_result = await self._compile(stsfile_name, options)
            res = {"compile": compile_result, "run": None, "disassembly": None}
            if compile_result["exit_code"] != 0:
                return res

            run_result = await self._run(abcfile_name)
            res["run"] = run_result
            if disasm:
                disasm_res = await self.disassembly(f"{stsfile_name}.abc", f"{stsfile_name}.pa")
                res["disassembly"] = disasm_res
            return res

    async def disassembly(self, input_file: str, output_file: str) -> Dict[str, Any]:
        """Call ark_disasm and read disassembly from file
        :rtype: Dict[str, Any]
        """
        stdout, stderr, retcode = await self._execute_cmd(self.disasm_bin, input_file, output_file)
        if retcode == -11:
            stderr += "disassembly: Segmentation fault"
        result = {"output": stdout, "error": stderr, "code": None, "exit_code": 0}
        if retcode != 0:
            result["exit_code"] = retcode
            return result
        async with aiofiles.open(output_file, mode="r", encoding="utf-8") as f:
            result["code"] = await f.read()
        return result

    async def _compile(self, filename: str, options: list) -> Dict[str, Any]:
        """Compiles sts code stored in file
        :rtype: Dict[str, Any]
        """
        options.extend(["--extension=sts", f"--output={filename}.abc", filename])
        stdout, stderr, retcode = await self._execute_cmd(self.es2panda, *options)
        if retcode == -11:
            stderr += "compilation: Segmentation fault"
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    async def _execute_cmd(self, cmd, *args) -> Tuple[str, str, int | None]:
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
            stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=self._exec_timeout)
        except asyncio.exceptions.TimeoutError:
            proc.kill()
            stdout, stderr = b"", b"Timeout expired"

        return stdout.decode("utf-8"), stderr.decode("utf-8"), proc.returncode

    async def _get_arkts_version(self) -> None:
        """Get ark version
        :rtype: None
        """
        output, _, _ = await self._execute_cmd(self.ark_bin, "--version")
        self._version["ark"] = output

    async def _get_es2panda_version(self):
        """Get ark version
        :rtype: None
        """
        _, stderr, _ = await self._execute_cmd(self.es2panda, "--version")
        self._version["es2panda"] = stderr

    def _get_playground_version(self) -> None:
        """Get arkts_playground package version
        :rtype: None
        """
        try:
            self._version["playground"] = version("arkts_playground")
        except PackageNotFoundError:
            self._version["playground"] = "not installed"

    async def _run(self, abcfile_name: str) -> Dict[str, Any]:
        """Runs abc file
        :rtype: Dict[str, Any]
        """
        run_cmd = [f"--boot-panda-files={self.stdlib_abc}", "--load-runtimes=ets", abcfile_name, "ETSGLOBAL::main"]
        stdout, stderr, retcode = await self._execute_cmd(self.ark_bin, *run_cmd)
        if retcode == -11:
            stderr += "run: Segmentation fault"
        return {"output": stdout, "error": stderr, "exit_code": retcode}

    def _validate(self):
        for f in [self.disasm_bin, self.ark_bin, self.es2panda, self.stdlib_abc]:
            if not Path(f).exists():
                raise RuntimeError(f"\"{f}\" doesn't exist")


@lru_cache
def get_runner() -> Runner:
    """Return Runner instance
    :rtype: Runner
    """
    sett = get_settings()
    return Runner(sett.binary.build, sett.binary.timeout)
