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

import contextlib
import io
import json
import os
import pathlib
import subprocess
import sys
import tempfile
import typing
import unittest
from unittest import mock

from test.mock.device import FakeDevice
from test.mock.hdc import FakeHdc
from src.device import Device, DeviceHealth, ScreenBounds, ThermalZoneHealth
from src.hdc import HdcResult
from src.log import configure_logger, reset_logger
from src.runner import BenchmarkOptions, run_benchmark, create_result_store, _stop_process, _MAX_THERMAL_TEMP_MILLIS_C, _MIN_BATTERY_CAPACITY_PCT
from src.report import generate_reports
from src.schema import Flow


DEFAULT_FLOW = '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[]}]}'


def _options(out_dir: pathlib.Path, reboot: bool = False, hilog: bool = True) -> BenchmarkOptions:
    return BenchmarkOptions(out_dir=out_dir, reboot=reboot, hilog=hilog)


def _load_flow(content: str) -> Flow:
    return Flow.model_validate(json.loads(content))


def _write_flow(_root: pathlib.Path, content: str = DEFAULT_FLOW) -> Flow:
    return _load_flow(content)


def _sleep_process() -> subprocess.Popen[bytes]:
    return subprocess.Popen(
        [sys.executable, "-c", "import time; time.sleep(3600)"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def _stubborn_process() -> subprocess.Popen[bytes]:
    process = _sleep_process()
    setattr(process, "terminate", lambda: None)
    return process


class _RecvCapturingDevice(FakeDevice):
    def __init__(self, **kwargs: typing.Any) -> None:
        super().__init__(**kwargs)
        self.recv_local_paths: list[pathlib.Path] = []

    def recv_file(
        self,
        remote_path: pathlib.PurePosixPath,
        local_path: pathlib.Path,
    ) -> HdcResult:
        self.recv_local_paths.append(local_path)
        return super().recv_file(remote_path, local_path)


class _EarlyExitHilogDevice(FakeDevice):
    def __init__(self, exit_code: int = 0, **kwargs: typing.Any) -> None:
        super().__init__(**kwargs)
        self.exit_code = exit_code

    def start_hilog(self, path: pathlib.PurePosixPath) -> subprocess.Popen[typing.Any]:
        if path.parent not in self.dirs:
            raise RuntimeError("missing remote log directory")
        self.hilog_paths.append(path)
        self.files[path] = self.default_log_content
        process = subprocess.Popen(
            [sys.executable, "-c", f"raise SystemExit({self.exit_code})"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        process.wait(5)
        self.hilog_processes.append(process)
        return process


class RunnerTest(unittest.TestCase):
    def test_result_store_remote_name_is_host_timestamped(self) -> None:
        store = create_result_store(
            _options(pathlib.Path("/tmp/memmem out/iteration:0")))

        self.assertRegex(
            str(store.remote_out_dir()),
            r"^/data/local/tmp/memmem-out-\d{8}_\d{6}_\d{6}$")

    def test_runner_restores_working_directory(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            original_cwd = pathlib.Path.cwd()
            try:
                options = _options(root.joinpath("out"))
                store = create_result_store(options)
                run_benchmark(_write_flow(root), options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

                self.assertEqual(pathlib.Path.cwd(), original_cwd)
            finally:
                os.chdir(original_cwd)

    def test_runner_restores_working_directory_on_failure(self) -> None:
        device = FakeDevice(
            screen=ScreenBounds(0, 0, 100, 200),
            invalid_bundles={"com.example"},
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            original_cwd = pathlib.Path.cwd()
            try:
                options = _options(root.joinpath("out"))
                store = create_result_store(options)
                with self.assertRaises(RuntimeError):
                    run_benchmark(_write_flow(root), options, store,
                                  device)  # type: ignore[arg-type]

                self.assertEqual(pathlib.Path.cwd(), original_cwd)
            finally:
                os.chdir(original_cwd)

    def test_existing_output_skips_all_finalizers_and_device_calls(self) -> None:
        device = mock.create_autospec(Device, instance=True)
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            out_dir.mkdir()
            metadata = out_dir.joinpath("app_metadata.json")
            metadata.write_bytes(b"SENTINEL\n")
            options = _options(out_dir, hilog=False)
            store = create_result_store(options)

            with self.assertRaises(FileExistsError):
                run_benchmark(_write_flow(root), options, store, device)

            self.assertEqual(metadata.read_bytes(), b"SENTINEL\n")
            self.assertEqual(device.mock_calls, [])

    def test_failure_before_remote_creation_skips_remote_cleanup(self) -> None:
        wrapped_device = FakeDevice(
            health_error=RuntimeError("health failed"))
        device = mock.Mock(wraps=wrapped_device, spec=FakeDevice)
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            options = _options(out_dir, hilog=False)
            store = create_result_store(options)

            with self.assertRaisesRegex(RuntimeError, "health failed"):
                run_benchmark(
                    _write_flow(root),
                    options,
                    store,
                    typing.cast(Device, device),
                )

            self.assertTrue(out_dir.joinpath("app_metadata.json").is_file())
            device.remove_dir.assert_not_called()

    def test_partial_remote_initialization_cleans_created_root(self) -> None:
        device = mock.create_autospec(Device, instance=True)
        device.device_health.return_value = DeviceHealth(
            battery_capacity_pct=100,
            thermal_zones=[ThermalZoneHealth("soc_thermal", 30000)],
        )
        device.make_dir.side_effect = [True, False]
        device.remove_dir.return_value = True
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            options = _options(out_dir, hilog=False)
            store = create_result_store(options)

            with self.assertRaisesRegex(
                    RuntimeError, "failed to create remote snapshots directory"):
                run_benchmark(_write_flow(root), options, store, device)

            self.assertTrue(out_dir.joinpath("app_metadata.json").is_file())
            device.remove_dir.assert_called_once_with(store.remote_out_dir())
            device.recv_file.assert_not_called()
            device.screen_bounds.assert_not_called()

    def test_runner_passes_relative_local_paths_to_recv(self) -> None:
        device = _RecvCapturingDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            original_cwd = pathlib.Path.cwd()
            try:
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(_write_flow(root), options, store,
                              device)  # type: ignore[arg-type]

                self.assertTrue(device.recv_local_paths)
                for path in device.recv_local_paths:
                    self.assertFalse(path.is_absolute())
                    self.assertNotIn("..", str(path))
                    self.assertTrue(out_dir.joinpath(path).is_file())
            finally:
                os.chdir(original_cwd)

    def test_hilog_process_stop_terminates_running_process(self) -> None:
        process = _sleep_process()

        _stop_process(process)

        self.assertIsNotNone(process.poll())

    def test_hilog_process_stop_uses_kill_fallback(self) -> None:
        process = _stubborn_process()

        _stop_process(process, timeout_seconds=0.01)

        self.assertIsNotNone(process.poll())

    def test_hilog_process_reports_early_failure(self) -> None:
        process = subprocess.Popen(
            [sys.executable, "-c", "import sys; sys.exit(1)"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        process.wait(5)

        with self.assertRaisesRegex(RuntimeError, "hilog stream exited"):
            _stop_process(process)

    def test_hilog_process_reports_early_clean_exit_without_terminating_again(self) -> None:
        process = mock.create_autospec(subprocess.Popen, instance=True)
        process.poll.return_value = 0

        with self.assertRaisesRegex(
            RuntimeError,
            "exited before requested shutdown with host code 0",
        ):
            _stop_process(process)

        process.terminate.assert_not_called()

    def test_early_clean_hilog_exit_fails_after_preserving_partial_artifact(self) -> None:
        partial_hilog = "partial hilog before early exit\n"
        device = _EarlyExitHilogDevice(
            screen=ScreenBounds(0, 0, 100, 200),
            default_log_content=partial_hilog,
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            options = _options(out_dir)
            store = create_result_store(options)

            with self.assertRaisesRegex(
                RuntimeError,
                "exited before requested shutdown with host code 0",
            ):
                run_benchmark(
                    _write_flow(root),
                    options,
                    store,
                    device,  # type: ignore[arg-type]
                )

            self.assertEqual(
                out_dir.joinpath("hilog", "hilog.log").read_text(
                    encoding="utf-8"),
                partial_hilog,
            )
            self.assertTrue(out_dir.joinpath("app_metadata.json").is_file())
            self.assertFalse(device.files)
            self.assertFalse(device.dirs)

    def test_flow_failure_remains_primary_when_hilog_exits_cleanly_early(self) -> None:
        device = _EarlyExitHilogDevice(
            screen=ScreenBounds(0, 0, 100, 200),
            invalid_bundles={"com.example"},
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            options = _options(out_dir)
            store = create_result_store(options)

            with self.assertRaisesRegex(
                RuntimeError,
                "could not resolve PID for bundle com.example",
            ) as raised:
                run_benchmark(
                    _write_flow(root),
                    options,
                    store,
                    device,  # type: ignore[arg-type]
                )

            self.assertIn(
                "additionally failed post_flow step: hilog stream exited "
                "before requested shutdown with host code 0",
                str(raised.exception),
            )
            self.assertTrue(out_dir.joinpath("hilog", "hilog.log").is_file())
            self.assertFalse(device.files)
            self.assertFalse(device.dirs)

    def test_invalid_flow_fails_before_device_actions(self) -> None:
        device = FakeDevice(health_error=RuntimeError("health failed"))

        with self.assertRaises(Exception):
            _load_flow('{"flow": "invalid"}')

        self.assertFalse(device.screen_timeout_disabled)
        self.assertFalse(device.hilog_configured)
        self.assertEqual(device.processes, {})

    def test_runner_uses_options_and_writes_metadata(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            options = _options(out_dir)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            self.assertEqual(device.processes, {"com.example": 1000})
            self.assertTrue(device.screen_timeout_disabled)
            self.assertTrue(device.hilog_configured)
            self.assertEqual(len(device.hilog_processes), 1)
            self.assertIsNotNone(device.hilog_processes[0].poll())
            self.assertTrue(out_dir.joinpath("app_metadata.json").is_file())
            self.assertTrue(out_dir.joinpath("hilog", "hilog.log").is_file())
            self.assertEqual(
                list(out_dir.joinpath("breakdowns").iterdir()), [])
            self.assertTrue(out_dir.joinpath("summary.csv").is_file())

    def test_info_logs_transitions_and_command_execution_with_text_payload(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        stdout = io.StringIO()
        with contextlib.redirect_stdout(stdout):
            configure_logger("info")
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow = _load_flow(
                '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[{"action":"text","payload":"secret"}]}]}'
            )
            out_dir = root.joinpath("out")

            options = _options(out_dir, hilog=False)
            store = create_result_store(options)
            run_benchmark(flow, options, store, typing.cast(Device, device))

        content = stdout.getvalue()
        self.assertIn("info: start benchmark pre flow", content)
        self.assertIn("info: start device verification", content)
        self.assertIn("info: start benchmark flow", content)
        self.assertIn("info: start application pre flow", content)
        self.assertIn("info: start application flow", content)
        self.assertIn("info: executing command: app=app action=text", content)
        self.assertIn("info: start benchmark post flow", content)
        self.assertIn("secret", content)
        reset_logger()

    def test_reboot_option_prepares_device_before_flow(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            options = _options(out_dir, reboot=True)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            self.assertTrue(device.rebooted)
            self.assertTrue(device.waited_available)
            self.assertTrue(device.waited_boot_completed)
            self.assertTrue(device.woken)
            self.assertTrue(device.screen_timeout_disabled)

    def test_no_reboot_skips_wakeup_routine(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            options = _options(out_dir, reboot=False)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            self.assertFalse(device.rebooted)
            self.assertFalse(device.waited_available)
            self.assertFalse(device.waited_boot_completed)
            self.assertFalse(device.woken)
            self.assertTrue(device.screen_timeout_disabled)

    def test_low_battery_rejects_startup_before_app_flows(self) -> None:
        device = FakeDevice(
            health=DeviceHealth(
                battery_capacity_pct=_MIN_BATTERY_CAPACITY_PCT - 1,
                thermal_zones=[ThermalZoneHealth(
                    "soc_thermal", _MAX_THERMAL_TEMP_MILLIS_C - 1)],
            )
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "battery too low"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]

            self.assertEqual(device.processes, {})
            self.assertTrue(device.screen_timeout_disabled)
            self.assertTrue(device.hilog_configured)

    def test_hot_thermal_zone_rejects_startup_before_app_flows(self) -> None:
        device = FakeDevice(
            health=DeviceHealth(
                battery_capacity_pct=_MIN_BATTERY_CAPACITY_PCT,
                thermal_zones=[ThermalZoneHealth(
                    "soc_thermal", _MAX_THERMAL_TEMP_MILLIS_C + 1)],
            )
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "device too hot"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]

            self.assertEqual(device.processes, {})
            self.assertTrue(device.screen_timeout_disabled)
            self.assertTrue(device.hilog_configured)

    def test_zero_thermal_zone_does_not_reject_startup(self) -> None:
        device = FakeDevice(
            health=DeviceHealth(
                battery_capacity_pct=_MIN_BATTERY_CAPACITY_PCT,
                thermal_zones=[ThermalZoneHealth("inactive", 0)],
            )
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            options = _options(out_dir)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            self.assertEqual(device.processes, {"com.example": 1000})
            self.assertTrue(device.screen_timeout_disabled)

    def test_health_failure_rejects_startup_before_app_flows(self) -> None:
        device = FakeDevice(health_error=RuntimeError("health failed"))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "health failed"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]

            self.assertEqual(device.processes, {})
            self.assertTrue(device.screen_timeout_disabled)
            self.assertTrue(device.hilog_configured)

    def test_health_verification_runs_after_prepare_device(self) -> None:
        hdc = FakeHdc(
            responses={
                "shell param get bootevent.boot.completed": HdcResult(0, "true\n", ""),
                "shell cat /sys/class/power_supply/Battery/capacity": HdcResult(0, "80\n", ""),
                "shell for z in /sys/class/thermal/thermal_zone*; do [ -f $z/type ] && [ -f $z/temp ] && printf '%s %s\\n' \"$(cat $z/type)\" \"$(cat $z/temp)\"; done": HdcResult(0, "soc_thermal 30000\n", ""),
            },
            prefix_responses={
                "shell uitest dumpLayout -p ": HdcResult(0, "DumpLayout saved", ""),
                "shell cat /data/local/tmp/memmem-layout.json": HdcResult(0, '{"attributes": {"bounds": "[0,0][100,200]"}}', ""),
            },
        )
        device = Device(hdc)  # type: ignore[arg-type]
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root, '{"flow":[]}')
            out_dir = root.joinpath("out")

            options = _options(out_dir, reboot=True, hilog=False)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store, device)
            generate_reports(store)

            self.assertEqual(
                hdc.calls[:15],
                [
                    ["target", "boot"],
                    ["wait"],
                    ["shell", "param", "get", "bootevent.boot.completed"],
                    ["shell", "power-shell", "wakeup"],
                    ["shell", "uitest", "uiInput", "dircFling", "2", "20000", "20"],
                    ["shell", "uitest", "uiInput", "keyEvent", "Back"],
                    ["shell", "power-shell", "timeout", "-o", "60000000"],
                    ["shell", "cat", "/sys/class/power_supply/Battery/capacity"],
                    [
                        "shell",
                        "for z in /sys/class/thermal/thermal_zone*; do [ -f $z/type ] && [ -f $z/temp ] && printf '%s %s\\n' \"$(cat $z/type)\" \"$(cat $z/temp)\"; done",
                    ],
                    ["shell", "mkdir", "-p", str(store.remote_out_dir())],
                    ["shell", "mkdir", "-p",
                        str(store.remote_snapshots_dir())],
                    ["shell", "mkdir", "-p",
                        str(store.remote_screenshots_dir())],
                    ['shell', 'uitest', 'dumpLayout', '-p',
                        '/data/local/tmp/memmem-layout.json'],
                    ['shell', 'cat', '/data/local/tmp/memmem-layout.json'],
                    ['shell', 'rm', '-f',
                        '/data/local/tmp/memmem-layout.json'],
                ],
            )

    def test_hilog_can_be_disabled(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            options = _options(out_dir, hilog=False)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            self.assertTrue(device.screen_timeout_disabled)
            self.assertFalse(device.hilog_configured)
            self.assertEqual(
                list(out_dir.joinpath("breakdowns").iterdir()), [])
            self.assertFalse(out_dir.joinpath("hilog").exists())

    def test_repeated_bundle_flow_writes_metadata_and_mirrored_artifacts(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _load_flow(
                """
                {
                  "flow": [
                    {"label":"app_1","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[{"action":"snapshot","payload":"before1"}]},
                    {"label":"app_2","bundle":"com.other","ability":"EntryAbility","terminate":false,"commands":[{"action":"snapshot","payload":"before2"}]},
                    {"label":"app_1_again","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[]}
                  ]
                }
                """
            )
            out_dir = root.joinpath("out")

            options = _options(out_dir)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            app_metadata = out_dir.joinpath(
                "app_metadata.json").read_text(encoding="utf-8")
            snapshot_metadata = out_dir.joinpath(
                "snapshots", "metadata.json").read_text(encoding="utf-8")
            self.assertEqual(device.processes, {
                             "com.example": 1000, "com.other": 1001})
            self.assertIn('"label": "app_1_again"', app_metadata)
            self.assertIn('"pid": 1000', app_metadata)
            self.assertIn('"label": "before1"', snapshot_metadata)
            expected_local_paths = {
                out_dir.joinpath(
                    "snapshots", "before1", "app_1-before1.smaps"),
                out_dir.joinpath(
                    "snapshots", "before2", "app_1-before2.smaps"),
                out_dir.joinpath(
                    "snapshots", "before2", "app_2-before2.smaps"),
            }
            for path in expected_local_paths:
                self.assertTrue(path.is_file())
            self.assertTrue(out_dir.joinpath("hilog", "hilog.log").is_file())
            self.assertFalse(device.files)
            self.assertFalse(device.dirs)

    def test_invalid_bundle_collects_hilog_and_preserves_failure(self) -> None:
        device = FakeDevice(screen=ScreenBounds(
            0, 0, 100, 200), invalid_bundles={"com.invalid.bundle"})
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.invalid.bundle","ability":"EntryAbility","terminate":false,"commands":[]}]}',
            )
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "could not resolve PID"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

            self.assertEqual(device.processes, {})
            self.assertTrue(out_dir.joinpath("hilog", "hilog.log").is_file())

    def test_log_receive_failure_fails_benchmark(self) -> None:
        device = FakeDevice(screen=ScreenBounds(
            0, 0, 100, 200), fail_recv_file=True)
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "missing remote file"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

            self.assertTrue(out_dir.joinpath("app_metadata.json").is_file())
            self.assertFalse(device.files)
            self.assertFalse(device.dirs)

    def test_receive_attempts_hilog_when_snapshot_receive_fails(self) -> None:
        device = FakeDevice(screen=ScreenBounds(
            0, 0, 100, 200),
            fail_recv_names={"app-snap.smaps"})
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[{"action":"snapshot","payload":"snap"}]}]}',
            )
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "missing remote file"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

            self.assertTrue(out_dir.joinpath("hilog", "hilog.log").is_file())
            self.assertFalse(device.files)
            self.assertFalse(device.dirs)

    def test_requested_termination_removes_process_after_hilog(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":true,"commands":[]}]}',
            )
            out_dir = root.joinpath("out")

            options = _options(out_dir)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]

            self.assertEqual(device.terminated_bundles, ["com.example"])
            self.assertEqual(device.processes, {})
            self.assertTrue(out_dir.joinpath("hilog", "hilog.log").is_file())

    def test_termination_failure_fails_benchmark(self) -> None:
        device = FakeDevice(screen=ScreenBounds(
            0, 0, 100, 200), fail_terminate_app=True)
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":true,"commands":[]}]}',
            )
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "app termination failed"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]

    def test_termination_failure_combines_with_app_failure(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200), invalid_bundles={
                            "com.invalid"}, fail_terminate_app=True)
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.invalid","ability":"EntryAbility","terminate":true,"commands":[]}]}',
            )
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "additionally failed post_app_flow step"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]

    def test_no_termination_when_not_requested(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            options = _options(out_dir)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]

            self.assertEqual(device.terminated_bundles, [])
            self.assertEqual(device.processes, {"com.example": 1000})

    def test_screenshot_artifact_is_received(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 100, 200))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[{"action":"screenshot","payload":"shot"}]}]}',
            )
            out_dir = root.joinpath("out")

            options = _options(out_dir)
            store = create_result_store(options)
            run_benchmark(flow_path, options, store,
                          device)  # type: ignore[arg-type]
            generate_reports(store)

            self.assertTrue(out_dir.joinpath(
                "screenshots", "shot.png").is_file())
            self.assertIn('"label": "shot"', out_dir.joinpath(
                "screenshots", "metadata.json").read_text(encoding="utf-8"))
            self.assertTrue(out_dir.joinpath("summary.csv").is_file())

    def test_receive_attempts_screenshot_when_snapshot_receive_fails(self) -> None:
        device = FakeDevice(screen=ScreenBounds(
            0, 0, 100, 200),
            fail_recv_names={"app-snap.smaps"})
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.example","ability":"EntryAbility","terminate":false,"commands":[{"action":"snapshot","payload":"snap"},{"action":"screenshot","payload":"shot"}]}]}',
            )
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "missing remote file"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

            self.assertTrue(out_dir.joinpath(
                "screenshots", "shot.png").is_file())
            self.assertFalse(device.files)
            self.assertFalse(device.dirs)

    def test_cleanup_failure_combines_with_app_failure(self) -> None:
        device = FakeDevice(
            screen=ScreenBounds(0, 0, 100, 200),
            invalid_bundles={"com.invalid.bundle"},
            fail_remove_dir=True,
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(
                root,
                '{"flow":[{"label":"app","bundle":"com.invalid.bundle","ability":"EntryAbility","terminate":false,"commands":[]}]}',
            )
            out_dir = root.joinpath("out")

            with self.assertRaisesRegex(RuntimeError, "failed to clean up remote run directory"):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

    def test_screen_bounds_failure_prevents_app_launch(self) -> None:
        device = FakeDevice(screen_error=RuntimeError("bounds failed"))
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            flow_path = _write_flow(root)
            out_dir = root.joinpath("out")

            with self.assertRaises(RuntimeError):
                options = _options(out_dir)
                store = create_result_store(options)
                run_benchmark(flow_path, options, store,
                              device)  # type: ignore[arg-type]
                generate_reports(store)

            self.assertTrue(device.screen_timeout_disabled)
            self.assertTrue(device.hilog_configured)
            self.assertEqual(device.processes, {})


if __name__ == "__main__":
    unittest.main()
