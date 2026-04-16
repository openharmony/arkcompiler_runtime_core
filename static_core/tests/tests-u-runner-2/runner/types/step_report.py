#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

from dataclasses import dataclass

from runner.options.options_step import StepKind


@dataclass
class StepReport:
    """
    Log of a step execution, validation and pre/post-requirements checks.
    """
    name: str  # name of step
    step_kind: StepKind = StepKind.NONE  # kind of step
    command_line: str = ""  # executed command line
    cmd_output: str = ""  # content of stdout after executing the command line
    cmd_error: str = ""  # content of stderr after executing the command line
    cmd_return_code: int = 0  # return code of stdout after executing the command line
    validator_messages: str = ""  # messages both info and errors from validators and requirement checks
    extra: str = ""  # additional information
    status: str = ""  # final status of the step (after execution, validator, pre/post-requirements checks), fail_kind?

    def __hash__(self) -> int:
        return hash(self.command_line) + hash(self.name)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, StepReport):
            return NotImplemented
        return self.command_line == other.command_line and self.name == other.name

    def __str__(self) -> str:
        result: list[str] = []
        step = self.name
        if self.command_line:
            result.append(f"{step}: {self.command_line.strip()}")
            result.append(f"{step}: Actual output: '{self.cmd_output.strip()}'")
            result.append(f"{step}: Actual error: '{self.cmd_error.strip()}'")
            result.append(f"{step}: Actual return code: {self.cmd_return_code}")
        if self.validator_messages:
            result.append(f"{step}: Validator messages: {self.validator_messages.strip()}")
        if self.extra:
            result.append(f"{step}: Extra: {self.extra.strip()}")
        if self.step_kind != StepKind.NONE:
            result.append(f"{step}: Step kind: {self.step_kind.value}")
        if self.status:
            status = self.status.strip()
            result.append(f"{step}: Step status: {status}")
        return "\n".join(result)

    def __repr__(self) -> str:
        return f"StepReport<{self.name}>"
