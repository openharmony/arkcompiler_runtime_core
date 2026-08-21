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
import csv
import io
import json
import os
import pathlib
import re
import tempfile
import unittest
from unittest import mock
from test.mock.device import FakeDevice

import pydantic

import lib


def _snapshot_tree(root: pathlib.Path) -> list[tuple[str, str, bytes | None]]:
    snapshot: list[tuple[str, str, bytes | None]] = []
    for path in sorted(root.rglob("*")):
        relative = str(path.relative_to(root))
        if path.is_dir():
            snapshot.append((relative, "directory", None))
        else:
            snapshot.append((relative, "file", path.read_bytes()))
    return snapshot


class LibTest(unittest.TestCase):
    def tearDown(self) -> None:
        lib.reset_logger()

    def test_builders_construct_valid_flow(self) -> None:
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=True,
                commands=[
                    lib.wait(1),
                    lib.snapshot("start"),
                    lib.screenshot("visual"),
                    lib.key("Home"),
                    lib.tap(50, 60),
                    lib.double_tap(50, 60),
                    lib.long_tap(50, 60),
                    lib.swipe(50, 80, 50, 20, 800),
                    lib.drag(50, 80, 50, 20, 800),
                    lib.fling(50, 80, 50, 20, 800, 20),
                    lib.directional_fling("down", 800, 20),
                    lib.input_text(50, 60, "hello"),
                    lib.text("hello"),
                ],
            )
        ])

        self.assertEqual(len(scenario.flow), 1)
        self.assertEqual(len(scenario.flow[0].commands), 13)

    def test_flow_builder_accepts_description(self) -> None:
        scenario = lib.flow([], desc="Reviewer note")

        self.assertEqual(scenario.desc, "Reviewer note")
        self.assertEqual(scenario.model_dump(), {
                         "$desc": "Reviewer note", "flow": []})

    def test_builder_validation_failure_is_raised(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            lib.snapshot("not valid")
        with self.assertRaises(pydantic.ValidationError):
            lib.tap(-1, 50)
        with self.assertRaises(pydantic.ValidationError):
            lib.directional_fling("north", 800, 20)  # type: ignore[arg-type]

    def test_public_facade_exports_helpers_and_types(self) -> None:
        expected_types = {
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
        }
        for name in expected_types:
            self.assertIn(name, lib.__all__)
            self.assertTrue(hasattr(lib, name))
        self.assertIn("app_flow", lib.__all__)
        self.assertIn("configure_logger", lib.__all__)
        self.assertIn("get_logger", lib.__all__)
        self.assertIn("preprocess_flow", lib.__all__)
        self.assertTrue(hasattr(lib, "preprocess_flow"))
        for name in (
            "unprocessed_flow",
            "unprocessed_app_flow",
            "unprocessed_snapshot",
            "unprocessed_screenshot",
            "repeat",
        ):
            self.assertIn(name, lib.__all__)
            self.assertTrue(hasattr(lib, name))
        self.assertIn("reset_logger", lib.__all__)
        self.assertNotIn("app", lib.__all__)
        self.assertFalse(hasattr(lib, "app"))
        self.assertNotIn("TapPayload", lib.__all__)
        self.assertNotIn("BenchmarkOptions", lib.__all__)
        for name in (
            "UnprocessedCommand",
            "RepeatMacroPayload",
            "WaitPayload",
            "TextPayload",
            "_CommandBase",
            "_MacroBase",
        ):
            self.assertNotIn(name, lib.__all__)

    def test_preprocess_flow_transforms_unprocessed_flows(self) -> None:
        scenario = lib.unprocessed_flow([
            lib.unprocessed_app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[
                    lib.repeat(
                        n_iter=2,
                        iter_var="i",
                        commands=[
                            lib.unprocessed_snapshot("shot_{i}"),
                            lib.unprocessed_screenshot("img_{i}"),
                        ],
                    ),
                ],
            )
        ], desc="Reviewer note")

        canonical = lib.preprocess_flow(scenario)

        self.assertEqual(canonical.desc, "Reviewer note")
        commands = canonical.flow[0].commands
        self.assertEqual(
            [command.payload for command in commands],
            ["shot_0", "img_0", "shot_1", "img_1"],
        )

    def test_preprocess_flow_wrapper_revalidates_input(self) -> None:
        scenario = lib.unprocessed_flow([
            lib.unprocessed_app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[
                    lib.repeat(
                        n_iter=1,
                        iter_var="i",
                        commands=[lib.unprocessed_snapshot("shot")],
                    ),
                ],
            )
        ])
        repeat = scenario.flow[0].commands[0]
        assert isinstance(repeat, lib.RepeatMacro)
        repeat.payload.n_iter = -1
        with self.assertRaises(pydantic.ValidationError):
            lib.preprocess_flow(scenario)

    def test_run_accepts_canonical_flow_models_only(self) -> None:
        device = FakeDevice()
        scenario = lib.unprocessed_flow([
            lib.unprocessed_app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[
                    lib.repeat(
                        n_iter=1,
                        iter_var="i",
                        commands=[lib.unprocessed_snapshot("shot_{i}")],
                    ),
                ],
            )
        ])
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            with self.assertRaises(pydantic.ValidationError):
                lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                        hilog=False)

            self.assertFalse(out_dir.exists())

    def test_unprocessed_builders_construct_valid_models(self) -> None:
        scenario = lib.unprocessed_flow([
            lib.unprocessed_app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[
                    lib.repeat(
                        n_iter=1,
                        iter_var="i",
                        commands=[
                            lib.unprocessed_snapshot("shot_{i}"),
                            lib.unprocessed_screenshot("img_{i}"),
                        ],
                    ),
                ],
            )
        ])

        canonical = lib.preprocess_flow(scenario)
        self.assertEqual(len(canonical.flow[0].commands), 2)

    def test_unprocessed_builder_validation_failures_are_raised(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            lib.repeat(n_iter=-1, iter_var="i", commands=[])
        with self.assertRaises(pydantic.ValidationError):
            lib.repeat(n_iter=1, iter_var="a-b", commands=[])
        with self.assertRaises(pydantic.ValidationError):
            lib.repeat(n_iter="1", iter_var="i",  # type: ignore[arg-type]
                       commands=[])
        with self.assertRaises(pydantic.ValidationError):
            lib.unprocessed_flow([
                lib.unprocessed_app_flow(
                    label="app",
                    bundle="com.example",
                    ability="EntryAbility",
                    terminate=False,
                    commands=[lib.snapshot("not valid")],
                )
            ])

    def test_run_returns_none_and_writes_output(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[],
            )
        ])
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                    hilog=False)

            self.assertTrue(out_dir.joinpath("flow.json").is_file())
            self.assertTrue(out_dir.joinpath("summary.csv").is_file())

    def test_run_resolves_relative_out_dir_against_caller_cwd(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[],
            )
        ])
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            original_cwd = pathlib.Path.cwd()
            try:
                os.chdir(root)

                lib.run(scenario, device, out_dir="out",  # type: ignore[arg-type]
                        hilog=False)

                self.assertTrue(root.joinpath("out", "flow.json").is_file())
                self.assertTrue(root.joinpath("out", "summary.csv").is_file())
                self.assertEqual(pathlib.Path.cwd(), root)
            finally:
                os.chdir(original_cwd)

    def test_run_revalidates_mutated_flow_before_device_actions(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[lib.snapshot("first"), lib.snapshot("second")],
            )
        ])
        scenario.flow[0].commands[1].payload = "first"
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            with self.assertRaises(pydantic.ValidationError):
                lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                        hilog=False)

            self.assertFalse(device.screen_timeout_disabled)
            self.assertFalse(device.hilog_configured)
            self.assertFalse(out_dir.exists())

    def test_existing_out_dir_is_unchanged_before_single_run(self) -> None:
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[],
            )
        ])
        device = mock.create_autospec(lib.Device, instance=True)
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")
            out_dir.joinpath("nested").mkdir(parents=True)
            out_dir.joinpath("app_metadata.json").write_bytes(b"SENTINEL\n")
            out_dir.joinpath("nested", "evidence.bin").write_bytes(
                b"\x00existing evidence\xff")
            before = _snapshot_tree(out_dir)

            with self.assertRaises(FileExistsError):
                lib.run(
                    scenario,
                    device,
                    out_dir=out_dir,
                    hilog=False,
                )

            self.assertEqual(_snapshot_tree(out_dir), before)
            self.assertEqual(device.mock_calls, [])

    def test_existing_root_is_unchanged_before_repeated_run(self) -> None:
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[],
            )
        ])
        device = mock.create_autospec(lib.Device, instance=True)
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")
            out_dir.mkdir()
            out_dir.joinpath("existing.txt").write_bytes(b"keep me")
            before = _snapshot_tree(out_dir)

            with self.assertRaises(FileExistsError):
                lib.run(
                    scenario,
                    device,
                    out_dir=out_dir,
                    hilog=False,
                    repeats=2,
                )

            self.assertEqual(_snapshot_tree(out_dir), before)
            self.assertFalse(out_dir.joinpath("iteration_0").exists())
            self.assertFalse(out_dir.joinpath("summary").exists())
            self.assertEqual(device.mock_calls, [])

    def test_run_writes_canonical_flow_json(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[lib.wait(1)],
            )
        ], desc="Reviewer note")
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            lib.run(scenario, device, out_dir=str(out_dir),  # type: ignore[arg-type]
                    hilog=False)

            flow_json = json.loads(out_dir.joinpath(
                "flow.json").read_text(encoding="utf-8"))
            self.assertEqual(flow_json, scenario.model_dump())

    def test_run_rejects_removed_logs_keyword(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([])
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            with self.assertRaises(TypeError):
                lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                        logs=False)  # type: ignore[call-arg]

    def test_run_info_logs_to_stdout(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[lib.text("secret")],
            )
        ])
        stdout = io.StringIO()
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            with contextlib.redirect_stdout(stdout):
                lib.configure_logger("info")
                lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                        hilog=False)

        self.assertIn("info: start benchmark pre flow", stdout.getvalue())
        self.assertIn(
            "info: executing command: app=app action=text", stdout.getvalue())
        self.assertIn("secret", stdout.getvalue())

    def test_run_writes_memmem_log_file(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([])
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            log_path = root.joinpath("memmem.log")
            log_path.write_text("old\n", encoding="utf-8")

            lib.configure_logger("info", log_path)
            lib.run(
                scenario,
                device,  # type: ignore[arg-type]
                out_dir=out_dir,
                hilog=False,
            )

            content = log_path.read_text(encoding="utf-8")
            self.assertNotIn("old", content)
            self.assertIn("info: start benchmark pre flow", content)

    def test_run_with_repeats_preserves_log_file_messages(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([])
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            out_dir = root.joinpath("out")
            log_path = root.joinpath("memmem.log")

            lib.configure_logger("info", log_path)
            lib.run(
                scenario,
                device,  # type: ignore[arg-type]
                out_dir=out_dir,
                hilog=False,
                repeats=2,
            )

            content = log_path.read_text(encoding="utf-8")
            self.assertEqual(
                content.count("info: start benchmark pre flow"), 2)

    def test_run_with_repeats_creates_iterations_and_summary(self) -> None:
        device = FakeDevice()
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[lib.snapshot("start")],
            )
        ])
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                    hilog=False, repeats=3)

            for i in range(3):
                iteration = out_dir.joinpath(f"iteration_{i}")
                self.assertTrue(iteration.joinpath("flow.json").is_file())
                self.assertTrue(iteration.joinpath("summary.csv").is_file())
            self.assertTrue(
                out_dir.joinpath("summary", "summary.csv").is_file())

    def test_run_smaps_filter_propagates_to_reports(self) -> None:
        device = FakeDevice(default_smaps_content=(
            "55f000000000-55f000001000 r--p 00000000 00:00 0 "
            "/lib/libm.so.6\n"
            "Size: 4 kB\nRss: 4 kB\nPss: 4 kB\n"
            "55f000002000-55f000003000 rw-p 00000000 00:00 0\n"
            "Size: 8 kB\nRss: 8 kB\nPss: 8 kB\n"
        ))
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[lib.snapshot("start")],
            )
        ])
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                    hilog=False, smaps_filter=re.compile(r".*\.so"))

            with out_dir.joinpath("summary.csv").open(
                    encoding="utf-8", newline="") as stream:
                rows = list(csv.reader(stream))
            self.assertEqual(len(rows), 2)
            self.assertEqual(rows[1][:2], ["app", "start"])
            size_idx = rows[0].index("size_kb")
            self.assertEqual(rows[1][size_idx], "4")
            with out_dir.joinpath(
                    "breakdowns", "start", "app-start.csv").open(
                    encoding="utf-8", newline="") as stream:
                breakdown_rows = list(csv.reader(stream))
            self.assertEqual([row[0] for row in breakdown_rows[1:]],
                             ["/lib/libm.so.6"])

    def test_run_smaps_filter_no_match_yields_header_only_summary(
            self) -> None:
        device = FakeDevice(default_smaps_content=(
            "55f000000000-55f000001000 r--p 00000000 00:00 0 "
            "/lib/libm.so.6\n"
            "Size: 4 kB\nRss: 4 kB\nPss: 4 kB\n"
        ))
        scenario = lib.flow([
            lib.app_flow(
                label="app",
                bundle="com.example",
                ability="EntryAbility",
                terminate=False,
                commands=[lib.snapshot("start")],
            )
        ])
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            lib.run(scenario, device, out_dir=out_dir,  # type: ignore[arg-type]
                    hilog=False, repeats=2,
                    smaps_filter=re.compile(r"nomatch"))

            for i in range(2):
                iteration = out_dir.joinpath(f"iteration_{i}")
                self.assertTrue(iteration.joinpath("summary.csv").is_file())
                with iteration.joinpath("summary.csv").open(
                        encoding="utf-8", newline="") as stream:
                    rows = list(csv.reader(stream))
                self.assertEqual(len(rows), 1)
            self.assertFalse(out_dir.joinpath(
                "plots", "app", "app-size.svg").exists())


if __name__ == "__main__":
    unittest.main()
