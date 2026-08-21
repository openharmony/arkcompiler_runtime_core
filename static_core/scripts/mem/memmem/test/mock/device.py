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

import pathlib
import subprocess
import sys
import time
from typing import Any

from src.device import DeviceHealth, Point, ScreenBounds, ThermalZoneHealth
from src.hdc import HdcResult


class FakeDevice:
    def __init__(
        self,
        screen: ScreenBounds | None = None,
        processes: dict[str, int] | None = None,
        dirs: set[pathlib.PurePosixPath] | None = None,
        files: dict[pathlib.PurePosixPath, str] | None = None,
        invalid_bundles: set[str] | None = None,
        screen_error: Exception | None = None,
        health: DeviceHealth | None = None,
        health_error: Exception | None = None,
        fail_make_dir: bool = False,
        fail_capture_smaps: bool = False,
        fail_capture_screenshot: bool = False,
        fail_remove_dir: bool = False,
        fail_recv_file: bool = False,
        fail_recv_paths: set[pathlib.PurePosixPath] | None = None,
        fail_recv_names: set[str] | None = None,
        fail_reboot: bool = False,
        fail_wait_available: bool = False,
        fail_wait_boot_completed: bool = False,
        fail_wakeup: bool = False,
        fail_disable_screen_timeout: bool = False,
        fail_configure_hilog: bool = False,
        fail_hilog: bool = False,
        fail_terminate_app: bool = False,
        default_smaps_content: str = "Size: 1 kB\nRss: 1 kB\nPss: 1 kB\n",
        default_log_content: str = "fake log\n",
        default_screenshot_content: str = "fake png\n",
    ) -> None:
        self.screen = screen or ScreenBounds(0, 0, 100, 200)
        self.processes = processes or {}
        self.dirs = dirs or set()
        self.files = files or {}
        self.invalid_bundles = invalid_bundles or set()
        self.screen_error = screen_error
        self.health = health or DeviceHealth(
            battery_capacity_pct=100,
            thermal_zones=[ThermalZoneHealth("soc_thermal", 30000)],
        )
        self.health_error = health_error
        self.fail_make_dir = fail_make_dir
        self.fail_capture_smaps = fail_capture_smaps
        self.fail_capture_screenshot = fail_capture_screenshot
        self.fail_remove_dir = fail_remove_dir
        self.fail_recv_file = fail_recv_file
        self.fail_recv_paths = fail_recv_paths or set()
        self.fail_recv_names = fail_recv_names or set()
        self.fail_reboot = fail_reboot
        self.fail_wait_available = fail_wait_available
        self.fail_wait_boot_completed = fail_wait_boot_completed
        self.fail_wakeup = fail_wakeup
        self.fail_disable_screen_timeout = fail_disable_screen_timeout
        self.fail_configure_hilog = fail_configure_hilog
        self.fail_hilog = fail_hilog
        self.fail_terminate_app = fail_terminate_app
        self.terminated_bundles: list[str] = []
        self.default_smaps_content = default_smaps_content
        self.default_log_content = default_log_content
        self.default_screenshot_content = default_screenshot_content
        self.rebooted = False
        self.waited_available = False
        self.waited_boot_completed = False
        self.woken = False
        self.screen_timeout_disabled = False
        self.hilog_configured = False
        self.hilog_paths: list[pathlib.PurePosixPath] = []
        self.hilog_processes: list[subprocess.Popen[Any]] = []

    def screen_bounds(self) -> ScreenBounds:
        if self.screen_error is not None:
            raise self.screen_error
        return self.screen

    def make_dir(self, path: pathlib.PurePosixPath) -> bool:
        if self.fail_make_dir:
            return False
        self.dirs.add(path)
        return True

    def launch_app(self, bundle: str, _ability: str) -> None:
        if bundle in self.invalid_bundles:
            return
        if bundle not in self.processes:
            self.processes[bundle] = 1000 + len(self.processes)

    def resolve_pid(self, bundle: str) -> int:
        if bundle not in self.processes:
            raise RuntimeError(f"could not resolve PID for bundle {bundle}")
        return self.processes[bundle]

    def terminate_app(self, bundle: str) -> None:
        if self.fail_terminate_app:
            raise RuntimeError("app termination failed")
        self.terminated_bundles.append(bundle)
        self.processes.pop(bundle, None)

    def reboot(self) -> None:
        if self.fail_reboot:
            raise RuntimeError("reboot failed")
        self.rebooted = True

    def wait_available(self, _timeout_seconds: int) -> None:
        if self.fail_wait_available:
            raise RuntimeError("wait available failed")
        self.waited_available = True

    def wait_boot_completed(self, _timeout_seconds: int, _poll_interval_seconds: int = 1) -> None:
        if self.fail_wait_boot_completed:
            raise RuntimeError("boot wait failed")
        self.waited_boot_completed = True

    def wakeup(self) -> None:
        if self.fail_wakeup:
            raise RuntimeError("wakeup failed")
        self.woken = True

    def device_health(self) -> DeviceHealth:
        if self.health_error is not None:
            raise self.health_error
        return self.health

    def disable_screen_timeout(self) -> None:
        if self.fail_disable_screen_timeout:
            raise RuntimeError("screen timeout setup failed")
        self.screen_timeout_disabled = True

    def configure_hilog(self) -> None:
        if self.fail_configure_hilog:
            raise RuntimeError("hilog setup failed")
        self.hilog_configured = True

    def start_hilog(self, path: pathlib.PurePosixPath) -> subprocess.Popen[Any]:
        if self.fail_hilog:
            raise RuntimeError("hilog failed")
        if path.parent not in self.dirs:
            raise RuntimeError("missing remote log directory")
        self.hilog_paths.append(path)
        self.files[path] = self.default_log_content
        process = subprocess.Popen(
            [sys.executable, "-c", "import time; time.sleep(3600)"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        self.hilog_processes.append(process)
        return process

    def timestamp(self) -> str:
        return str(time.time_ns())

    def pid_exists(self, pid: int) -> bool:
        return pid in self.processes.values()

    def capture_smaps(self, pid: int, path: pathlib.PurePosixPath) -> bool:
        if self.fail_capture_smaps:
            return False
        if path.parent not in self.dirs:
            return False
        if not self.pid_exists(pid):
            return False
        self.files[path] = self.default_smaps_content
        return True

    def capture_screenshot(self, path: pathlib.PurePosixPath) -> bool:
        if self.fail_capture_screenshot:
            return False
        if path.parent not in self.dirs:
            return False
        self.files[path] = self.default_screenshot_content
        return True

    def recv_file(self, remote_path: pathlib.PurePosixPath, local_path: pathlib.Path) -> HdcResult:
        if self.fail_recv_file or remote_path in self.fail_recv_paths or remote_path.name in self.fail_recv_names:
            return HdcResult(1, "", "missing remote file")
        if remote_path.name == "hilog.log" and remote_path not in self.files:
            self.files[remote_path] = self.default_log_content
        if remote_path not in self.files:
            return HdcResult(1, "", "missing remote file")
        local_path.parent.mkdir(parents=True, exist_ok=True)
        local_path.write_text(self.files[remote_path], encoding="utf-8")
        return HdcResult(0, "", "")

    def remove_dir(self, path: pathlib.PurePosixPath) -> bool:
        if self.fail_remove_dir:
            return False
        self.dirs = {
            directory for directory in self.dirs if not _is_relative_to(directory, path)}
        self.files = {file_path: content for file_path, content in self.files.items(
        ) if not _is_relative_to(file_path, path)}
        return True

    def send_file(self, local_path: pathlib.Path, remote_path: pathlib.PurePosixPath) -> HdcResult:
        if remote_path.parent not in self.dirs:
            return HdcResult(1, "", "missing remote directory")
        self.files[remote_path] = local_path.read_text(encoding="utf-8")
        return HdcResult(0, "", "")

    def tap(self, _point: Point) -> None:
        pass

    def double_tap(self, _point: Point) -> None:
        pass

    def long_tap(self, _point: Point) -> None:
        pass

    def swipe(self, _start: Point, _end: Point, _velocity: int) -> None:
        pass

    def drag(self, _start: Point, _end: Point, _velocity: int) -> None:
        pass

    def fling(self, _start: Point, _end: Point, _velocity: int, _step_length: int) -> None:
        pass

    def directional_fling(self, _direction: str, _velocity: int, _step_length: int) -> None:
        pass

    def input_text(self, _point: Point, _text: str) -> None:
        pass

    def text(self, _text: str) -> None:
        pass

    def send_key(self, _key: str) -> None:
        pass


def _is_relative_to(path: pathlib.PurePosixPath, parent: pathlib.PurePosixPath) -> bool:
    return path == parent or parent in path.parents
