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

# These white-box tests intentionally exercise record's internal parsers.
# pylint: disable=protected-access

import contextlib
import io
import signal
import typing
import unittest
from unittest import mock
from test.mock.hdc import FakeHdc

import record
from src.hdc import Hdc, HdcResult
from src.schema import DoubleTapCommand, FlingCommand, Flow, KeyCommand, TapCommand


_RECORDER_READY = (
    "windowBounds : (0,0,100,100)\n"
    "Started Recording Successfully...\n"
)
_SUPPORTED_RECORD = (
    '{"EVENT_TYPE":"pointer","OP_TYPE":"click",'
    '"BUNDLE":"com.example","ABILITY":"EntryAbility",'
    '"finger_list":[{"X_POSI":"10","Y_POSI":"20"}]}\n'
)


class RecordTest(unittest.TestCase):
    def test_bounds_parsing_and_normalization(self) -> None:
        bounds = record._parse_bounds_from_line(
            "windowBounds : (0,0,1280,2832)")

        self.assertEqual(bounds, record.Bounds(
            left=0, top=0, right=1280, bottom=2832))
        self.assertEqual(record._normalize(640, 0, 1280), 50)
        self.assertEqual(record._normalize(-1, 0, 1280), 0)
        self.assertEqual(record._normalize(3000, 0, 2832), 100)

    def test_pointer_events_convert_and_normalize_values(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            app_flows = record._convert_events(
                self._events([
                    {
                        "EVENT_TYPE": "pointer",
                        "OP_TYPE": "click",
                        "BUNDLE": "com.example.app",
                        "ABILITY": "EntryAbility",
                        "finger_list": [{"X_POSI": "640", "Y_POSI": "1416"}],
                    },
                    {
                        "EVENT_TYPE": "pointer",
                        "OP_TYPE": "fling",
                        "BUNDLE": "com.example.app",
                        "ABILITY": "EntryAbility",
                        "VELO": "117.4",
                        "LENGTH": "0",
                        "finger_list": [{"X_POSI": "0", "Y_POSI": "2832", "X2_POSI": "1280", "Y2_POSI": "0"}],
                    },
                    {"EVENT_TYPE": "pointer", "OP_TYPE": "pinch"},
                ]),
                record.Bounds(left=0, top=0, right=1280, bottom=2832),
            )

        self.assertEqual(len(app_flows), 1)
        commands = app_flows[0].commands
        tap_command = commands[0]
        fling_command = commands[1]
        self.assertIsInstance(tap_command, TapCommand)
        assert isinstance(tap_command, TapCommand)
        self.assertEqual(tap_command.payload.x_pct, 50)
        self.assertEqual(tap_command.payload.y_pct, 50)
        self.assertIsInstance(fling_command, FlingCommand)
        assert isinstance(fling_command, FlingCommand)
        self.assertEqual(fling_command.payload.velocity, 200)
        self.assertEqual(fling_command.payload.step_length, 1)
        self.assertEqual(fling_command.payload.x1_pct, 0)
        self.assertEqual(fling_command.payload.y1_pct, 100)
        self.assertEqual(fling_command.payload.x2_pct, 100)
        self.assertEqual(fling_command.payload.y2_pct, 0)
        self.assertNotIn("pinch", app_flows[0].label)

    def test_key_events_convert_and_unknown_keys_warn(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            app_flows = record._convert_events(
                self._events([
                    {"EVENT_TYPE": "key", "key_code_1": "2057",
                        "BUNDLE": "a", "ABILITY": "b"},
                    {"EVENT_TYPE": "key", "key_code_1": "123",
                        "BUNDLE": "a", "ABILITY": "b"},
                ]),
                record.Bounds(left=0, top=0, right=100, bottom=100),
            )

        self.assertEqual(len(app_flows), 1)
        key_command = app_flows[0].commands[0]
        self.assertIsInstance(key_command, KeyCommand)
        assert isinstance(key_command, KeyCommand)
        self.assertEqual(key_command.payload.key, "Back")

    def test_identity_segments_preserve_order_and_empty_identity(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            app_flows = record._convert_events(
                self._events([
                    self._click("com.a", "A", 1, 1),
                    self._click("com.b", "B", 2, 2),
                    self._click("com.a", "A", 3, 3),
                    self._click("", "", 4, 4),
                ]),
                record.Bounds(left=0, top=0, right=100, bottom=100),
            )

        self.assertEqual([app.bundle for app in app_flows],
                         ["com.a", "com.b", "com.a", ""])
        self.assertEqual([app.ability for app in app_flows],
                         ["A", "B", "A", ""])
        self.assertEqual([app.label for app in app_flows], [
                         "com_a-A-0", "com_b-B-0", "com_a-A-1", "--0"])
        self.assertEqual([app.terminate for app in app_flows], [
                         False, False, False, False])
        first_command = app_flows[0].commands[0]
        empty_identity_command = app_flows[3].commands[0]
        assert isinstance(first_command, TapCommand)
        assert isinstance(empty_identity_command, TapCommand)
        self.assertEqual(first_command.payload.x_pct, 1)
        self.assertEqual(empty_identity_command.payload.x_pct, 4)
        Flow(flow=app_flows, desc="Recorded")

    def test_malformed_supported_input_fails(self) -> None:
        with self.assertRaises(ValueError):
            record._convert_events(
                self._events(
                    [{"EVENT_TYPE": "pointer", "OP_TYPE": "click", "finger_list": [{}]}]),
                record.Bounds(left=0, top=0, right=100, bottom=100),
            )

    def test_no_supported_commands_produces_empty_flow_list(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            app_flows = record._convert_events(
                self._events([{"EVENT_TYPE": "pointer", "OP_TYPE": "pinch"}]),
                record.Bounds(left=0, top=0, right=100, bottom=100),
            )

        self.assertEqual(app_flows, [])

    def test_parse_record_csv_preserves_empty_identity(self) -> None:
        parsed = record._parse_record_csv(
            "{\"EVENT_TYPE\":\"pointer\",\"OP_TYPE\":\"pinch\",\"BUNDLE\":\"\",\"ABILITY\":\"\"}\n"
        )

        self.assertEqual(parsed[0].BUNDLE, "")
        self.assertEqual(parsed[0].ABILITY, "")

    def test_parse_record_csv_rejects_malformed_json(self) -> None:
        with self.assertRaises(ValueError):
            record._parse_record_csv("not-json\n")

    def test_parse_record_csv_rejects_unknown_event_type(self) -> None:
        with self.assertRaises(ValueError):
            record._parse_record_csv(
                "{\"EVENT_TYPE\":\"unsupported\",\"OP_TYPE\":\"click\"}\n"
            )

    def test_parser_rejects_invalid_timeout(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                record.build_parser().parse_args(["--timeout", "0"])

    def test_record_device_inputs_readiness_timeout_fails(self) -> None:
        fake_process = _FakeProcess(io.StringIO(""))
        original_startup_timeout = record._STARTUP_TIMEOUT_S
        record._STARTUP_TIMEOUT_S = 0.01
        try:
            with self.assertRaisesRegex(RuntimeError, "readiness"):
                record.convert_ui_inputs_into_flow(
                    typing.cast(Hdc, FakeHdc(start_shell_process=fake_process)), timeout=1)
        finally:
            record._STARTUP_TIMEOUT_S = original_startup_timeout

        self.assertEqual(fake_process.signals, [signal.SIGINT])

    def test_record_device_inputs_no_supported_commands_fails(self) -> None:
        fake_process = _FakeProcess(io.StringIO(
            "windowBounds : (0,0,100,100)\nCurrent ForAbility :com.example, EntryAbility\nStarted Recording Successfully...\n"))
        fake_hdc = FakeHdc(
            default_result=HdcResult(
                0, "{\"EVENT_TYPE\":\"pointer\",\"OP_TYPE\":\"pinch\"}\n", ""),
            start_shell_process=fake_process,
        )
        stdout = io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaisesRegex(RuntimeError, "no supported"):
                record.convert_ui_inputs_into_flow(
                    typing.cast(Hdc, fake_hdc), timeout=1)

        self.assertNotIn("windowBounds", stdout.getvalue())
        self.assertNotIn("Current ForAbility", stdout.getvalue())
        self.assertNotIn("Started Recording Successfully", stdout.getvalue())

    def test_unexpected_recorder_exit_rejects_parseable_csv(self) -> None:
        fake_process = _FakeProcess(
            io.StringIO(_RECORDER_READY),
            poll_results=[23],
            stderr="recorder crashed\n",
        )
        fake_hdc = FakeHdc(
            default_result=HdcResult(0, _SUPPORTED_RECORD, ""),
            start_shell_process=fake_process,
        )

        with self.assertRaisesRegex(
                RuntimeError, r"returncode=23, stderr=recorder crashed"):
            record.convert_ui_inputs_into_flow(
                typing.cast(Hdc, fake_hdc), timeout=1)

        self.assertEqual(fake_process.signals, [])
        self.assertNotIn(
            ["shell", "cat", "/data/local/tmp/record.csv"], fake_hdc.calls)

    def test_unexpected_clean_recorder_exit_is_failure(self) -> None:
        fake_process = _FakeProcess(
            io.StringIO(_RECORDER_READY), poll_results=[0])
        fake_hdc = FakeHdc(
            default_result=HdcResult(0, _SUPPORTED_RECORD, ""),
            start_shell_process=fake_process,
        )

        with self.assertRaisesRegex(
                RuntimeError, r"returncode=0, stderr=<no stderr>"):
            record.convert_ui_inputs_into_flow(
                typing.cast(Hdc, fake_hdc), timeout=1)

        self.assertNotIn(
            ["shell", "cat", "/data/local/tmp/record.csv"], fake_hdc.calls)

    def test_exit_before_timeout_signal_is_failure(self) -> None:
        fake_process = _FakeProcess(
            io.StringIO(_RECORDER_READY),
            poll_results=[None, 17],
            stderr="late failure",
        )
        fake_hdc = FakeHdc(
            default_result=HdcResult(0, _SUPPORTED_RECORD, ""),
            start_shell_process=fake_process,
        )

        with self.assertRaisesRegex(RuntimeError, r"returncode=17"):
            record.convert_ui_inputs_into_flow(
                typing.cast(Hdc, fake_hdc), timeout=0)

        self.assertEqual(fake_process.signals, [])
        self.assertNotIn(
            ["shell", "cat", "/data/local/tmp/record.csv"], fake_hdc.calls)

    def test_timeout_stop_sends_sigint_and_accepts_signal_exit(self) -> None:
        fake_process = _FakeProcess(
            io.StringIO(_RECORDER_READY),
            poll_results=[None, None, -signal.SIGINT],
        )
        fake_hdc = FakeHdc(
            default_result=HdcResult(0, _SUPPORTED_RECORD, ""),
            start_shell_process=fake_process,
        )

        flow = record.convert_ui_inputs_into_flow(
            typing.cast(Hdc, fake_hdc), timeout=0)

        self.assertEqual(len(flow.flow[0].commands), 1)
        self.assertEqual(fake_process.signals, [signal.SIGINT])
        self.assertIn(
            ["shell", "cat", "/data/local/tmp/record.csv"], fake_hdc.calls)

    def test_manual_stop_sends_sigint_and_accepts_signal_exit(self) -> None:
        fake_process = _FakeProcess(
            io.StringIO(_RECORDER_READY),
            poll_results=[None, -signal.SIGINT],
        )
        fake_hdc = FakeHdc(
            default_result=HdcResult(0, _SUPPORTED_RECORD, ""),
            start_shell_process=fake_process,
        )

        with mock.patch.object(
                record, "_wait_for_recording_stop", return_value="manual"):
            flow = record.convert_ui_inputs_into_flow(
                typing.cast(Hdc, fake_hdc), timeout=None)

        self.assertEqual(len(flow.flow[0].commands), 1)
        self.assertEqual(fake_process.signals, [signal.SIGINT])
        self.assertIn(
            ["shell", "cat", "/data/local/tmp/record.csv"], fake_hdc.calls)

    def test_generated_flow_contains_valid_double_tap(self) -> None:
        app_flows = record._convert_events(
            self._events(
                [self._double_click("com.example", "Ability", 50, 50)]),
            record.Bounds(left=0, top=0, right=100, bottom=100),
        )

        flow = Flow(flow=app_flows, desc="Recorded")
        command = flow.flow[0].commands[0]
        self.assertIsInstance(command, DoubleTapCommand)
        assert isinstance(command, DoubleTapCommand)
        self.assertEqual(command.payload.x_pct, 50)

    def _events(self, rows: list[dict[str, object]]) -> list[record.UIEvent]:
        return [record.UIEvent.model_validate(row) for row in rows]

    def _click(self, bundle: str, ability: str, x: int, y: int) -> dict[str, object]:
        return {
            "EVENT_TYPE": "pointer",
            "OP_TYPE": "click",
            "BUNDLE": bundle,
            "ABILITY": ability,
            "finger_list": [{"X_POSI": str(x), "Y_POSI": str(y)}],
        }

    def _double_click(self, bundle: str, ability: str, x: int, y: int) -> dict[str, object]:
        return {
            "EVENT_TYPE": "pointer",
            "OP_TYPE": "doubleClick",
            "BUNDLE": bundle,
            "ABILITY": ability,
            "finger_list": [{"X_POSI": str(x), "Y_POSI": str(y)}],
        }


class _FakeProcess:
    def __init__(
        self,
        stdout: io.StringIO,
        poll_results: list[int | None] | None = None,
        stderr: str = "",
    ) -> None:
        self.stdout = stdout
        self.stderr = io.StringIO(stderr)
        self.signals: list[int] = []
        self.poll_results = poll_results or [None]
        self.communicated_stderr = stderr

    def poll(self) -> int | None:
        if len(self.poll_results) > 1:
            return self.poll_results.pop(0)
        return self.poll_results[0]

    def send_signal(self, signal_number: int) -> None:
        self.signals.append(signal_number)

    def communicate(self, timeout: float | None = None) -> tuple[str, str]:
        return ("", self.communicated_stderr)

    def terminate(self) -> None:
        return None

    def kill(self) -> None:
        return None


if __name__ == "__main__":
    unittest.main()
