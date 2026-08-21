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

import typing
import unittest

import pydantic

from src.schema import (
    DirectionalFlingCommand,
    DoubleTapCommand,
    DragCommand,
    FlingCommand,
    Flow,
    InputTextCommand,
    KeyCommand,
    LongTapCommand,
    Macro,
    NamedKeyPayload,
    RepeatMacro,
    RepeatMacroPayload,
    ScreenshotCommand,
    SnapshotCommand,
    SwipeCommand,
    TapCommand,
    TextCommand,
    UnprocessedAppFlow,
    UnprocessedFlow,
    UnprocessedScreenshotCommand,
    UnprocessedSnapshotCommand,
    WaitCommand,
    _CommandBase,
    _MacroBase,
)


class SchemaTest(unittest.TestCase):
    def test_valid_flow_loads_and_preserves_order(self) -> None:
        flow = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "first_app",
                        "bundle": "com.example.first",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "wait", "payload": 1}],
                    },
                    {
                        "label": "second-app",
                        "bundle": "com.example.second",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "snapshot", "payload": "after_start"}],
                    },
                ]
            }
        )

        self.assertIsInstance(flow, Flow)
        self.assertEqual([app.label for app in flow.flow],
                         ["first_app", "second-app"])

    def test_flow_description_loads_and_serializes_as_alias(self) -> None:
        flow = Flow.model_validate(
            {
                "$desc": "Measures memory after startup.",
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [],
                    }
                ],
            }
        )

        self.assertEqual(flow.desc, "Measures memory after startup.")
        self.assertEqual(flow.model_dump()[
                         "$desc"], "Measures memory after startup.")

    def test_flow_description_must_be_string(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "$desc": 123,
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [],
                        }
                    ],
                }
            )

    def test_complex_flow_with_mixed_commands_loads(self) -> None:
        flow = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "first_app",
                        "bundle": "com.example.first",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [
                            {"action": "wait", "payload": 0},
                            {"action": "key", "payload": {"key": "Home"}},
                            {"action": "tap", "payload": {
                                "x_pct": 50, "y_pct": 50}},
                            {"action": "snapshot", "payload": "after_home"},
                        ],
                    },
                    {
                        "label": "second-app",
                        "bundle": "com.example.second",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [
                            {"action": "snapshot", "payload": "after_start"},
                            {"action": "wait", "payload": 2},
                            {"action": "key", "payload": {"key": "Back"}},
                            {"action": "snapshot", "payload": "after_back"},
                        ],
                    },
                ]
            }
        )

        first_commands = flow.flow[0].commands
        second_commands = flow.flow[1].commands
        self.assertEqual([app.label for app in flow.flow],
                         ["first_app", "second-app"])
        self.assertIsInstance(first_commands[0], WaitCommand)
        self.assertIsInstance(first_commands[1], KeyCommand)
        self.assertIsInstance(first_commands[2], TapCommand)
        self.assertIsInstance(first_commands[3], SnapshotCommand)
        self.assertIsInstance(second_commands[0], SnapshotCommand)
        self.assertIsInstance(second_commands[1], WaitCommand)
        self.assertIsInstance(second_commands[2], KeyCommand)
        self.assertIsInstance(second_commands[3], SnapshotCommand)
        self.assertEqual([command.action for command in first_commands], [
                         "wait", "key", "tap", "snapshot"])
        self.assertEqual([command.action for command in second_commands], [
                         "snapshot", "wait", "key", "snapshot"])
        first_key = first_commands[1]
        self.assertIsInstance(first_key, KeyCommand)
        first_payload = typing.cast(NamedKeyPayload, first_key.payload)
        self.assertEqual(first_payload.key, "Home")
        self.assertEqual(second_commands[3].payload, "after_back")

    def test_all_new_ui_commands_load(self) -> None:
        flow = Flow.model_validate(
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
                                "x_pct": 50, "y_pct": 50}},
                            {"action": "long_tap", "payload": {
                                "x_pct": 50, "y_pct": 50}},
                            {
                                "action": "swipe",
                                "payload": {"x1_pct": 50, "y1_pct": 80, "x2_pct": 50, "y2_pct": 20, "velocity": 800},
                            },
                            {
                                "action": "drag",
                                "payload": {"x1_pct": 10, "y1_pct": 20, "x2_pct": 90, "y2_pct": 80, "velocity": 600},
                            },
                            {
                                "action": "fling",
                                "payload": {
                                    "x1_pct": 10,
                                    "y1_pct": 20,
                                    "x2_pct": 90,
                                    "y2_pct": 80,
                                    "velocity": 1200,
                                    "step_length": 10,
                                },
                            },
                            {
                                "action": "directional_fling",
                                "payload": {"direction": "up", "velocity": 1200, "step_length": 10},
                            },
                            {"action": "input_text", "payload": {
                                "x_pct": 50, "y_pct": 40, "text": "hello"}},
                            {"action": "text", "payload": "world"},
                        ],
                    }
                ]
            }
        )

        commands = flow.flow[0].commands
        self.assertIsInstance(commands[0], TapCommand)
        self.assertIsInstance(commands[1], DoubleTapCommand)
        self.assertIsInstance(commands[2], LongTapCommand)
        self.assertIsInstance(commands[3], SwipeCommand)
        self.assertIsInstance(commands[4], DragCommand)
        self.assertIsInstance(commands[5], FlingCommand)
        self.assertIsInstance(commands[6], DirectionalFlingCommand)
        self.assertIsInstance(commands[7], InputTextCommand)
        self.assertIsInstance(commands[8], TextCommand)

    def test_missing_required_field_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate({"flow": [
                                {"label": "app", "bundle": "com.example", "ability": "EntryAbility", "commands": []}]})

    def test_screenshot_command_loads(self) -> None:
        flow = Flow.model_validate(
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
        )

        command = flow.flow[0].commands[0]
        self.assertIsInstance(command, ScreenshotCommand)
        self.assertEqual(command.payload, "after_start")

    def test_invalid_app_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {"flow": [{"label": "bad label", "bundle": "com.example",
                           "ability": "EntryAbility", "terminate": False, "commands": []}]}
            )

    def test_empty_app_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {"flow": [{"label": "", "bundle": "com.example",
                           "ability": "EntryAbility", "terminate": False, "commands": []}]}
            )

    def test_invalid_snapshot_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "snapshot", "payload": "bad label"}],
                        }
                    ]
                }
            )

    def test_invalid_screenshot_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "screenshot", "payload": "bad label"}],
                        }
                    ]
                }
            )

    def test_duplicate_app_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {"label": "app", "bundle": "com.example.one",
                            "ability": "EntryAbility", "terminate": False, "commands": []},
                        {"label": "app", "bundle": "com.example.two",
                            "ability": "EntryAbility", "terminate": False, "commands": []},
                    ]
                }
            )

    def test_duplicate_snapshot_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app_one",
                            "bundle": "com.example.one",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "snapshot", "payload": "same"}],
                        },
                        {
                            "label": "app_two",
                            "bundle": "com.example.two",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "snapshot", "payload": "same"}],
                        },
                    ]
                }
            )

    def test_duplicate_screenshot_label_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app_one",
                            "bundle": "com.example.one",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "screenshot", "payload": "same"}],
                        },
                        {
                            "label": "app_two",
                            "bundle": "com.example.two",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "screenshot", "payload": "same"}],
                        },
                    ]
                }
            )

    def test_shared_snapshot_and_screenshot_label_loads(self) -> None:
        flow = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [
                            {"action": "snapshot", "payload": "same"},
                            {"action": "screenshot", "payload": "same"},
                        ],
                    }
                ]
            }
        )

        self.assertIsInstance(flow.flow[0].commands[0], SnapshotCommand)
        self.assertIsInstance(flow.flow[0].commands[1], ScreenshotCommand)

    def test_repeated_bundle_ability_with_unique_labels_loads(self) -> None:
        flow = Flow.model_validate(
            {
                "flow": [
                    {"label": "app_one", "bundle": "com.example",
                        "ability": "EntryAbility", "terminate": False, "commands": []},
                    {"label": "app_two", "bundle": "com.example",
                        "ability": "EntryAbility", "terminate": False, "commands": []},
                ]
            }
        )

        self.assertEqual([app.label for app in flow.flow],
                         ["app_one", "app_two"])

    def test_key_payload_accepts_named_key(self) -> None:
        flow = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "key", "payload": {"key": "Home"}}],
                    }
                ]
            }
        )

        command = flow.flow[0].commands[0]
        self.assertIsInstance(command, KeyCommand)
        payload = typing.cast(NamedKeyPayload, command.payload)
        self.assertEqual(payload.key, "Home")

    def test_wait_payload_must_be_non_negative_integer(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "wait", "payload": -1}],
                        }
                    ]
                }
            )

    def test_wait_payload_can_be_integer(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "wait", "payload": "1"}],
                        }
                    ]
                }
            )

    def test_wait_payload_accepts_non_negative_float(self) -> None:
        flow = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [{"action": "wait", "payload": 2.5}],
                    }
                ]
            }
        )
        command = flow.flow[0].commands[0]
        self.assertIsInstance(command, WaitCommand)
        self.assertEqual(command.payload, 2.5)

    def test_wait_payload_must_be_non_negative_float(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "wait", "payload": -0.5}],
                        }
                    ]
                }
            )

    def test_text_payload_rejects_struct_form(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "text",
                                          "payload": {"text": "hi"}}],
                        }
                    ]
                }
            )

    def test_invalid_key_name_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "key", "payload": {"key": "Menu"}}],
                        }
                    ]
                }
            )

    def test_numeric_key_codes_fail(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "key", "payload": {"keys": [2072, 2038]}}],
                        }
                    ]
                }
            )

    def test_coordinate_percentile_must_be_in_range(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "tap", "payload": {"x_pct": 101, "y_pct": 50}}],
                        }
                    ]
                }
            )

    def test_coordinate_percentile_must_be_integer(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "tap", "payload": {"x_pct": 50.5, "y_pct": 50}}],
                        }
                    ]
                }
            )

    def test_velocity_must_be_in_uitest_range(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [
                                {
                                    "action": "swipe",
                                    "payload": {"x1_pct": 50, "y1_pct": 80, "x2_pct": 50, "y2_pct": 20, "velocity": 199},
                                }
                            ],
                        }
                    ]
                }
            )

    def test_fling_requires_step_length(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [
                                {
                                    "action": "fling",
                                    "payload": {"x1_pct": 50, "y1_pct": 80, "x2_pct": 50, "y2_pct": 20, "velocity": 800},
                                }
                            ],
                        }
                    ]
                }
            )

    def test_step_length_must_be_positive(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [
                                {
                                    "action": "directional_fling",
                                    "payload": {"direction": "up", "velocity": 800, "step_length": 0},
                                }
                            ],
                        }
                    ]
                }
            )

    def test_unknown_command_action_fails(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            Flow.model_validate(
                {
                    "flow": [
                        {
                            "label": "app",
                            "bundle": "com.example",
                            "ability": "EntryAbility",
                            "terminate": False,
                            "commands": [{"action": "unknown", "payload": {}}],
                        }
                    ]
                }
            )


class CommandBaseRefactorTest(unittest.TestCase):
    def test_all_canonical_commands_share_the_payload_base(self) -> None:
        flow: Flow = Flow.model_validate(
            {
                "flow": [
                    {
                        "label": "app",
                        "bundle": "com.example",
                        "ability": "EntryAbility",
                        "terminate": False,
                        "commands": [
                            {"action": "wait", "payload": 1},
                            {"action": "snapshot", "payload": "s"},
                            {"action": "screenshot", "payload": "shot"},
                            {"action": "key", "payload": {"key": "Home"}},
                            {"action": "tap", "payload": {"x_pct": 1, "y_pct": 2}},
                            {"action": "double_tap", "payload": {
                                "x_pct": 1, "y_pct": 2}},
                            {"action": "long_tap", "payload": {
                                "x_pct": 1, "y_pct": 2}},
                            {"action": "swipe", "payload": {
                                "x1_pct": 1, "y1_pct": 2, "x2_pct": 3,
                                "y2_pct": 4, "velocity": 600}},
                            {"action": "drag", "payload": {
                                "x1_pct": 1, "y1_pct": 2, "x2_pct": 3,
                                "y2_pct": 4, "velocity": 600}},
                            {"action": "fling", "payload": {
                                "x1_pct": 1, "y1_pct": 2, "x2_pct": 3,
                                "y2_pct": 4, "velocity": 600, "step_length": 5}},
                            {"action": "directional_fling", "payload": {
                                "direction": "up", "velocity": 600,
                                "step_length": 5}},
                            {"action": "input_text", "payload": {
                                "x_pct": 1, "y_pct": 2, "text": "hi"}},
                            {"action": "text", "payload": "hi"},
                        ],
                    }
                ]
            }
        )

        for command in flow.flow[0].commands:
            self.assertIsInstance(command, _CommandBase)

    def test_wait_payload_constraint_survives_generic_argument(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            WaitCommand.model_validate({"action": "wait", "payload": -0.5})
        with self.assertRaises(pydantic.ValidationError):
            WaitCommand.model_validate({"action": "wait", "payload": "1"})
        self.assertEqual(
            WaitCommand.model_validate(
                {"action": "wait", "payload": 1.5}).payload, 1.5)

    def test_canonical_snapshot_stays_strict(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            SnapshotCommand.model_validate(
                {"action": "snapshot", "payload": 123})
        with self.assertRaises(pydantic.ValidationError):
            SnapshotCommand.model_validate(
                {"action": "snapshot", "payload": "bad label"})
        self.assertEqual(SnapshotCommand.model_validate(
            {"action": "snapshot", "payload": "ok_1"}).payload, "ok_1")


class UnprocessedSchemaTest(unittest.TestCase):
    def _app(self, commands: list[object]) -> dict[str, object]:
        return {
            "label": "app",
            "bundle": "com.example",
            "ability": "EntryAbility",
            "terminate": False,
            "commands": commands,
        }

    def test_unprocessed_snapshot_command_accepts_placeholder_label(self) -> None:
        command = UnprocessedSnapshotCommand.model_validate(
            {"action": "snapshot", "payload": "shot_{i}"})

        self.assertIsInstance(command, _CommandBase)
        self.assertEqual(command.payload, "shot_{i}")
        self.assertTrue(
            UnprocessedSnapshotCommand.model_fields[
                "payload"].annotation is str)

    def test_unprocessed_screenshot_command_accepts_placeholder_label(self) -> None:
        command = UnprocessedScreenshotCommand.model_validate(
            {"action": "screenshot", "payload": "img_{i}"})

        self.assertIsInstance(command, _CommandBase)
        self.assertEqual(command.payload, "img_{i}")

    def test_canonical_commands_reject_same_placeholder_labels(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            SnapshotCommand.model_validate(
                {"action": "snapshot", "payload": "shot_{i}"})
        with self.assertRaises(pydantic.ValidationError):
            ScreenshotCommand.model_validate(
                {"action": "screenshot", "payload": "img_{i}"})

    def test_unprocessed_command_union_discriminates_on_action(self) -> None:
        payload = RepeatMacroPayload.model_validate(
            {
                "iter_var": "i",
                "n_iter": 1,
                "commands": [
                    {"action": "wait", "payload": 1},
                    {"action": "snapshot", "payload": "shot_{i}"},
                    {"action": "key", "payload": {"key": "Home"}},
                ],
            }
        )

        body = payload.commands
        self.assertIsInstance(body[0], WaitCommand)
        self.assertIsInstance(body[1], UnprocessedSnapshotCommand)
        self.assertIsInstance(body[2], KeyCommand)

    def test_unprocessed_app_flow_accepts_macro_and_command_items(self) -> None:
        app = UnprocessedAppFlow.model_validate(
            self._app(
                [
                    {
                        "macro": "repeat",
                        "payload": {
                            "iter_var": "i",
                            "n_iter": 2,
                            "commands": [
                                {"action": "snapshot", "payload": "shot_{i}"},
                            ],
                        },
                    },
                    {"action": "wait", "payload": 0},
                ]
            )
        )

        self.assertIsInstance(app.commands[0], RepeatMacro)
        self.assertIsInstance(app.commands[1], WaitCommand)

    def test_unprocessed_flow_uses_desc_alias(self) -> None:
        flow = UnprocessedFlow.model_validate(
            {
                "$desc": "Authored flow.",
                "flow": [self._app([])],
            }
        )

        self.assertEqual(flow.desc, "Authored flow.")
        self.assertEqual(flow.model_dump()[
                         "$desc"], "Authored flow.")

    def test_unprocessed_flow_defers_label_uniqueness(self) -> None:
        flow = UnprocessedFlow.model_validate(
            {
                "flow": [
                    self._app(
                        [
                            {"action": "snapshot", "payload": "same"},
                        ]
                    ),
                    self._app(
                        [
                            {"action": "snapshot", "payload": "same"},
                        ]
                    ),
                ]
            }
        )
        self.assertEqual(len(flow.flow), 2)

    def test_repeat_macro_payload_constraints(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {"iter_var": "a-b", "n_iter": 1, "commands": []})
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {"iter_var": "1bad", "n_iter": 1, "commands": []})
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {"iter_var": "i", "n_iter": -1, "commands": []})
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {"iter_var": "i", "n_iter": "1", "commands": []})
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {"iter_var": "i", "n_iter": 1.5, "commands": []})
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {"iter_var": "i", "n_iter": 1})
        payload = RepeatMacroPayload.model_validate(
            {"iter_var": "i", "n_iter": 0, "commands": []})
        self.assertEqual((payload.iter_var, payload.n_iter), ("i", 0))

    def test_nested_macro_is_rejected(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            RepeatMacroPayload.model_validate(
                {
                    "iter_var": "i",
                    "n_iter": 1,
                    "commands": [
                        {
                            "macro": "repeat",
                            "payload": {
                                "iter_var": "j",
                                "n_iter": 1,
                                "commands": [],
                            },
                        }
                    ],
                }
            )

    def test_top_level_placeholder_labels_fail_unprocessed_validation(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        self._app(
                            [{"action": "snapshot", "payload": "shot_{i}"}]
                        )
                    ]
                }
            )
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        self._app(
                            [{"action": "screenshot", "payload": "img_{i}"}]
                        )
                    ]
                }
            )

    def test_unknown_keys_are_rejected_everywhere(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {"extra": True, "flow": [self._app([])]})
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {"flow": [{**self._app([]), "extra": True}]})
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        self._app(
                            [
                                {
                                    "action": "tap",
                                    "payload": {
                                        "x_pct": 1,
                                        "y_pct": 2,
                                        "z_pct": 3,
                                    },
                                }
                            ]
                        )
                    ]
                }
            )
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        self._app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 1,
                                        "commands": [],
                                        "unexpected": True,
                                    },
                                }
                            ]
                        )
                    ]
                }
            )

    def test_item_with_action_and_macro_keys_is_rejected(self) -> None:
        for item in (
            {
                "action": "wait",
                "payload": 1,
                "macro": "repeat",
            },
            {
                "macro": "repeat",
                "payload": {
                    "iter_var": "i",
                    "n_iter": 1,
                    "commands": [],
                },
                "action": "wait",
            },
        ):
            with self.assertRaises(pydantic.ValidationError):
                UnprocessedFlow.model_validate(
                    {"flow": [self._app([item])]}
                )

    def test_macro_union_discriminates_on_macro_field(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        self._app(
                            [
                                {
                                    "macro": "unknown",
                                    "payload": {},
                                }
                            ]
                        )
                    ]
                }
            )


if __name__ == "__main__":
    unittest.main()
