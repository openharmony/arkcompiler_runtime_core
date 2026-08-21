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
import tempfile
import unittest
from test.mock.hdc import FakeHdc

from src.device import Device, Point, ScreenBounds, ThermalZoneHealth
from src.hdc import HdcResult


_LAYOUT_DUMP_PREFIX = "shell uitest dumpLayout -p "
_LAYOUT_CAT_PREFIX = "shell cat /data/local/tmp/memmem-layout.json"
_VALID_LAYOUT = '{"attributes": {"bounds": "[0,0][1280,2832]"}}'
_BAD_LAYOUT = '{"attributes": {"bounds": "bad"}}'


class DeviceTest(unittest.TestCase):
    def test_hdc_result_is_error_checks_returncode_and_stderr(self) -> None:
        self.assertFalse(HdcResult(0, "", "").is_error())
        self.assertTrue(HdcResult(1, "", "").is_error())
        self.assertTrue(HdcResult(0, "", "bad file").is_error())

    def test_point_from_normalized_converts_and_clamps(self) -> None:
        bounds = ScreenBounds(left=0, top=0, right=1280, bottom=2832)

        self.assertEqual(Point.from_normalized(
            bounds, 50, 50), Point(640, 1416))
        self.assertEqual(Point.from_normalized(bounds, 0, 0), Point(1, 1))
        self.assertEqual(Point.from_normalized(
            bounds, 100, 100), Point(1279, 2831))

    def test_point_from_normalized_uses_non_zero_origin(self) -> None:
        bounds = ScreenBounds(left=10, top=20, right=110, bottom=220)

        self.assertEqual(Point.from_normalized(bounds, 50, 50), Point(60, 120))

    def test_screen_bounds_reads_root_layout_bounds(self) -> None:
        hdc = FakeHdc(
            prefix_responses={
                _LAYOUT_DUMP_PREFIX: HdcResult(0, "DumpLayout saved", ""),
                _LAYOUT_CAT_PREFIX: HdcResult(0, _VALID_LAYOUT, ""),
            }
        )
        device = Device(hdc)  # type: ignore[arg-type]

        self.assertEqual(device.screen_bounds(),
                         ScreenBounds(0, 0, 1280, 2832))
        self.assertTrue(
            any(call[:4] == ["shell", "uitest", "dumpLayout", "-p"] for call in hdc.calls))
        self.assertTrue(any(call[:2] == ["shell", "cat"]
                        for call in hdc.calls))

    def test_screen_bounds_rejects_malformed_bounds(self) -> None:
        hdc = FakeHdc(
            prefix_responses={
                _LAYOUT_DUMP_PREFIX: HdcResult(0, "DumpLayout saved", ""),
                _LAYOUT_CAT_PREFIX: HdcResult(0, _BAD_LAYOUT, ""),
            }
        )

        with self.assertRaises(RuntimeError):
            Device(hdc).screen_bounds()  # type: ignore[arg-type]

    def test_resolve_pid_returns_single_pid(self) -> None:
        hdc = FakeHdc(
            responses={"shell pidof com.example": HdcResult(0, "123\n", "")})

        self.assertEqual(Device(hdc).resolve_pid(  # type: ignore[arg-type]
            "com.example"), 123)
        self.assertEqual(hdc.calls[-1], ["shell", "pidof", "com.example"])

    def test_resolve_pid_rejects_zero_pids(self) -> None:
        hdc = FakeHdc(
            responses={"shell pidof com.example": HdcResult(0, "not-a-pid\n", "")})

        with self.assertRaises(RuntimeError):
            Device(hdc).resolve_pid("com.example")  # type: ignore[arg-type]

    def test_resolve_pid_rejects_multiple_pids(self) -> None:
        hdc = FakeHdc(
            responses={"shell pidof com.example": HdcResult(0, "123 456\n", "")})

        with self.assertRaises(RuntimeError):
            Device(hdc).resolve_pid("com.example")  # type: ignore[arg-type]

    def test_pid_exists_reads_explicit_shell_output(self) -> None:
        hdc = FakeHdc(responses={
                      "shell [ -d /proc/123 ] && echo yes || echo no": HdcResult(0, "yes\n", "")})

        self.assertTrue(Device(hdc).pid_exists(123))  # type: ignore[arg-type]
        self.assertEqual(
            hdc.calls[-1], ["shell", "[ -d /proc/123 ] && echo yes || echo no"])

    def test_pid_exists_rejects_no_output(self) -> None:
        hdc = FakeHdc(responses={
                      "shell [ -d /proc/123 ] && echo yes || echo no": HdcResult(0, "no\n", "")})

        self.assertFalse(Device(hdc).pid_exists(123))  # type: ignore[arg-type]

    def test_timestamp_uses_device_time(self) -> None:
        hdc = FakeHdc(
            responses={"shell date +%s%N": HdcResult(0, "1700000000000000000\n", "")})

        self.assertEqual(Device(hdc).timestamp(),  # type: ignore[arg-type]
                         "1700000000000000000")

    def test_terminate_app_uses_force_stop_and_requires_success_marker(self) -> None:
        hdc = FakeHdc(responses={
                      "shell aa force-stop com.example": HdcResult(0, "force stop process successfully.\n", "")})

        Device(hdc).terminate_app("com.example")  # type: ignore[arg-type]

        self.assertEqual(
            hdc.calls, [["shell", "aa", "force-stop", "com.example"]])

    def test_terminate_app_rejects_hdc_error(self) -> None:
        hdc = FakeHdc(
            responses={"shell aa force-stop com.example": HdcResult(1, "", "failed")})

        with self.assertRaisesRegex(RuntimeError, "failed"):
            Device(hdc).terminate_app("com.example")  # type: ignore[arg-type]

    def test_terminate_app_rejects_error_output_with_zero_returncode(self) -> None:
        hdc = FakeHdc(responses={"shell aa force-stop missing.bundle": HdcResult(
            0, "error: failed to force stop process.\n", "")})

        with self.assertRaisesRegex(RuntimeError, "error"):
            Device(hdc).terminate_app(  # type: ignore[arg-type]
                "missing.bundle")

    def test_terminate_app_rejects_missing_success_marker(self) -> None:
        hdc = FakeHdc(
            responses={"shell aa force-stop com.example": HdcResult(0, "done\n", "")})

        with self.assertRaisesRegex(RuntimeError, "done"):
            Device(hdc).terminate_app("com.example")  # type: ignore[arg-type]

    def test_capture_screenshot_uses_uitest_screencap(self) -> None:
        hdc = FakeHdc()

        self.assertTrue(Device(hdc).capture_screenshot(pathlib.PurePosixPath(  # type: ignore[arg-type]
            "/remote/screenshots/shot.png")))
        self.assertEqual(hdc.calls, [
                         ["shell", "uitest", "screenCap", "-p", "/remote/screenshots/shot.png"]])

    def test_capture_screenshot_reports_failure(self) -> None:
        hdc = FakeHdc(responses={
                      "shell uitest screenCap -p /remote/screenshots/shot.png": HdcResult(1, "", "missing dir")})

        self.assertFalse(Device(hdc).capture_screenshot(pathlib.PurePosixPath(  # type: ignore[arg-type]
            "/remote/screenshots/shot.png")))

    def test_reboot_uses_target_boot(self) -> None:
        hdc = FakeHdc()

        Device(hdc).reboot()  # type: ignore[arg-type]

        self.assertEqual(hdc.calls, [["target", "boot"]])

    def test_wait_available_uses_hdc_wait_with_timeout(self) -> None:
        hdc = FakeHdc()

        Device(hdc).wait_available(120)  # type: ignore[arg-type]

        self.assertEqual(hdc.calls, [["wait"]])

    def test_wait_boot_completed_polls_boot_parameter(self) -> None:
        hdc = FakeHdc(responses={
                      "shell param get bootevent.boot.completed": HdcResult(0, "true\n", "")})

        Device(hdc).wait_boot_completed(180)  # type: ignore[arg-type]

        self.assertEqual(
            hdc.calls, [["shell", "param", "get", "bootevent.boot.completed"]])

    def test_disable_screen_timeout_uses_power_shell(self) -> None:
        hdc = FakeHdc()

        Device(hdc).disable_screen_timeout()  # type: ignore[arg-type]

        self.assertEqual(
            hdc.calls, [["shell", "power-shell", "timeout", "-o", "60000000"]])

    def test_wakeup_uses_power_shell(self) -> None:
        hdc = FakeHdc()

        Device(hdc).wakeup()  # type: ignore[arg-type]

        self.assertEqual(hdc.calls, [["shell", "power-shell", "wakeup"]])

    def test_wakeup_reports_failure(self) -> None:
        hdc = FakeHdc(
            responses={"shell power-shell wakeup": HdcResult(1, "", "wake failed")})

        with self.assertRaisesRegex(RuntimeError, "wake failed"):
            Device(hdc).wakeup()  # type: ignore[arg-type]

    def test_device_health_reads_battery_and_thermal_zones(self) -> None:
        thermal_command = (
            "shell for z in /sys/class/thermal/thermal_zone*; do "
            "[ -f $z/type ] && [ -f $z/temp ] && "
            "printf '%s %s\\n' \"$(cat $z/type)\" \"$(cat $z/temp)\"; "
            "done"
        )
        hdc = FakeHdc(
            responses={
                "shell cat /sys/class/power_supply/Battery/capacity": HdcResult(0, "80\n", ""),
                thermal_command: HdcResult(0, "soc_thermal 30000\nshell_back 26000\n", ""),
            }
        )

        health = Device(hdc).device_health()  # type: ignore[arg-type]

        self.assertEqual(health.battery_capacity_pct, 80)
        self.assertEqual(
            health.thermal_zones,
            [
                ThermalZoneHealth("soc_thermal", 30000),
                ThermalZoneHealth("shell_back", 26000),
            ],
        )

    def test_device_health_rejects_missing_battery_capacity(self) -> None:
        hdc = FakeHdc(responses={
                      "shell cat /sys/class/power_supply/Battery/capacity": HdcResult(1, "", "missing")})

        with self.assertRaisesRegex(RuntimeError, "missing"):
            Device(hdc).device_health()  # type: ignore[arg-type]

    def test_device_health_rejects_malformed_battery_capacity(self) -> None:
        hdc = FakeHdc(responses={
                      "shell cat /sys/class/power_supply/Battery/capacity": HdcResult(0, "bad\n", "")})

        with self.assertRaisesRegex(RuntimeError, "battery capacity is not an integer"):
            Device(hdc).device_health()  # type: ignore[arg-type]

    def test_device_health_rejects_malformed_thermal_zone(self) -> None:
        hdc = FakeHdc(
            responses={
                "shell cat /sys/class/power_supply/Battery/capacity": HdcResult(0, "80\n", ""),
                "shell for z in /sys/class/thermal/thermal_zone*; do [ -f $z/type ] && [ -f $z/temp ] && printf '%s %s\\n' \"$(cat $z/type)\" \"$(cat $z/temp)\"; done": HdcResult(0, "bad-zone\n", ""),
            }
        )

        with self.assertRaisesRegex(RuntimeError, "malformed thermal zone"):
            Device(hdc).device_health()  # type: ignore[arg-type]

    def test_configure_hilog_disables_flow_control_and_privacy(self) -> None:
        hdc = FakeHdc()

        Device(hdc).configure_hilog()  # type: ignore[arg-type]

        self.assertEqual(
            hdc.calls, [["shell", "hilog", "-Q", "pidoff"], ["shell", "hilog", "-p", "off"]])

    def test_hilog_streams_to_remote_path(self) -> None:
        hdc = FakeHdc()

        process = Device(hdc).start_hilog(pathlib.PurePosixPath(  # type: ignore[arg-type]
            "/remote/hilog/hilog.log"))
        try:
            self.assertEqual(
                hdc.calls, [["shell", "hilog > /remote/hilog/hilog.log"]])
            self.assertIs(process, hdc.started_processes[0])
        finally:
            process.terminate()
            process.wait(5)

    def test_boot_completed_times_out(self) -> None:
        hdc = FakeHdc(responses={
                      "shell param get bootevent.boot.completed": HdcResult(0, "false\n", "")})

        with self.assertRaises(RuntimeError):
            Device(hdc).wait_boot_completed(0)  # type: ignore[arg-type]

    def test_wait_available_rejects_hdc_timeout(self) -> None:
        hdc = FakeHdc(timeout_after=10)

        with self.assertRaises(RuntimeError):
            Device(hdc).wait_available(1)  # type: ignore[arg-type]

    def test_file_transfer_commands(self) -> None:
        hdc = FakeHdc()
        device = Device(hdc)  # type: ignore[arg-type]
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            local_path = root.joinpath("local.txt")
            receive_path = root.joinpath("out", "file.txt")

            device.send_file(
                local_path, pathlib.PurePosixPath("/remote/file.txt"))
            device.recv_file(pathlib.PurePosixPath(
                "/remote/file.txt"), receive_path)

        self.assertEqual(
            hdc.calls,
            [
                ["file", "send", str(local_path), "/remote/file.txt"],
                ["file", "recv", "/remote/file.txt", str(receive_path)],
            ],
        )

    def test_ui_input_commands(self) -> None:
        hdc = FakeHdc()
        device = Device(hdc)  # type: ignore[arg-type]

        device.tap(Point(1, 2))
        device.double_tap(Point(3, 4))
        device.long_tap(Point(5, 6))
        device.swipe(Point(1, 2), Point(3, 4), 800)
        device.drag(Point(5, 6), Point(7, 8), 900)
        device.fling(Point(9, 10), Point(11, 12), 1000, 20)
        device.directional_fling("up", 1100, 30)
        device.input_text(Point(13, 14), "hello")
        device.text("world")
        device.send_key("Home")

        self.assertEqual(
            hdc.calls,
            [
                ["shell", "uitest", "uiInput", "click", "1", "2"],
                ["shell", "uitest", "uiInput", "doubleClick", "3", "4"],
                ["shell", "uitest", "uiInput", "longClick", "5", "6"],
                ["shell", "uitest", "uiInput", "swipe", "1", "2", "3", "4", "800"],
                ["shell", "uitest", "uiInput", "drag", "5", "6", "7", "8", "900"],
                ["shell", "uitest", "uiInput", "fling",
                    "9", "10", "11", "12", "1000", "20"],
                ["shell", "uitest", "uiInput", "dircFling", "2", "1100", "30"],
                ["shell", "uitest", "uiInput", "inputText", "13", "14", "hello"],
                ["shell", "uitest", "uiInput", "text", "world"],
                ["shell", "uitest", "uiInput", "keyEvent", "Home"],
            ],
        )


if __name__ == "__main__":
    unittest.main()
