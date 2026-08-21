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
import unittest
from test.mock.device import FakeDevice

from src.commands import ExecutionContext, execute_command
from src.device import ScreenBounds
from src.metadata import AppMetadata
from src.result import ResultStore
from src.schema import Flow


_OUT_DIR = pathlib.Path.cwd().joinpath("out")


def _local(path: str) -> pathlib.Path:
    return _OUT_DIR.joinpath(path)


def _context(
    device: FakeDevice,
    store: ResultStore,
    apps: list[AppMetadata] | None = None,
) -> ExecutionContext:
    return ExecutionContext(
        device=device,  # type: ignore[arg-type]
        store=store,
        flow=Flow(flow=[]),
        apps=apps or [],
        pending_artifacts=[],
        artifact_metadata={},
        child_processes=[],
        screen_bounds=ScreenBounds(0, 0, 1000, 2000),
    )


class CommandsTest(unittest.TestCase):
    def test_ui_commands_convert_and_delegate(self) -> None:
        device = FakeDevice(screen=ScreenBounds(0, 0, 1000, 2000))
        context = _context(
            device,
            ResultStore(_OUT_DIR, pathlib.PurePosixPath("/remote")),
        )
        commands = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [
                            {"action": "tap", "payload": {
                                "x_pct": 50, "y_pct": 50}},
                            {"action": "double_tap", "payload": {
                                "x_pct": 10, "y_pct": 20}},
                            {"action": "long_tap", "payload": {
                                "x_pct": 30, "y_pct": 40}},
                            {
                                "action": "swipe",
                                "payload": {"x1_pct": 50, "y1_pct": 80, "x2_pct": 50, "y2_pct": 20, "velocity": 800},
                            },
                            {
                                "action": "drag",
                                "payload": {"x1_pct": 10, "y1_pct": 20, "x2_pct": 90, "y2_pct": 80, "velocity": 900},
                            },
                            {
                                "action": "fling",
                                "payload": {
                                    "x1_pct": 10,
                                    "y1_pct": 20,
                                    "x2_pct": 90,
                                    "y2_pct": 80,
                                    "velocity": 1000,
                                    "step_length": 20,
                                },
                            },
                            {
                                "action": "directional_fling",
                                "payload": {"direction": "up", "velocity": 1100, "step_length": 30},
                            },
                            {"action": "input_text", "payload": {
                                "x_pct": 40, "y_pct": 60, "text": "hello"}},
                            {"action": "text", "payload": "world"},
                            {"action": "key", "payload": {"key": "Home"}},
                        ],
                    }
                ]
            }
        ).flow[0].commands

        for command in commands:
            execute_command(command, context)

    def test_snapshot_records_metadata_and_mirrored_artifact_paths(self) -> None:
        device = FakeDevice(
            processes={"com.example": 123, "com.example.dead": 456})
        context = _context(
            device,
            ResultStore(_OUT_DIR, pathlib.PurePosixPath("/remote")),
            apps=[
                AppMetadata(
                    pid=123,
                    label="App",
                    bundle="com.example",
                    ability="EntryAbility",
                ),
                AppMetadata(
                    pid=456,
                    label="DeadApp",
                    bundle="com.example.dead",
                    ability="EntryAbility",
                ),
            ],
        )
        command = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "snapshot", "payload": "after_start"}],
                    }
                ]
            }
        ).flow[0].commands[0]

        execute_command(command, context)

        remote_path = pathlib.PurePosixPath(
            "/remote/snapshots/after_start/App-after_start.smaps")
        self.assertEqual(len(context.pending_artifacts), 2)
        self.assertEqual(
            len(context.artifact_metadata[_local("snapshots")]), 1)
        metadata = context.artifact_metadata[_local("snapshots")][0]
        self.assertEqual(metadata.label, "after_start")
        self.assertTrue(metadata.timestamp.isdigit())
        self.assertEqual(context.pending_artifacts[0].remote_base,
                         pathlib.PurePosixPath("/remote/snapshots"))
        self.assertEqual(context.pending_artifacts[0].local_base,
                         _local("snapshots"))
        self.assertEqual(context.pending_artifacts[0].artifact,
                         ["after_start", "App-after_start.smaps"])
        self.assertEqual(context.pending_artifacts[1].artifact,
                         ["after_start", "DeadApp-after_start.smaps"])
        self.assertIn(pathlib.PurePosixPath(
            "/remote/snapshots/after_start"), device.dirs)
        self.assertIn(remote_path, device.files)
        self.assertIn(pathlib.PurePosixPath(
            "/remote/snapshots/after_start/DeadApp-after_start.smaps"),
            device.files)

    def test_screenshot_records_metadata_and_artifact_path(self) -> None:
        device = FakeDevice(processes={"com.example": 123}, dirs={
                            pathlib.PurePosixPath("/remote/screenshots")})
        context = _context(
            device,
            ResultStore(_OUT_DIR, pathlib.PurePosixPath("/remote")),
        )
        command = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "screenshot", "payload": "after_start"}],
                    }
                ]
            }
        ).flow[0].commands[0]

        execute_command(command, context)

        self.assertEqual(len(context.pending_artifacts), 1)
        metadata = context.artifact_metadata[_local(
            "screenshots")][0]
        self.assertEqual(metadata.label, "after_start")
        self.assertTrue(metadata.timestamp.isdigit())
        self.assertEqual(context.pending_artifacts[0].remote_base,
                         pathlib.PurePosixPath("/remote/screenshots"))
        self.assertEqual(context.pending_artifacts[0].local_base,
                         _local("screenshots"))
        self.assertEqual(context.pending_artifacts[0].artifact,
                         ["after_start.png"])
        self.assertIn(pathlib.PurePosixPath(
            "/remote/screenshots/after_start.png"), device.files)

    def test_screenshot_failure_reports_label(self) -> None:
        device = FakeDevice(fail_capture_screenshot=True)
        context = _context(
            device,
            ResultStore(_OUT_DIR, pathlib.PurePosixPath("/remote")),
        )
        command = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "screenshot", "payload": "after_start"}],
                    }
                ]
            }
        ).flow[0].commands[0]

        with self.assertRaisesRegex(RuntimeError, "after_start"):
            execute_command(command, context)

        self.assertEqual(context.artifact_metadata, {})
        self.assertEqual(context.pending_artifacts, [])

    def test_snapshot_failure_does_not_record_metadata(self) -> None:
        device = FakeDevice(
            processes={"com.example": 123}, fail_capture_smaps=True)
        context = _context(
            device,
            ResultStore(_OUT_DIR, pathlib.PurePosixPath("/remote")),
            apps=[AppMetadata(
                pid=123,
                label="App",
                bundle="com.example",
                ability="EntryAbility",
            )],
        )
        command = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "snapshot", "payload": "after_start"}],
                    }
                ]
            }
        ).flow[0].commands[0]

        with self.assertRaisesRegex(RuntimeError, "123"):
            execute_command(command, context)

        self.assertEqual(context.artifact_metadata, {})
        self.assertEqual(context.pending_artifacts, [])


if __name__ == "__main__":
    unittest.main()
