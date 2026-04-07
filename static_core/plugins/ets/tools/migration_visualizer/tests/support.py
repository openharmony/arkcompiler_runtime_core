#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

from __future__ import annotations

import copy
import importlib.util
import itertools
import json
import sys
from pathlib import Path
from typing import Any, Dict


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
_MODULE_COUNTER = itertools.count()

if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))


def load_module(module_label: str, relative_path: str):
    module_path = PROJECT_ROOT / relative_path
    module_name = f"{module_label}_{next(_MODULE_COUNTER)}"
    spec = importlib.util.spec_from_file_location(module_name, module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module spec: {module_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    try:
        spec.loader.exec_module(module)
    except Exception:
        sys.modules.pop(module_name, None)
        raise
    return module


def write_json(path: Path, data: Any) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    return path


def minimal_input_bundle() -> Dict[str, Any]:
    return copy.deepcopy(
        {
            "fileDepGraph": {
                "categories": [{"id": 1, "name": "FILE"}],
                "nodes": [
                    {
                        "id": "f_entry_index",
                        "name": "workspace/project/entry/src/main/ets/pages/Index.ets",
                        "kind": 1,
                    },
                    {
                        "id": "f_entry_api",
                        "name": "workspace/project/entry/src/main/ets/services/Api.ets",
                        "kind": 1,
                    },
                    {
                        "id": "f_utils_helper",
                        "name": "workspace/project/utils/src/main/ets/utils/Helper.ets",
                        "kind": 1,
                    },
                ],
                "edges": [
                    {"source": "f_entry_index", "target": "f_entry_api"},
                    {"source": "f_entry_api", "target": "f_utils_helper"},
                ],
            },
            "moduleDepGraph": {
                "nodes": [
                    {"id": "m_project", "name": "workspace/project"},
                    {"id": "m_entry", "name": "workspace/project/entry"},
                    {"id": "m_utils", "name": "workspace/project/utils"},
                ],
                "edges": [{"source": "m_entry", "target": "m_utils"}],
            },
            "hapray_report": {
                "perf": {
                    "steps": [
                        {
                            "data": [
                                {
                                    "stepIdx": 0,
                                    "pid": 100,
                                    "tid": 10,
                                    "processName": "main",
                                    "componentName": "entry.har",
                                    "file": "entry/src/main/ets/pages/Index.ts",
                                    "fileEvents": 30,
                                    "symbol": "render",
                                    "symbolEvents": 18,
                                },
                                {
                                    "stepIdx": 0,
                                    "pid": 101,
                                    "tid": 11,
                                    "processName": "worker",
                                    "componentName": "utils.har",
                                    "file": "utils/src/main/ets/utils/Helper.ts",
                                    "fileEvents": 13,
                                    "symbol": "helper",
                                    "symbolEvents": 7,
                                },
                            ]
                        }
                    ],
                    "har": [
                        {"name": "entry.har", "count": 30},
                        {"name": "utils.har", "count": 13},
                    ],
                }
            },
            "component_timings": [
                {"name": "entry.har", "count": 30},
                {"name": "utils.har", "count": 13},
            ],
        }
    )


def write_minimal_input_bundle(target_dir: Path) -> Dict[str, Any]:
    bundle = minimal_input_bundle()
    write_json(target_dir / "fileDepGraph.json", bundle["fileDepGraph"])
    write_json(target_dir / "moduleDepGraph.json", bundle["moduleDepGraph"])
    write_json(target_dir / "hapray_report.json", bundle["hapray_report"])
    write_json(target_dir / "component_timings.json", bundle["component_timings"])
    return bundle
