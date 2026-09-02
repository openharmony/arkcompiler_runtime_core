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

import subprocess
import sys
from typing import Any

from src.hdc import HdcResult, TIMEOUT_ERR_CODE


class FakeHdc:
    def __init__(
        self,
        default_result: HdcResult | None = None,
        responses: dict[str, HdcResult] | None = None,
        prefix_responses: dict[str, HdcResult] | None = None,
        timeout_after: int = -1,
        start_shell_process: Any | None = None,
    ) -> None:
        self.calls: list[list[object]] = []
        self.default_result = default_result or HdcResult(0, "", "")
        self.responses = responses or {}
        self.prefix_responses = prefix_responses or {}
        self.timeout_after = timeout_after
        self.start_shell_process = start_shell_process
        self.started_processes: list[Any] = []

    def run(self, *args: str, timeout: int = -1) -> HdcResult:
        return self._record_and_result(args, timeout)

    def shell(self, *args: str, timeout: int = -1) -> HdcResult:
        return self.run("shell", *args, timeout=timeout)

    def shell_raw(self, command: str, timeout: int = -1) -> HdcResult:
        return self.run("shell", command, timeout=timeout)

    def start_shell(
        self,
        command: str,
        stdout: int | None = subprocess.DEVNULL,
        stderr: int | None = subprocess.DEVNULL,
    ) -> Any:
        self.calls.append(["shell", command])
        process = self.start_shell_process
        if process is None:
            process = subprocess.Popen(
                [sys.executable, "-c", "import time; time.sleep(3600)"],
                stdout=stdout,
                stderr=stderr,
            )
        self.started_processes.append(process)
        return process

    def _record_and_result(self, args: tuple[str, ...], timeout: int) -> HdcResult:
        self.calls.append(list(args))
        if self.timeout_after >= 0 and 0 <= timeout < self.timeout_after:
            return HdcResult(TIMEOUT_ERR_CODE, "", f"memmem: TIMEOUT {list(args)}")
        key = " ".join(args)
        if key in self.responses:
            return self.responses[key]
        for prefix, result in self.prefix_responses.items():
            if key.startswith(prefix):
                return result
        return self.default_result
