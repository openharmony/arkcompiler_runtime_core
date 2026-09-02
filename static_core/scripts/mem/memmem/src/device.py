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
import json
import pathlib
import re
import shlex
import subprocess
import time
import typing

from src.hdc import Hdc, HdcResult


_BOUNDS_RE = re.compile(r"^\[(\d+),(\d+)\]\[(\d+),(\d+)\]$")


def _raise_if_error(result: HdcResult, fallback: str) -> None:
    if result.is_error():
        raise RuntimeError(result.stderr.strip()
                           or result.stdout.strip() or fallback)


@dataclasses.dataclass(frozen=True)
class ScreenBounds:
    left: int
    top: int
    right: int
    bottom: int


@dataclasses.dataclass(frozen=True)
class ThermalZoneHealth:
    name: str
    temp_millis_celsius: int


@dataclasses.dataclass(frozen=True)
class DeviceHealth:
    battery_capacity_pct: int
    thermal_zones: list[ThermalZoneHealth]


@dataclasses.dataclass(frozen=True)
class Point:
    x: int
    y: int

    @staticmethod
    def from_normalized(bounds: ScreenBounds, x_pct: int, y_pct: int) -> "Point":
        width = bounds.right - bounds.left
        height = bounds.bottom - bounds.top
        if width <= 1 or height <= 1:
            raise ValueError("screen bounds must describe a non-empty area")
        x = bounds.left + round(width * x_pct / 100)
        y = bounds.top + round(height * y_pct / 100)
        return Point(
            x=min(max(x, bounds.left + 1), bounds.right - 1),
            y=min(max(y, bounds.top + 1), bounds.bottom - 1),
        )


class Device:
    def __init__(self, hdc: Hdc) -> None:
        self.hdc = hdc

    def launch_app(self, bundle: str, ability: str) -> None:
        result = self.hdc.shell("aa", "start", "-a", ability, "-b", bundle)
        _raise_if_error(result, "app launch failed")

    def terminate_app(self, bundle: str) -> None:
        result = self.hdc.shell("aa", "force-stop", bundle)
        _raise_if_error(result, "app termination failed")
        output = result.stdout.strip()
        if "error:" in output.lower() or "force stop process successfully" not in output:
            raise RuntimeError(output or "app termination failed")

    def reboot(self) -> None:
        result = self.hdc.run("target", "boot")
        _raise_if_error(result, "device reboot failed")

    def wait_available(self, timeout_seconds: int) -> None:
        result = self.hdc.run("wait", timeout=timeout_seconds)
        _raise_if_error(result, "device wait failed")

    def wait_boot_completed(
        self,
        timeout_seconds: int,
        poll_interval_seconds: int = 1,
    ) -> None:
        deadline = time.monotonic() + timeout_seconds
        while time.monotonic() < deadline:
            result = self.hdc.shell("param", "get", "bootevent.boot.completed")
            if not result.is_error() and result.stdout.strip() == "true":
                return
            time.sleep(poll_interval_seconds)
        raise RuntimeError("device boot did not complete before timeout")

    def disable_screen_timeout(self) -> None:
        result = self.hdc.shell("power-shell", "timeout", "-o", "60000000")
        _raise_if_error(result, "screen timeout setup failed")

    def wakeup(self) -> None:
        result = self.hdc.shell("power-shell", "wakeup")
        _raise_if_error(result, "wakeup failed")

    def device_health(self) -> DeviceHealth:
        capacity = self._read_int_path(
            "/sys/class/power_supply/Battery/capacity",
            "battery capacity",
        )
        zones = self._read_thermal_zones()
        if not zones:
            raise RuntimeError("device health failed: no thermal zones found")
        return DeviceHealth(
            battery_capacity_pct=capacity,
            thermal_zones=zones,
        )

    def configure_hilog(self) -> None:
        flow_control = self.hdc.shell("hilog", "-Q", "pidoff")
        _raise_if_error(flow_control, "hilog flow control setup failed")
        privacy = self.hdc.shell("hilog", "-p", "off")
        _raise_if_error(privacy, "hilog privacy setup failed")

    def start_hilog(self, remote_path: pathlib.PurePosixPath) -> subprocess.Popen[typing.Any]:
        return self.hdc.start_shell(
            f"hilog > {shlex.quote(str(remote_path))}")

    def resolve_pid(self, bundle: str) -> int:
        result = self.hdc.shell("pidof", bundle)
        _raise_if_error(result, "pid lookup failed")
        pids = []
        for field in result.stdout.split():
            try:
                pids.append(int(field))
            except ValueError:
                continue
        if len(pids) == 1:
            return pids[0]
        if len(pids) > 1:
            raise RuntimeError(f"multiple PIDs resolved for bundle {bundle}")
        raise RuntimeError(f"could not resolve PID for bundle {bundle}")

    def pid_exists(self, pid: int) -> bool:
        proc_path = pathlib.PurePosixPath("/").joinpath("proc", str(pid))
        result = self.hdc.shell_raw(
            f"[ -d {shlex.quote(str(proc_path))} ] && echo yes || echo no")
        return not result.is_error() and result.stdout.strip() == "yes"

    def timestamp(self) -> str:
        result = self.hdc.shell("date", "+%s%N")
        candidate = result.stdout.strip()
        if not result.is_error() and candidate.isdigit():
            return candidate

        fallback = self.hdc.shell("date", "+%s")
        seconds = fallback.stdout.strip()
        if not fallback.is_error() and seconds.isdigit():
            return f"{seconds}000000000"

        raise RuntimeError(fallback.stderr.strip(
        ) or result.stderr.strip() or "device timestamp failed")

    def make_dir(self, remote_path: pathlib.PurePosixPath) -> bool:
        return not self.hdc.shell("mkdir", "-p", str(remote_path)).is_error()

    def remove_dir(self, remote_path: pathlib.PurePosixPath) -> bool:
        return not self.hdc.shell("rm", "-rf", str(remote_path)).is_error()

    def capture_smaps(self, pid: int, remote_path: pathlib.PurePosixPath) -> bool:
        smaps_path = pathlib.PurePosixPath("/").joinpath(
            "proc",
            str(pid),
            "smaps",
        )
        result = self.hdc.shell_raw(
            f"{shlex.join(('cat', str(smaps_path)))} > "
            f"{shlex.quote(str(remote_path))}")
        return not result.is_error()

    def capture_screenshot(self, remote_path: pathlib.PurePosixPath) -> bool:
        result = self.hdc.shell("uitest", "screenCap", "-p", str(remote_path))
        return not result.is_error()

    def screen_bounds(self) -> ScreenBounds:
        layout = self._dump_layout()
        attributes = layout.get("attributes")
        if not isinstance(attributes, dict):
            raise RuntimeError("layout root attributes are missing")
        bounds = attributes.get("bounds")
        if not isinstance(bounds, str):
            raise RuntimeError("layout root bounds are missing")
        match = _BOUNDS_RE.fullmatch(bounds)
        if match is None:
            raise RuntimeError(f"layout root bounds are malformed: {bounds}")
        left, top, right, bottom = (int(group) for group in match.groups())
        if right <= left or bottom <= top:
            raise RuntimeError(f"layout root bounds are malformed: {bounds}")
        return ScreenBounds(left=left, top=top, right=right, bottom=bottom)

    def tap(self, point: Point) -> None:
        self._run_ui_input("click", str(point.x), str(point.y))

    def double_tap(self, point: Point) -> None:
        self._run_ui_input("doubleClick", str(point.x), str(point.y))

    def long_tap(self, point: Point) -> None:
        self._run_ui_input("longClick", str(point.x), str(point.y))

    def swipe(self, start: Point, end: Point, velocity: int) -> None:
        self._run_ui_input(
            "swipe",
            str(start.x),
            str(start.y),
            str(end.x),
            str(end.y),
            str(velocity),
        )

    def drag(self, start: Point, end: Point, velocity: int) -> None:
        self._run_ui_input(
            "drag",
            str(start.x),
            str(start.y),
            str(end.x),
            str(end.y),
            str(velocity),
        )

    def fling(self, start: Point, end: Point, velocity: int, step_length: int) -> None:
        self._run_ui_input(
            "fling",
            str(start.x),
            str(start.y),
            str(end.x),
            str(end.y),
            str(velocity),
            str(step_length),
        )

    def directional_fling(self, direction: str, velocity: int, step_length: int) -> None:
        direction_id = {"left": "0", "right": "1",
                        "up": "2", "down": "3"}[direction]
        self._run_ui_input("dircFling", direction_id,
                           str(velocity), str(step_length))

    def input_text(self, point: Point, text: str) -> None:
        self._run_ui_input("inputText", str(point.x), str(point.y), text)

    def text(self, text: str) -> None:
        self._run_ui_input("text", text)

    def send_key(self, key: str) -> None:
        self._run_ui_input("keyEvent", key)

    def send_file(
        self,
        local_path: pathlib.Path,
        remote_path: pathlib.PurePosixPath,
    ) -> HdcResult:
        return self.hdc.run("file", "send", str(local_path), str(remote_path))

    def recv_file(
        self,
        remote_path: pathlib.PurePosixPath,
        local_path: pathlib.Path,
    ) -> HdcResult:
        local_path.parent.mkdir(parents=True, exist_ok=True)
        return self.hdc.run("file", "recv", str(remote_path), str(local_path))

    def _read_int_path(self, path: str, label: str) -> int:
        result = self.hdc.shell("cat", path)
        _raise_if_error(result, f"device health failed: {label} read failed")
        value = result.stdout.strip()
        if not value:
            raise RuntimeError(f"device health failed: {label} is empty")
        try:
            return int(value)
        except ValueError as error:
            raise RuntimeError(
                f"device health failed: {label} is not an integer: {value}"
            ) from error

    def _read_thermal_zones(self) -> list[ThermalZoneHealth]:
        result = self.hdc.shell_raw(
            "for z in /sys/class/thermal/thermal_zone*; do "
            "[ -f $z/type ] && [ -f $z/temp ] && "
            "printf '%s %s\\n' \"$(cat $z/type)\" \"$(cat $z/temp)\"; "
            "done"
        )
        _raise_if_error(
            result, "device health failed: thermal zones read failed")
        zones = []
        for line in result.stdout.splitlines():
            fields = line.split()
            if len(fields) != 2:
                raise RuntimeError(
                    f"device health failed: malformed thermal zone: {line}")
            name, temp = fields
            try:
                zones.append(ThermalZoneHealth(
                    name=name, temp_millis_celsius=int(temp)))
            except ValueError as error:
                raise RuntimeError(
                    f"device health failed: thermal zone {name} is not an integer: {temp}"
                ) from error
        return zones

    def _dump_layout(self) -> dict[str, object]:
        remote_path = pathlib.PurePosixPath("/data/local/tmp").joinpath(
            "memmem-layout.json")
        result = self.hdc.shell("uitest", "dumpLayout", "-p", str(remote_path))
        _raise_if_error(result, "layout dump failed")
        try:
            cat = self.hdc.shell("cat", str(remote_path))
            _raise_if_error(cat, "layout read failed")
            parsed = json.loads(cat.stdout)
        finally:
            self.hdc.shell("rm", "-f", str(remote_path))
        if not isinstance(parsed, dict):
            raise RuntimeError("layout root must be a JSON object")
        return parsed

    def _run_ui_input(self, *args: str) -> None:
        result = self.hdc.shell("uitest", "uiInput", *args)
        _raise_if_error(result, f"{args[0]} failed")
