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

import argparse
import dataclasses
from datetime import datetime
import json
import pathlib
import re
import shlex
import signal
import subprocess
import sys
import threading
import time
import typing

import pydantic

from src.schema import (
    AppFlow,
    Command,
    DoubleTapCommand,
    DragCommand,
    FlingCommand,
    FlingPayload,
    Flow,
    KeyCommand,
    LongTapCommand,
    NamedKeyPayload,
    SwipeCommand,
    SwipePayload,
    TapCommand,
    TapPayload,
)
from run import load_config
from src.hdc import Hdc
from src.types import positive_int

_STARTUP_TIMEOUT_S = 120.0

_EVENT_TYPE_POINTER = "pointer"
_EVENT_TYPE_KEY = "key"

_OP_TAP = {
    "click": "tap",
    "longClick": "long_tap",
    "doubleClick": "double_tap",
}
_KEYCODE_MAP: dict[int, str] = {
    2057: "Back",
    2066: "Home",
    2074: "Power",
}

_BOUNDS_RE = re.compile(
    r"windowBounds\s*:\s*\((\d+),(\d+),(\d+),(\d+)\)")

_NON_LABEL_RE = re.compile(r"[^A-Za-z0-9_-]")


@dataclasses.dataclass(frozen=True)
class Bounds:
    left: int
    top: int
    right: int
    bottom: int

    @property
    def width(self) -> int:
        return self.right - self.left

    @property
    def height(self) -> int:
        return self.bottom - self.top


@dataclasses.dataclass(frozen=True)
class _RecorderStopResult:
    returncode: int | None
    stderr: str
    sigint_sent: bool


@dataclasses.dataclass(frozen=True)
class AppInfo:
    bundle: str
    ability: str


class FingerEvent(pydantic.BaseModel):
    X_POSI: str | None = None
    Y_POSI: str | None = None
    X2_POSI: str | None = None
    Y2_POSI: str | None = None
    LENGTH: str | None = None
    MAX_VEL: str | None = None
    VELO: str | None = None
    direction_x: str | None = pydantic.Field(
        default=None, validation_alias="direction.X")
    direction_y: str | None = pydantic.Field(
        default=None, validation_alias="direction.Y")


class UIEvent(pydantic.BaseModel):
    model_config = pydantic.ConfigDict(validate_by_name=True, extra="ignore")

    ABILITY: str = ""
    BUNDLE: str = ""
    CENTER_X: str = ""
    CENTER_Y: str = ""
    EVENT_TYPE: typing.Literal["key", "pointer"]
    FILEPATH: str = pydantic.Field(
        default="", validation_alias=pydantic.AliasChoices("FILEPATH", "FILEPAHT"))
    LENGTH: str = ""
    OP_TYPE: str = ""
    VELO: str = ""
    direction_x: str = pydantic.Field(
        default="", validation_alias="direction.X")
    direction_y: str = pydantic.Field(
        default="", validation_alias="direction.Y")
    duration: float | str | None = None
    finger_list: list[FingerEvent] = pydantic.Field(default_factory=list)
    key_code_1: str | None = None


def _parse_bounds_from_line(line: str) -> Bounds | None:
    match = _BOUNDS_RE.search(line)
    if match is None:
        return None
    return Bounds(
        left=int(match.group(1)),
        top=int(match.group(2)),
        right=int(match.group(3)),
        bottom=int(match.group(4)),
    )


def _normalize(raw: int, start: int, span: int) -> int:
    return max(0, min(100, round((raw - start) * 100 / span)))


def _first_finger(raw: UIEvent) -> FingerEvent:
    try:
        return raw.finger_list[0]
    except IndexError as exc:
        raise ValueError(
            f"malformed supported pointer record missing finger_list: {raw}") from exc


def _handle_pointer_home_back(raw: UIEvent, bounds: Bounds) -> Command:
    del bounds
    key_str = "Home" if raw.OP_TYPE == "home" else "Back"
    return KeyCommand(action="key", payload=NamedKeyPayload(
        key=typing.cast(typing.Literal["Home", "Back", "Power"], key_str)))


def _handle_pointer_tap(raw: UIEvent, bounds: Bounds) -> Command:
    first_finger = _first_finger(raw)
    try:
        x_finger = int(first_finger.X_POSI or "")
        y_finger = int(first_finger.Y_POSI or "")
    except (ValueError, TypeError) as exc:
        raise ValueError(f"malformed tap record: {raw}") from exc

    x_pct = _normalize(x_finger, bounds.left, bounds.width)
    y_pct = _normalize(y_finger, bounds.top, bounds.height)

    action = _OP_TAP[raw.OP_TYPE]
    if action == "tap":
        return TapCommand(action="tap", payload=TapPayload(x_pct=x_pct, y_pct=y_pct))
    if action == "double_tap":
        return DoubleTapCommand(action="double_tap", payload=TapPayload(x_pct=x_pct, y_pct=y_pct))
    return LongTapCommand(action="long_tap", payload=TapPayload(x_pct=x_pct, y_pct=y_pct))


def _handle_pointer_swipe(raw: UIEvent, bounds: Bounds) -> Command:
    first_finger = _first_finger(raw)
    try:
        x1 = int(first_finger.X_POSI or "")
        y1 = int(first_finger.Y_POSI or "")
        x2 = int(first_finger.X2_POSI or first_finger.X_POSI or "")
        y2 = int(first_finger.Y2_POSI or first_finger.Y_POSI or "")
    except (ValueError, TypeError) as exc:
        raise ValueError(f"malformed swipe record: {raw}") from exc

    vel_raw = raw.VELO or 0
    try:
        velocity = round(float(vel_raw))
    except (ValueError, TypeError):
        velocity = 0
    velocity = max(200, min(40000, velocity))

    x1_pct = _normalize(x1, bounds.left, bounds.width)
    y1_pct = _normalize(y1, bounds.top, bounds.height)
    x2_pct = _normalize(x2, bounds.left, bounds.width)
    y2_pct = _normalize(y2, bounds.top, bounds.height)

    if raw.OP_TYPE == "swipe":
        return SwipeCommand(action="swipe", payload=SwipePayload(
            x1_pct=x1_pct, y1_pct=y1_pct, x2_pct=x2_pct, y2_pct=y2_pct, velocity=velocity))
    if raw.OP_TYPE == "drag":
        return DragCommand(action="drag", payload=SwipePayload(
            x1_pct=x1_pct, y1_pct=y1_pct, x2_pct=x2_pct, y2_pct=y2_pct, velocity=velocity))

    length_raw = raw.LENGTH or 0
    try:
        step_length = max(1, round(float(length_raw)))
    except (ValueError, TypeError):
        step_length = 1
    return FlingCommand(action="fling", payload=FlingPayload(
        x1_pct=x1_pct, y1_pct=y1_pct, x2_pct=x2_pct, y2_pct=y2_pct, velocity=velocity, step_length=step_length))


def _handle_key(raw: UIEvent) -> Command | None:
    try:
        key_code = int(raw.key_code_1 or "")
    except (ValueError, TypeError) as exc:
        raise ValueError(f"malformed key record: {raw}") from exc

    key_str = _KEYCODE_MAP.get(key_code)
    if key_str is None:
        return None
    return KeyCommand(action="key", payload=NamedKeyPayload(
        key=typing.cast(typing.Literal["Home", "Back", "Power"], key_str)))


_POINTER_HANDLERS: dict[str, typing.Callable[[UIEvent, Bounds], Command]] = {
    "click": _handle_pointer_tap,
    "longClick": _handle_pointer_tap,
    "doubleClick": _handle_pointer_tap,
    "swipe": _handle_pointer_swipe,
    "drag": _handle_pointer_swipe,
    "fling": _handle_pointer_swipe,
    "home": _handle_pointer_home_back,
    "back": _handle_pointer_home_back,
}


def _convert_event(raw: UIEvent, bounds: Bounds) -> Command | None:
    if raw.EVENT_TYPE == _EVENT_TYPE_POINTER:
        handler = _POINTER_HANDLERS.get(raw.OP_TYPE)
        if handler is None:
            print(
                f"  warning: skipping unsupported pointer OP_TYPE: {raw.OP_TYPE}", file=sys.stderr)
            return None
        return handler(raw, bounds)

    if raw.EVENT_TYPE == _EVENT_TYPE_KEY:
        command = _handle_key(raw)
        if command is None:
            print(
                f"  warning: skipping unknown key code {raw.key_code_1}; add manually if needed", file=sys.stderr)
        return command

    raise AssertionError("unreachable EVENT_TYPE")


def _identity_key(raw: UIEvent) -> AppInfo:
    return AppInfo(bundle=raw.BUNDLE or "", ability=raw.ABILITY or "")


def _append_app_flow(
    app_flows: list[AppFlow],
    identity_counts: dict[AppInfo, int],
    app_info: AppInfo,
    commands: list[Command],
) -> None:
    count = identity_counts.get(app_info, 0)
    identity_counts[app_info] = count + 1
    label = _NON_LABEL_RE.sub(
        "_", f"{app_info.bundle}-{app_info.ability}-{count}")
    if not app_info.bundle or not app_info.ability:
        print(
            "  warning: generated AppFlow has empty bundle or ability; edit them before benchmarking", file=sys.stderr)
    app_flows.append(AppFlow(label=label, bundle=app_info.bundle,
                             ability=app_info.ability, terminate=False, commands=commands))


def _convert_events(
    records: list[UIEvent],
    bounds: Bounds,
) -> list[AppFlow]:
    identity_counts: dict[AppInfo, int] = {}
    app_flows: list[AppFlow] = []
    current_identity: AppInfo | None = None
    current_commands: list[Command] = []

    for raw in records:
        command = _convert_event(raw, bounds)
        if command is None:
            continue

        identity = _identity_key(raw)
        if current_identity is None:
            current_identity = identity
        elif identity != current_identity:
            _append_app_flow(app_flows, identity_counts,
                             current_identity, current_commands)
            current_commands = []
            current_identity = identity
        current_commands.append(command)

    if current_identity is not None and current_commands:
        _append_app_flow(app_flows, identity_counts,
                         current_identity, current_commands)
    return app_flows


def _parse_record_csv(content: str) -> list[UIEvent]:
    records: list[UIEvent] = []
    for line in content.splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        try:
            event = UIEvent.model_validate_json(stripped)
        except pydantic.ValidationError as exc:
            raise ValueError(
                f"malformed record row, cannot parse: {line}") from exc
        records.append(event)
    return records


def _stop_recorder(
    process: subprocess.Popen[typing.Any],
    *,
    request_sigint: bool = True,
) -> _RecorderStopResult:
    sigint_sent = False
    if request_sigint and process.poll() is None:
        try:
            process.send_signal(signal.SIGINT)
            sigint_sent = True
        except ProcessLookupError:
            pass

    stderr = ""
    try:
        _, stderr = process.communicate(timeout=10)
    except subprocess.TimeoutExpired:
        process.terminate()
        try:
            _, stderr = process.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            _, stderr = process.communicate()
    return _RecorderStopResult(
        returncode=process.poll(),
        stderr=stderr or "",
        sigint_sent=sigint_sent,
    )


def _raise_unexpected_recorder_exit(result: _RecorderStopResult) -> typing.NoReturn:
    returncode = "unknown" if result.returncode is None else str(
        result.returncode)
    stderr = result.stderr.strip() or "<no stderr>"
    raise RuntimeError(
        f"recorder exited unexpectedly (returncode={returncode}, stderr={stderr})")


def _wait_for_stdin_stop(stop_event: threading.Event) -> None:
    try:
        line = sys.stdin.readline()
        if line.strip().lower() == "y":
            stop_event.set()
    except (EOFError, OSError):
        return


def _read_recorder_startup(process: subprocess.Popen[typing.Any]) -> Bounds:
    ready_event = threading.Event()
    values_lock = threading.Lock()
    bounds: Bounds | None = None

    def _read_stdout() -> None:
        nonlocal bounds
        if process.stdout is None:
            return
        for line in process.stdout:
            if "Started Recording Successfully" in line:
                ready_event.set()
            parsed_bounds = _parse_bounds_from_line(line)
            if parsed_bounds is None:
                continue
            with values_lock:
                if bounds is None:
                    bounds = parsed_bounds

    threading.Thread(target=_read_stdout, daemon=True).start()

    if not ready_event.wait(timeout=_STARTUP_TIMEOUT_S):
        _stop_recorder(process)
        raise RuntimeError(
            "recorder readiness not observed within startup timeout")

    with values_lock:
        parsed_bounds = bounds

    if parsed_bounds is None:
        _stop_recorder(process)
        raise RuntimeError("screen bounds not observed in recorder output")
    if parsed_bounds.width <= 0 or parsed_bounds.height <= 0:
        _stop_recorder(process)
        raise RuntimeError("invalid screen bounds from recorder")
    return parsed_bounds


def _prompt_recording_started(timeout: int | None) -> None:
    print("Recording started. Perform UI inputs now.")
    if timeout is not None:
        print(
            f"Recording will stop after {timeout} seconds or when you press 'y' + Enter.")
    else:
        print("Press 'y' + Enter when finished.")


def _wait_for_recording_stop(
    process: subprocess.Popen[typing.Any],
    timeout: int | None,
) -> str:
    stop_event = threading.Event()
    threading.Thread(target=_wait_for_stdin_stop,
                     args=(stop_event,), daemon=True).start()
    start_time = time.monotonic()

    while True:
        if process.poll() is not None:
            return "process"
        if stop_event.is_set():
            return "manual"
        if timeout is not None and (time.monotonic() - start_time) >= timeout:
            return "timeout"
        time.sleep(0.1)


def _build_flow(app_flows: list[AppFlow]) -> Flow:
    raw_flow = {
        "flow": [app_flow.model_dump(by_alias=True) for app_flow in app_flows],
        "$desc": "Recorded from device UI inputs. Add waits/snapshots/screenshots manually before benchmarking.",
    }
    try:
        return Flow.model_validate(raw_flow)
    except pydantic.ValidationError as exc:
        print(json.dumps(raw_flow))
        raise RuntimeError(
            f"generated flow rejected by schema: {exc}") from exc


def convert_ui_inputs_into_flow(hdc: Hdc, timeout: int | None) -> Flow:
    process = hdc.start_shell(
        shlex.join(("uitest", "uiRecord", "record",
                   "-W", "false", "-c", "true")),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    bounds = _read_recorder_startup(process)

    _prompt_recording_started(timeout)
    stopped_by = _wait_for_recording_stop(process, timeout)

    print(f"Stopping recording (reason={stopped_by})...")
    if stopped_by == "process":
        _raise_unexpected_recorder_exit(
            _stop_recorder(process, request_sigint=False))

    stop_result = _stop_recorder(process)
    if not stop_result.sigint_sent:
        _raise_unexpected_recorder_exit(stop_result)

    cat_result = hdc.shell("cat", "/data/local/tmp/record.csv")
    if cat_result.is_error():
        raise RuntimeError(
            f"failed to read device record.csv: {cat_result.stderr}")

    records = _parse_record_csv(cat_result.stdout)
    app_flows = _convert_events(records, bounds)
    if not app_flows:
        raise RuntimeError("no supported commands recorded")
    return _build_flow(app_flows)


def output_path(now: datetime) -> pathlib.Path:
    return pathlib.Path(f"flow-{now.strftime('%Y%m%d_%H%M%S_%f')}.json")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Record UI inputs from an OpenHarmony device and convert them into a benchmark flow JSON file.")
    parser.add_argument(
        "--timeout",
        type=positive_int,
        default=None,
        help="recording duration in seconds; if omitted, recording runs until manual stop",
    )
    return parser


def main() -> int:
    args = build_parser().parse_args()
    timeout: int | None = args.timeout

    try:
        output = output_path(datetime.now())

        config = load_config(pathlib.Path(".env"))
        hdc = Hdc(config.hdc_path)
        flow = convert_ui_inputs_into_flow(hdc, timeout)

        output.write_text(json.dumps(flow.model_dump(
            by_alias=True, exclude_none=True), indent=2), encoding="utf-8")
        print(f"\nFlow saved to {output}")

    except Exception as error:
        print(f"memmem error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
