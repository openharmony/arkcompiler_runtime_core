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

__all__ = [
    "AppFlow",
    "Command",
    "Device",
    "DirectionalFlingCommand",
    "DoubleTapCommand",
    "DragCommand",
    "FlingCommand",
    "Flow",
    "Hdc",
    "InputTextCommand",
    "KeyCommand",
    "LongTapCommand",
    "Macro",
    "RepeatMacro",
    "ScreenshotCommand",
    "SnapshotCommand",
    "SwipeCommand",
    "TapCommand",
    "TextCommand",
    "UnprocessedAppFlow",
    "UnprocessedFlow",
    "UnprocessedScreenshotCommand",
    "UnprocessedSnapshotCommand",
    "WaitCommand",
    "app_flow",
    "configure_logger",
    "directional_fling",
    "double_tap",
    "drag",
    "fling",
    "flow",
    "get_device",
    "get_hdc",
    "get_logger",
    "input_text",
    "key",
    "long_tap",
    "preprocess_flow",
    "repeat",
    "reset_logger",
    "run",
    "screenshot",
    "snapshot",
    "swipe",
    "tap",
    "text",
    "unprocessed_app_flow",
    "unprocessed_flow",
    "unprocessed_screenshot",
    "unprocessed_snapshot",
    "wait",
]

import pathlib
import re
import typing

from src.device import Device
from src.hdc import Hdc
from src.log import configure_logger, get_logger, reset_logger
from src.preprocess import preprocess_flow as _preprocess_flow
from src.types import positive_int
from src.path import absolutize
from src.report import generate_reports, average_reports
from src.result import ResultStore
from src.runner import BenchmarkOptions, create_result_store, run_benchmark
from src.schema import (
    AppFlow,
    Command,
    DirectionalFlingCommand,
    DirectionalFlingPayload,
    DoubleTapCommand,
    DragCommand,
    FlingCommand,
    FlingPayload,
    Flow,
    InputTextCommand,
    InputTextPayload,
    KeyCommand,
    LongTapCommand,
    Macro,
    NamedKeyPayload,
    RepeatMacro,
    RepeatMacroPayload,
    ScreenshotCommand,
    SnapshotCommand,
    SwipeCommand,
    SwipePayload,
    TapCommand,
    TapPayload,
    TextCommand,
    UnprocessedAppFlow,
    UnprocessedCommand,
    UnprocessedFlow,
    UnprocessedScreenshotCommand,
    UnprocessedSnapshotCommand,
    WaitCommand,
)


def get_hdc(hdc_exe_path: str | pathlib.Path) -> Hdc:
    return Hdc(absolutize(pathlib.Path(hdc_exe_path)))


def get_device(hdc: Hdc) -> Device:
    return Device(hdc)


def run(
    f: Flow,
    d: Device,
    *,
    out_dir: str | pathlib.Path,
    reboot: bool = False,
    hilog: bool = True,
    repeats: int = 1,
    smaps_filter: re.Pattern[str] | None = None,
) -> None:
    validated_repeats = positive_int(repeats)
    validated_flow = Flow.model_validate(f.model_dump())

    resolved_out_dir = absolutize(pathlib.Path(out_dir))
    resolved_out_dir.mkdir(parents=True, exist_ok=False)

    def run_single(
        out: pathlib.Path,
        *,
        local_root_precreated: bool = False,
    ) -> ResultStore:
        options = BenchmarkOptions(
            out_dir=out,
            reboot=reboot,
            hilog=hilog,
        )
        store = create_result_store(options)
        run_benchmark(
            validated_flow,
            options,
            store,
            d,
            local_root_precreated=local_root_precreated,
        )
        generate_reports(store, smaps_filter)
        return store

    if validated_repeats == 1:
        run_single(resolved_out_dir, local_root_precreated=True)
    else:
        out_dirs: list[ResultStore] = []
        for i in range(validated_repeats):
            out_dirs.append(run_single(resolved_out_dir.joinpath(
                f"iteration_{i}")))
        average_reports(resolved_out_dir, out_dirs)


def flow(apps: list[AppFlow], desc: str | None = None) -> Flow:
    return Flow(flow=apps, desc=desc)


def app_flow(
    label: str,
    bundle: str,
    ability: str,
    terminate: bool,
    commands: list[Command],
) -> AppFlow:
    return AppFlow(
        label=label,
        bundle=bundle,
        ability=ability,
        terminate=terminate,
        commands=commands,
    )


def wait(seconds: float) -> WaitCommand:
    return WaitCommand(action="wait", payload=seconds)


def snapshot(label: str) -> SnapshotCommand:
    return SnapshotCommand(action="snapshot", payload=label)


def screenshot(label: str) -> ScreenshotCommand:
    return ScreenshotCommand(action="screenshot", payload=label)


def key(name: typing.Literal["Home", "Back", "Power"]) -> KeyCommand:
    return KeyCommand(action="key", payload=NamedKeyPayload(key=name))


def tap(x_pct: int, y_pct: int) -> TapCommand:
    return TapCommand(action="tap", payload=TapPayload(x_pct=x_pct, y_pct=y_pct))


def double_tap(x_pct: int, y_pct: int) -> DoubleTapCommand:
    return DoubleTapCommand(
        action="double_tap", payload=TapPayload(x_pct=x_pct, y_pct=y_pct)
    )


def long_tap(x_pct: int, y_pct: int) -> LongTapCommand:
    return LongTapCommand(
        action="long_tap", payload=TapPayload(x_pct=x_pct, y_pct=y_pct)
    )


def swipe(
    x1_pct: int,
    y1_pct: int,
    x2_pct: int,
    y2_pct: int,
    velocity: int,
) -> SwipeCommand:
    return SwipeCommand(
        action="swipe",
        payload=SwipePayload(
            x1_pct=x1_pct,
            y1_pct=y1_pct,
            x2_pct=x2_pct,
            y2_pct=y2_pct,
            velocity=velocity,
        ),
    )


def drag(
    x1_pct: int,
    y1_pct: int,
    x2_pct: int,
    y2_pct: int,
    velocity: int,
) -> DragCommand:
    return DragCommand(
        action="drag",
        payload=SwipePayload(
            x1_pct=x1_pct,
            y1_pct=y1_pct,
            x2_pct=x2_pct,
            y2_pct=y2_pct,
            velocity=velocity,
        ),
    )


def fling(
    x1_pct: int,
    y1_pct: int,
    x2_pct: int,
    y2_pct: int,
    velocity: int,
    step_length: int,
) -> FlingCommand:
    return FlingCommand(
        action="fling",
        payload=FlingPayload(
            x1_pct=x1_pct,
            y1_pct=y1_pct,
            x2_pct=x2_pct,
            y2_pct=y2_pct,
            velocity=velocity,
            step_length=step_length,
        ),
    )


def directional_fling(
    direction: typing.Literal["left", "right", "up", "down"],
    velocity: int,
    step_length: int,
) -> DirectionalFlingCommand:
    return DirectionalFlingCommand(
        action="directional_fling",
        payload=DirectionalFlingPayload(
            direction=direction,
            velocity=velocity,
            step_length=step_length,
        ),
    )


def input_text(x_pct: int, y_pct: int, payload: str) -> InputTextCommand:
    return InputTextCommand(
        action="input_text",
        payload=InputTextPayload(x_pct=x_pct, y_pct=y_pct, text=payload),
    )


def text(payload: str) -> TextCommand:
    return TextCommand(action="text", payload=payload)


def unprocessed_flow(
    apps: list[UnprocessedAppFlow], desc: str | None = None
) -> UnprocessedFlow:
    return UnprocessedFlow(flow=apps, desc=desc)


def unprocessed_app_flow(
    label: str,
    bundle: str,
    ability: str,
    terminate: bool,
    commands: list[Macro | Command],
) -> UnprocessedAppFlow:
    return UnprocessedAppFlow(
        label=label,
        bundle=bundle,
        ability=ability,
        terminate=terminate,
        commands=commands,
    )


def unprocessed_snapshot(label: str) -> UnprocessedSnapshotCommand:
    return UnprocessedSnapshotCommand(action="snapshot", payload=label)


def unprocessed_screenshot(label: str) -> UnprocessedScreenshotCommand:
    return UnprocessedScreenshotCommand(action="screenshot", payload=label)


def repeat(
    n_iter: int,
    iter_var: str,
    commands: list[UnprocessedCommand],
) -> RepeatMacro:
    return RepeatMacro(
        macro="repeat",
        payload=RepeatMacroPayload(
            iter_var=iter_var, n_iter=n_iter, commands=commands),
    )


def preprocess_flow(uflow: UnprocessedFlow) -> Flow:
    validated_flow = UnprocessedFlow.model_validate(
        uflow.model_dump())
    return _preprocess_flow(validated_flow)
