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
import shlex
import subprocess
import unittest
from unittest import mock

from test.mock.hdc import FakeHdc

from src.device import Device, Point
from src.hdc import Hdc, HdcResult, TIMEOUT_ERR_CODE


class HdcTest(unittest.TestCase):
    def setUp(self) -> None:
        self.hdc_path = pathlib.Path("/opt/hdc")
        self.completed = subprocess.CompletedProcess(
            args=[], returncode=0, stdout="", stderr="")

    def test_shell_quotes_each_remote_argument(self) -> None:
        payloads = [
            "hello world",
            "don't split this",
            "value; touch /data/local/tmp/injected",
            "left && right",
            "$(touch /data/local/tmp/injected)",
        ]

        with mock.patch("src.hdc.subprocess.run", return_value=self.completed) as run:
            for payload in payloads:
                with self.subTest(payload=payload):
                    Hdc(self.hdc_path).shell(
                        "uitest", "uiInput", "text", payload)

                    host_command = run.call_args.args[0]
                    self.assertEqual(
                        host_command[:2], [str(self.hdc_path), "shell"])
                    self.assertEqual(len(host_command), 3)
                    self.assertEqual(
                        shlex.split(host_command[2]),
                        ["uitest", "uiInput", "text", payload],
                    )
                    run.reset_mock()

    def test_shell_without_arguments_preserves_interactive_invocation(self) -> None:
        with mock.patch("src.hdc.subprocess.run", return_value=self.completed) as run:
            Hdc(self.hdc_path).shell(timeout=3)

        self.assertEqual(
            run.call_args.args[0], [str(self.hdc_path), "shell"])
        self.assertEqual(run.call_args.kwargs["timeout"], 3)

    def test_shell_raw_preserves_deliberate_shell_syntax(self) -> None:
        command = "cat /proc/42/smaps > '/data/local/tmp/out file.smaps'"

        with mock.patch("src.hdc.subprocess.run", return_value=self.completed) as run:
            Hdc(self.hdc_path).shell_raw(command, timeout=4)

        self.assertEqual(
            run.call_args.args[0], [str(self.hdc_path), "shell", command])
        self.assertEqual(run.call_args.kwargs["timeout"], 4)

    def test_shell_timeout_keeps_hdc_result_contract(self) -> None:
        with mock.patch(
            "src.hdc.subprocess.run",
            side_effect=subprocess.TimeoutExpired(cmd="hdc", timeout=1),
        ):
            result = Hdc(self.hdc_path).shell("date", timeout=1)

        self.assertEqual(result.returncode, TIMEOUT_ERR_CODE)
        self.assertEqual(result.stdout, "")
        self.assertIn("memmem: TIMEOUT", result.stderr)

    def test_device_flow_fields_cross_shell_as_single_arguments(self) -> None:
        bundle = "com.example; echo bundle-injected"
        ability = "Entry Ability && echo ability-injected"
        input_text = "input 'quoted'; $(echo input-injected)"
        text = "focused text && echo text-injected"

        with mock.patch("src.hdc.subprocess.run", return_value=self.completed) as run:
            device = Device(Hdc(self.hdc_path))
            device.launch_app(bundle, ability)
            device.input_text(Point(13, 14), input_text)
            device.text(text)

        remote_commands = [call.args[0][2]
                           for call in run.call_args_list]
        self.assertEqual(
            [shlex.split(command) for command in remote_commands],
            [
                ["aa", "start", "-a", ability, "-b", bundle],
                ["uitest", "uiInput", "inputText", "13", "14", input_text],
                ["uitest", "uiInput", "text", text],
            ],
        )

    def test_device_raw_redirect_quotes_dynamic_remote_path(self) -> None:
        remote_path = pathlib.PurePosixPath(
            "/data/local/tmp/snapshot dir/result;ignored.smaps")

        with mock.patch("src.hdc.subprocess.run", return_value=self.completed) as run:
            self.assertTrue(Device(Hdc(self.hdc_path)).capture_smaps(
                42, remote_path))

        host_command = run.call_args.args[0]
        self.assertEqual(host_command[:2], [str(self.hdc_path), "shell"])
        self.assertEqual(
            shlex.split(host_command[2]),
            ["cat", "/proc/42/smaps", ">", str(remote_path)],
        )

    def test_start_hilog_quotes_dynamic_remote_path(self) -> None:
        remote_path = pathlib.PurePosixPath(
            "/data/local/tmp/hilog dir/hilog;ignored.log")

        with mock.patch("src.hdc.subprocess.Popen") as popen:
            Device(Hdc(self.hdc_path)).start_hilog(remote_path)

        host_command = popen.call_args.args[0]
        self.assertEqual(host_command[:2], [str(self.hdc_path), "shell"])
        self.assertEqual(
            shlex.split(host_command[2]),
            ["hilog", ">", str(remote_path)],
        )

    def test_fake_hdc_shell_raw_uses_normal_result_and_timeout_semantics(self) -> None:
        command = "echo ready && echo done"
        expected = HdcResult(0, "ready\ndone\n", "")
        hdc = FakeHdc(
            responses={f"shell {command}": expected},
            timeout_after=5,
        )

        self.assertEqual(hdc.shell_raw(command, timeout=5), expected)
        self.assertEqual(
            hdc.shell_raw(command, timeout=4).returncode, TIMEOUT_ERR_CODE)
        self.assertEqual(
            hdc.calls,
            [["shell", command], ["shell", command]],
        )


if __name__ == "__main__":
    unittest.main()
