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
import subprocess
import time
import typing

from src.device import Device, Point, ScreenBounds
from src.metadata import AppMetadata, ArtifactMetadata, RelativeParts
from src.result import ResultStore
from src.schema import (
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
    NamedKeyPayload,
    ScreenshotCommand,
    SnapshotCommand,
    SwipeCommand,
    SwipePayload,
    TapCommand,
    TapPayload,
    TextCommand,
    WaitCommand,
)


@dataclasses.dataclass(frozen=True)
class PendingArtifact:
    remote_base: pathlib.PurePosixPath
    local_base: pathlib.Path
    artifact: RelativeParts


@dataclasses.dataclass
class ExecutionContext:
    device: Device
    store: ResultStore
    flow: Flow
    apps: list[AppMetadata]
    pending_artifacts: list[PendingArtifact]
    artifact_metadata: dict[pathlib.Path, list[ArtifactMetadata]]
    child_processes: list[subprocess.Popen[typing.Any]]
    screen_bounds: ScreenBounds


def execute_command(command: Command, context: ExecutionContext) -> None:
    if isinstance(command, WaitCommand):
        execute_wait(command.payload)
        return
    if isinstance(command, SnapshotCommand):
        execute_snapshot(command.payload, context)
        return
    if isinstance(command, ScreenshotCommand):
        execute_screenshot(command.payload, context)
        return
    if isinstance(command, KeyCommand):
        execute_key(command.payload, context)
        return
    if isinstance(command, TapCommand):
        execute_tap(command.payload, context)
        return
    if isinstance(command, DoubleTapCommand):
        execute_double_tap(command.payload, context)
        return
    if isinstance(command, LongTapCommand):
        execute_long_tap(command.payload, context)
        return
    if isinstance(command, SwipeCommand):
        execute_swipe(command.payload, context)
        return
    if isinstance(command, DragCommand):
        execute_drag(command.payload, context)
        return
    if isinstance(command, FlingCommand):
        execute_fling(command.payload, context)
        return
    if isinstance(command, DirectionalFlingCommand):
        execute_directional_fling(command.payload, context)
        return
    if isinstance(command, InputTextCommand):
        execute_input_text(command.payload, context)
        return
    if isinstance(command, TextCommand):
        execute_text(command.payload, context)
        return
    typing.assert_never(command)


def execute_wait(payload: float) -> None:
    time.sleep(payload)


def execute_snapshot(
    payload: str,
    context: ExecutionContext,
) -> None:
    metadata = ArtifactMetadata(
        label=payload,
        timestamp=context.device.timestamp(),
    )
    remote_snapshot_dir = context.store.remote_snapshot_dir(payload)
    pending_artifacts: list[PendingArtifact] = []
    for process in context.apps:
        if not context.device.pid_exists(process.pid):
            continue

        if not context.device.make_dir(remote_snapshot_dir):
            raise RuntimeError(
                f"failed to create remote snapshot directory for {payload}"
            )
        remote_path = context.store.remote_snapshot_path(
            payload, process.label)
        if not context.device.capture_smaps(process.pid, remote_path):
            raise RuntimeError(
                f"failed to capture smaps for PID {process.pid}")

        artifact = context.store.snapshot_relative_parts(
            payload, process.label)
        pending_artifacts.append(PendingArtifact(
            remote_base=context.store.remote_snapshots_dir(),
            local_base=context.store.local_snapshots_dir(),
            artifact=artifact,
        ))
    context.pending_artifacts.extend(pending_artifacts)
    context.artifact_metadata.setdefault(
        context.store.local_snapshots_dir(), []).append(metadata)


def execute_screenshot(
    payload: str,
    context: ExecutionContext,
) -> None:
    metadata = ArtifactMetadata(
        label=payload,
        timestamp=context.device.timestamp(),
    )
    remote_path = context.store.remote_screenshot_path(payload)
    if not context.device.capture_screenshot(remote_path):
        raise RuntimeError(f"failed to capture screenshot {payload}")
    context.artifact_metadata.setdefault(
        context.store.local_screenshots_dir(), []).append(metadata)
    artifact = context.store.screenshot_relative_parts(payload)
    context.pending_artifacts.append(PendingArtifact(
        remote_base=context.store.remote_screenshots_dir(),
        local_base=context.store.local_screenshots_dir(),
        artifact=artifact,
    ))


def execute_key(
    payload: NamedKeyPayload,
    context: ExecutionContext,
) -> None:
    context.device.send_key(payload.key)


def execute_tap(payload: TapPayload, context: ExecutionContext) -> None:
    context.device.tap(
        Point.from_normalized(context.screen_bounds, payload.x_pct, payload.y_pct))


def execute_double_tap(payload: TapPayload, context: ExecutionContext) -> None:
    context.device.double_tap(
        Point.from_normalized(context.screen_bounds, payload.x_pct, payload.y_pct))


def execute_long_tap(payload: TapPayload, context: ExecutionContext) -> None:
    context.device.long_tap(
        Point.from_normalized(context.screen_bounds, payload.x_pct, payload.y_pct))


def execute_swipe(payload: SwipePayload, context: ExecutionContext) -> None:
    start = Point.from_normalized(
        context.screen_bounds, payload.x1_pct, payload.y1_pct)
    end = Point.from_normalized(
        context.screen_bounds, payload.x2_pct, payload.y2_pct)
    context.device.swipe(start, end, payload.velocity)


def execute_drag(payload: SwipePayload, context: ExecutionContext) -> None:
    start = Point.from_normalized(
        context.screen_bounds, payload.x1_pct, payload.y1_pct)
    end = Point.from_normalized(
        context.screen_bounds, payload.x2_pct, payload.y2_pct)
    context.device.drag(start, end, payload.velocity)


def execute_fling(payload: FlingPayload, context: ExecutionContext) -> None:
    start = Point.from_normalized(
        context.screen_bounds, payload.x1_pct, payload.y1_pct)
    end = Point.from_normalized(
        context.screen_bounds, payload.x2_pct, payload.y2_pct)
    context.device.fling(start, end, payload.velocity, payload.step_length)


def execute_directional_fling(payload: DirectionalFlingPayload, context: ExecutionContext) -> None:
    context.device.directional_fling(
        payload.direction, payload.velocity, payload.step_length)


def execute_input_text(payload: InputTextPayload, context: ExecutionContext) -> None:
    context.device.input_text(
        Point.from_normalized(context.screen_bounds,
                              payload.x_pct, payload.y_pct),
        payload.text,
    )


def execute_text(text: str, context: ExecutionContext) -> None:
    context.device.text(text)
