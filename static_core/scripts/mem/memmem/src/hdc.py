# -- coding: utf-8 --
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

import dataclasses
import pathlib
import shlex
import subprocess
import typing

from src.log import get_logger

TIMEOUT_ERR_CODE = 127


@dataclasses.dataclass(frozen=True)
class HdcResult:
    returncode: int
    stdout: str
    stderr: str

    def is_error(self) -> bool:
        return self.returncode != 0 or self.stderr != ""


class Hdc:
    def __init__(self, hdc_path: pathlib.Path) -> None:
        self.hdc_path = hdc_path

    def run(self, *args: str, timeout: int = -1) -> HdcResult:
        cmd = [str(self.hdc_path), *args]
        get_logger().info(f"$ {' '.join(cmd)}")
        try:
            completed = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
                timeout=None if timeout == -1 else timeout,
            )
        except subprocess.TimeoutExpired:
            return HdcResult(TIMEOUT_ERR_CODE, "", f"memmem: TIMEOUT {cmd}")
        return HdcResult(
            returncode=completed.returncode,
            stdout=completed.stdout,
            stderr=completed.stderr,
        )

    def shell(self, *args: str, timeout: int = -1) -> HdcResult:
        """Run argv-style arguments through the remote POSIX shell."""
        if not args:
            return self.run("shell", timeout=timeout)
        return self.run("shell", shlex.join(args), timeout=timeout)

    def shell_raw(self, command: str, timeout: int = -1) -> HdcResult:
        """Run a pre-serialized remote command that deliberately uses shell syntax."""
        return self.run("shell", command, timeout=timeout)

    def start_shell(
        self,
        command: str,
        stdout: int | None = subprocess.DEVNULL,
        stderr: int | None = subprocess.DEVNULL,
    ) -> subprocess.Popen[typing.Any]:
        """Start a pre-serialized remote shell command as a child process."""
        return subprocess.Popen(
            [str(self.hdc_path), "shell", command],
            stdout=stdout,
            stderr=stderr,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
