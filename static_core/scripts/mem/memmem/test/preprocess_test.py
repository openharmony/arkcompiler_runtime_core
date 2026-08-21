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

import json
import pathlib
import tempfile
import unittest
from test.mock.device import FakeDevice

import pydantic

from src.preprocess import preprocess_flow
from src.schema import (
    InputTextCommand,
    RepeatMacroPayload,
    SnapshotCommand,
    UnprocessedFlow,
)

import lib


def _app(commands: list[object], label: str = "app") -> dict[str, object]:
    return {
        "label": label,
        "bundle": "com.example",
        "ability": "EntryAbility",
        "terminate": False,
        "commands": commands,
    }


class PreprocessTest(unittest.TestCase):
    def test_macro_expands_with_count_order_and_0_based_substitution(self) -> None:
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 3,
                                        "commands": [
                                            {"action": "snapshot",
                                                "payload": "shot_{i}"},
                                            {"action": "wait", "payload": 1},
                                        ],
                                    },
                                },
                                {"action": "snapshot", "payload": "final"},
                            ]
                        )
                    ]
                }
            )
        )

        commands = flow.flow[0].commands
        labels = [
            command.payload
            for command in commands
            if isinstance(command, SnapshotCommand)
        ]
        self.assertEqual(labels, ["shot_0", "shot_1", "shot_2", "final"])
        self.assertEqual(
            [type(command).__name__ for command in commands],
            [
                "SnapshotCommand",
                "WaitCommand",
                "SnapshotCommand",
                "WaitCommand",
                "SnapshotCommand",
                "WaitCommand",
                "SnapshotCommand",
            ],
        )

    def test_substitution_applies_to_flat_string_payloads_only(self) -> None:
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 2,
                                        "commands": [
                                            {
                                                "action": "input_text",
                                                "payload": {
                                                    "x_pct": 10,
                                                    "y_pct": 20,
                                                    "text": "item{i}",
                                                },
                                            },
                                            {"action": "text",
                                                "payload": "note{i}"},
                                        ],
                                    },
                                }
                            ]
                        )
                    ]
                }
            )
        )

        commands = flow.flow[0].commands
        assert isinstance(commands[0], InputTextCommand)
        self.assertEqual(commands[0].payload.text, "item{i}")
        self.assertEqual(commands[0].payload.x_pct, 10)
        self.assertEqual(commands[1].payload, "note0")
        self.assertEqual(commands[3].payload, "note1")

    def test_text_payload_substitutes_in_macro_bodies(self) -> None:
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 2,
                                        "commands": [
                                            {"action": "text",
                                                "payload": "note{i}"},
                                        ],
                                    },
                                }
                            ]
                        )
                    ]
                }
            )
        )

        self.assertEqual(
            [command.payload for command in flow.flow[0].commands],
            ["note0", "note1"],
        )

    def test_expanded_labels_are_validated_for_regex_and_uniqueness(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            preprocess_flow(
                UnprocessedFlow.model_validate(
                    {
                        "flow": [
                            _app(
                                [
                                    {
                                        "macro": "repeat",
                                        "payload": {
                                            "iter_var": "i",
                                            "n_iter": 2,
                                            "commands": [
                                                {
                                                    "action": "snapshot",
                                                    "payload": "s {i}",
                                                }
                                            ],
                                        },
                                    }
                                ]
                            )
                        ]
                    }
                )
            )
        with self.assertRaises(pydantic.ValidationError):
            preprocess_flow(
                UnprocessedFlow.model_validate(
                    {
                        "flow": [
                            _app(
                                [
                                    {
                                        "macro": "repeat",
                                        "payload": {
                                            "iter_var": "i",
                                            "n_iter": 2,
                                            "commands": [
                                                {
                                                    "action": "snapshot",
                                                    "payload": "same",
                                                }
                                            ],
                                        },
                                    }
                                ]
                            )
                        ]
                    }
                )
            )

    def test_string_placeholder_in_numeric_field_fails_before_expansion(self) -> None:
        with self.assertRaises(pydantic.ValidationError):
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 1,
                                        "commands": [
                                            {"action": "wait", "payload": "{i}"}
                                        ],
                                    },
                                }
                            ]
                        )
                    ]
                }
            )

    def test_zero_iterations_and_empty_body_expand_to_nothing(self) -> None:
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 0,
                                        "commands": [
                                            {"action": "snapshot", "payload": "s"}
                                        ],
                                    },
                                },
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 5,
                                        "commands": [],
                                    },
                                },
                                {"action": "wait", "payload": 1},
                            ]
                        )
                    ]
                }
            )
        )

        self.assertEqual(len(flow.flow[0].commands), 1)

    def test_zero_iterations_expand_to_nothing_regardless_of_commands(self) -> None:
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 0,
                                        "commands": [
                                            {"action": "snapshot",
                                                "payload": "s{i}"},
                                            {"action": "snapshot",
                                                "payload": "s{i}"},
                                        ],
                                    },
                                }
                            ]
                        )
                    ]
                }
            )
        )

        self.assertEqual(flow.flow[0].commands, [])

    def test_unbounded_n_iter_is_accepted(self) -> None:
        payload = RepeatMacroPayload.model_validate(
            {"iter_var": "i", "n_iter": 10**9, "commands": []})
        self.assertEqual(payload.n_iter, 10**9)

    def test_preprocess_preserves_desc_metadata(self) -> None:
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "$desc": "Reviewer note",
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 2,
                                        "commands": [
                                            {
                                                "action": "snapshot",
                                                "payload": "shot_{i}",
                                            }
                                        ],
                                    },
                                }
                            ]
                        )
                    ],
                }
            )
        )

        self.assertEqual(flow.desc, "Reviewer note")
        self.assertEqual(flow.model_dump(), {
            "$desc": "Reviewer note",
            "flow": [
                {
                    "label": "app",
                    "bundle": "com.example",
                    "ability": "EntryAbility",
                    "terminate": False,
                    "commands": [
                        {"action": "snapshot", "payload": "shot_0"},
                        {"action": "snapshot", "payload": "shot_1"},
                    ],
                }
            ],
        })

    def test_run_persists_expanded_canonical_flow_json(self) -> None:
        device = FakeDevice()
        flow = preprocess_flow(
            UnprocessedFlow.model_validate(
                {
                    "flow": [
                        _app(
                            [
                                {
                                    "macro": "repeat",
                                    "payload": {
                                        "iter_var": "i",
                                        "n_iter": 2,
                                        "commands": [
                                            {"action": "snapshot",
                                                "payload": "shot_{i}"},
                                        ],
                                    },
                                }
                            ]
                        )
                    ]
                }
            )
        )
        with tempfile.TemporaryDirectory() as directory:
            out_dir = pathlib.Path(directory).joinpath("out")

            lib.run(flow, device, out_dir=out_dir,  # type: ignore[arg-type]
                    hilog=False)

            flow_json = json.loads(
                out_dir.joinpath("flow.json").read_text(encoding="utf-8"))
        self.assertNotIn("macro", json.dumps(flow_json))
        self.assertEqual(flow_json["flow"][0]["commands"], [
            {"action": "snapshot", "payload": "shot_0"},
            {"action": "snapshot", "payload": "shot_1"},
        ])


if __name__ == "__main__":
    unittest.main()
