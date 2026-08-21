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

from src.schema import (
    AppFlow,
    Command,
    Flow,
    Macro,
    RepeatMacro,
    ScreenshotCommand,
    SnapshotCommand,
    UnprocessedAppFlow,
    UnprocessedCommand,
    UnprocessedFlow,
    UnprocessedScreenshotCommand,
    UnprocessedSnapshotCommand,
)


def preprocess_flow(flow: UnprocessedFlow) -> Flow:
    apps = [expand_app(app) for app in flow.flow]
    return Flow(flow=apps, desc=flow.desc)


def expand_app(app: UnprocessedAppFlow) -> AppFlow:
    return AppFlow(
        label=app.label,
        bundle=app.bundle,
        ability=app.ability,
        terminate=app.terminate,
        commands=expand_commands(app.commands),
    )


def expand_commands(commands: list[Macro | Command]) -> list[Command]:
    expanded: list[Command] = []
    for command in commands:
        expanded.extend(expand_item(command))
    return expanded


def expand_item(command: Macro | Command) -> list[Command]:
    if isinstance(command, RepeatMacro):
        return expand_repeat_macro(command)
    return [command]


def expand_repeat_macro(macro: RepeatMacro) -> list[Command]:
    repeat = macro.payload
    expanded: list[Command] = []
    for index in range(repeat.n_iter):
        for body in repeat.commands:
            expanded.append(expand_body_command(body, repeat.iter_var, index))
    return expanded


def expand_body_command(
    command: UnprocessedCommand,
    var: str,
    index: int,
) -> Command:
    if isinstance(command, UnprocessedSnapshotCommand):
        return SnapshotCommand(
            action="snapshot",
            payload=substitute(command.payload, var, index),
        )
    if isinstance(command, UnprocessedScreenshotCommand):
        return ScreenshotCommand(
            action="screenshot",
            payload=substitute(command.payload, var, index),
        )
    if isinstance(command.payload, str):
        return command.model_copy(update={
            "payload": substitute(command.payload, var, index)})

    return command.model_copy()


def substitute(value: str, var: str, index: int) -> str:
    return value.replace("{" + var + "}", str(index))
