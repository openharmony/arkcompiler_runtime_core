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

import json
import tempfile
import unittest
from pathlib import Path

from support import load_module, write_minimal_input_bundle


class IntegrateDepTest(unittest.TestCase):
    def test_extract_external_declaration_dependency_name(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")

        self.assertEqual(
            integrate_dep.extract_external_declaration_dependency_name(
                "C:\\SDK\\default\\openharmony\\ets\\kits\\@kit.ArkUI.d.ts"
            ),
            "@kit.ArkUI",
        )
        self.assertEqual(
            integrate_dep.extract_external_declaration_dependency_name(
                "C:\\SDK\\default\\openharmony\\ets\\kits\\@kit.AbilityKit.d.ets"
            ),
            "@kit.AbilityKit",
        )
        self.assertEqual(
            integrate_dep.extract_external_declaration_dependency_name("workspace/project/entry/src/main/ets/pages/Index.ets"),
            "",
        )

    def test_build_cache_file_is_mapped_back_to_source(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        source_path = "workspace/project/entry/src/main/ets/pages/Index.ets"
        build_cache_path = (
            "workspace/project/entry/build/default/cache/default/default@CompileArkTS/"
            "esmodule/debug/entry/src/main/ets/pages/Index.ts"
        )
        file_graph = {
            "categories": [{"id": 1, "name": "FILE"}],
            "nodes": [
                {"id": "src", "name": source_path, "kind": 1},
                {"id": "cache", "name": build_cache_path, "kind": 1},
            ],
        }

        source_index = integrate_dep.build_source_file_index(file_graph)

        self.assertEqual(
            integrate_dep.map_build_cache_file_to_source(build_cache_path, source_index),
            source_path,
        )
        self.assertEqual(
            integrate_dep.build_file_backsource_map(file_graph, source_index),
            {build_cache_path: source_path},
        )

    def test_collect_hapray_file_data_deduplicates_and_backsources(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        source_path = "workspace/project/entry/src/main/ets/pages/Index.ets"
        helper_path = "workspace/project/utils/src/main/ets/utils/Helper.ets"
        file_graph = {
            "categories": [{"id": 1, "name": "FILE"}],
            "nodes": [
                {"id": "src", "name": source_path, "kind": 1},
                {"id": "helper", "name": helper_path, "kind": 1},
            ],
        }
        hapray_report = {
            "perf": {
                "steps": [
                    {
                        "data": [
                            {
                                "stepIdx": 0,
                                "pid": 1,
                                "tid": 2,
                                "processName": "main",
                                "componentName": "entry.har",
                                "file": "entry/src/main/ets/pages/Index.ts",
                                "fileEvents": 10,
                                "symbol": "foo",
                                "symbolEvents": 3,
                            },
                            {
                                "stepIdx": 0,
                                "pid": 1,
                                "tid": 2,
                                "processName": "main",
                                "componentName": "entry.har",
                                "file": "entry/src/main/ets/pages/Index.ts",
                                "fileEvents": 20,
                                "symbol": "foo",
                                "symbolEvents": 5,
                            },
                            {
                                "stepIdx": 0,
                                "pid": 1,
                                "tid": 2,
                                "processName": "main",
                                "componentName": "entry.har",
                                "file": "entry/src/main/ets/pages/Index.ts",
                                "fileEvents": 20,
                                "symbol": "bar",
                                "symbolEvents": 2,
                            },
                            {
                                "stepIdx": 0,
                                "pid": 3,
                                "tid": 4,
                                "processName": "worker",
                                "componentName": "utils.har",
                                "file": "utils/src/main/ets/utils/Helper.ts",
                                "fileEvents": 13,
                                "symbol": "baz",
                                "symbolEvents": 4,
                            },
                        ]
                    }
                ]
            }
        }

        source_index = integrate_dep.build_source_file_index(file_graph)
        collected = integrate_dep.collect_hapray_file_data(hapray_report, source_index)
        entry_key = integrate_dep.normalize_path_key(source_path)
        helper_key = integrate_dep.normalize_path_key(helper_path)

        self.assertEqual(collected["path_timing"]["total"][entry_key], 20)
        self.assertEqual(collected["path_timing"]["self"][entry_key], 7)
        self.assertEqual(collected["path_timing"]["raw"][entry_key], 50)
        self.assertEqual(collected["path_timing"]["total"][helper_key], 13)
        self.assertEqual(collected["report_component_timing"]["total"]["entry"], 20)
        self.assertEqual(collected["report_component_timing"]["self"]["entry"], 7)
        self.assertEqual(
            collected["build_cache_backsource_map"],
            {
                "entry/src/main/ets/pages/Index.ts": source_path,
                "utils/src/main/ets/utils/Helper.ts": helper_path,
            },
        )

    def test_infer_file_to_har_map_filters_project_path_leakage(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        file_graph = {
            "categories": [{"id": 1, "name": "FILE"}],
            "nodes": [
                {
                    "id": "entry",
                    "name": "home/dev/project/entry/src/main/ets/pages/Index.ets",
                    "kind": 1,
                },
                {
                    "id": "utils",
                    "name": "home/dev/project/utils/src/main/ets/utils/Helper.ets",
                    "kind": 1,
                },
            ],
        }
        module_graph = {
            "nodes": [
                {"id": "project", "name": "home/dev/project"},
                {"id": "entry", "name": "home/dev/project/entry"},
                {"id": "utils", "name": "home/dev/project/utils"},
            ]
        }

        file_to_har, module_hars, inferred_hars, path_leakage, project_container = (
            integrate_dep.infer_file_to_har_map(file_graph, module_graph)
        )

        self.assertEqual(module_hars, ["entry", "utils"])
        self.assertEqual(inferred_hars, [])
        self.assertEqual(
            file_to_har["home/dev/project/entry/src/main/ets/pages/Index.ets"],
            "entry",
        )
        self.assertEqual(
            file_to_har["home/dev/project/utils/src/main/ets/utils/Helper.ets"],
            "utils",
        )
        self.assertTrue({"home", "dev", "project"}.issubset(path_leakage))
        self.assertEqual(project_container, "project")

    def test_integrate_artifact_bundle_prefers_sdk_kit_dependencies_over_backsourced_build_cache_packages(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            bundle_dir = Path(temp_dir)
            file_graph = {
                "categories": [
                    {"id": 0, "name": "FILE"},
                    {"id": 1, "name": "PKG"},
                ],
                "nodes": [
                    {
                        "id": "src_index",
                        "name": "C:\\OH\\FrameAndPropertyAnimation\\entry\\src\\main\\ets\\pages\\Index.ets",
                        "kind": 0,
                    },
                    {
                        "id": "src_helper",
                        "name": "C:\\OH\\FrameAndPropertyAnimation\\entry\\src\\main\\ets\\pages\\Helper.ets",
                        "kind": 0,
                    },
                    {
                        "id": "cache_index",
                        "name": (
                            "C:\\OH\\FrameAndPropertyAnimation\\entry\\build\\default\\cache\\default\\"
                            "default@CompileArkTS\\esmodule\\debug\\entry\\src\\main\\ets\\pages\\Index.ts"
                        ),
                        "kind": 0,
                    },
                    {
                        "id": "cache_helper",
                        "name": (
                            "C:\\OH\\FrameAndPropertyAnimation\\entry\\build\\default\\cache\\default\\"
                            "default@CompileArkTS\\esmodule\\debug\\entry\\src\\main\\ets\\pages\\Helper.ts"
                        ),
                        "kind": 0,
                    },
                    {
                        "id": "sdk_arkui",
                        "name": (
                            "C:\\SDK\\default\\openharmony\\ets\\kits\\"
                            "@kit.ArkUI.d.ts"
                        ),
                        "kind": 0,
                    },
                    {"id": "pkg_hypium", "name": "@ohos/hypium", "kind": 1},
                    {"id": "pkg_router", "name": "@ohos:router", "kind": 1},
                    {"id": "pkg_base", "name": "@ohos:base", "kind": 1},
                ],
                "edges": [
                    {"source": "src_index", "target": "src_helper"},
                    {"source": "src_index", "target": "sdk_arkui"},
                    {"source": "src_index", "target": "pkg_hypium"},
                    {"source": "src_index", "target": "pkg_router"},
                    {"source": "cache_index", "target": "cache_helper"},
                    {"source": "cache_index", "target": "pkg_router"},
                    {"source": "cache_index", "target": "pkg_base"},
                ],
            }
            module_graph = {
                "nodes": [
                    {"id": "project", "name": "C:\\OH\\FrameAndPropertyAnimation"},
                    {"id": "entry", "name": "C:\\OH\\FrameAndPropertyAnimation\\entry"},
                ],
                "edges": [{"source": "project", "target": "entry"}],
            }
            hapray_report = {
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
                                }
                            ]
                        }
                    ],
                    "har": [{"name": "entry.har", "count": 30}],
                }
            }
            component_timings = [{"name": "entry.har", "count": 30}]

            (bundle_dir / "fileDepGraph.json").write_text(json.dumps(file_graph, ensure_ascii=False), encoding="utf-8")
            (bundle_dir / "moduleDepGraph.json").write_text(json.dumps(module_graph, ensure_ascii=False), encoding="utf-8")
            (bundle_dir / "hapray_report.json").write_text(json.dumps(hapray_report, ensure_ascii=False), encoding="utf-8")
            (bundle_dir / "component_timings.json").write_text(json.dumps(component_timings, ensure_ascii=False), encoding="utf-8")

            integrate_dep.integrate_artifact_bundle(
                integrate_dep.IntegrationRequest(input_dir=bundle_dir)
            )

            payload = json.loads((bundle_dir / "hierarchical_integrated_data.json").read_text(encoding="utf-8"))
            har_node_names = {node["name"] for node in payload["harGraph"]["nodes"]}
            entry_edges = payload["fileGraphs"]["entry"]["edges"]
            cross_har_targets = {
                (item["sourceId"], item["targetHarId"], item["count"])
                for item in payload["crossHarDependencies"]
            }

            self.assertIn("@kit.ArkUI", har_node_names)
            self.assertIn("@ohos/hypium", har_node_names)
            self.assertIn("@ohos:router", har_node_names)
            self.assertNotIn("Program Files", har_node_names)
            self.assertNotIn("@ohos:base", har_node_names)
            self.assertEqual(entry_edges, [{"source": "src_index", "target": "src_helper"}])
            self.assertIn(("src_index", "@kit.ArkUI", 1), cross_har_targets)
            self.assertIn(("src_index", "@ohos/hypium", 1), cross_har_targets)
            self.assertIn(("src_index", "@ohos:router", 1), cross_har_targets)

    def test_integrate_artifact_bundle_deduplicates_backsource_internal_edges(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            bundle_dir = Path(temp_dir)
            file_graph = {
                "categories": [{"id": 0, "name": "FILE"}],
                "nodes": [
                    {
                        "id": "src_index",
                        "name": "workspace/project/entry/src/main/ets/pages/Index.ets",
                        "kind": 0,
                    },
                    {
                        "id": "src_helper",
                        "name": "workspace/project/entry/src/main/ets/pages/Helper.ets",
                        "kind": 0,
                    },
                    {
                        "id": "cache_index",
                        "name": (
                            "workspace/project/entry/build/default/cache/default/default@CompileArkTS/"
                            "esmodule/debug/entry/src/main/ets/pages/Index.ts"
                        ),
                        "kind": 0,
                    },
                    {
                        "id": "cache_helper",
                        "name": (
                            "workspace/project/entry/build/default/cache/default/default@CompileArkTS/"
                            "esmodule/debug/entry/src/main/ets/pages/Helper.ts"
                        ),
                        "kind": 0,
                    },
                ],
                "edges": [
                    {"source": "src_index", "target": "src_helper"},
                    {"source": "cache_index", "target": "cache_helper"},
                ],
            }
            module_graph = {
                "nodes": [
                    {"id": "project", "name": "workspace/project"},
                    {"id": "entry", "name": "workspace/project/entry"},
                ],
                "edges": [{"source": "project", "target": "entry"}],
            }
            hapray_report = {"perf": {"steps": [], "har": []}}
            component_timings = []

            (bundle_dir / "fileDepGraph.json").write_text(json.dumps(file_graph, ensure_ascii=False), encoding="utf-8")
            (bundle_dir / "moduleDepGraph.json").write_text(json.dumps(module_graph, ensure_ascii=False), encoding="utf-8")
            (bundle_dir / "hapray_report.json").write_text(json.dumps(hapray_report, ensure_ascii=False), encoding="utf-8")
            (bundle_dir / "component_timings.json").write_text(json.dumps(component_timings, ensure_ascii=False), encoding="utf-8")

            integrate_dep.integrate_artifact_bundle(
                integrate_dep.IntegrationRequest(input_dir=bundle_dir)
            )

            payload = json.loads((bundle_dir / "hierarchical_integrated_data.json").read_text(encoding="utf-8"))

            self.assertEqual(
                payload["fileGraphs"]["entry"]["edges"],
                [{"source": "src_index", "target": "src_helper"}],
            )

    def test_integrate_artifact_bundle_generates_output_and_debug_files(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            bundle_dir = Path(temp_dir)
            write_minimal_input_bundle(bundle_dir)

            integrate_dep.integrate_artifact_bundle(
                integrate_dep.IntegrationRequest(input_dir=bundle_dir)
            )

            output_path = bundle_dir / "hierarchical_integrated_data.json"
            debug_dir = bundle_dir / "debug"
            payload = json.loads(output_path.read_text(encoding="utf-8"))

            self.assertEqual(
                set(payload),
                {"metricInfo", "filters", "harGraph", "fileGraphs", "crossHarDependencies"},
            )
            self.assertIn("entry", payload["fileGraphs"])
            self.assertIn("utils", payload["fileGraphs"])
            self.assertTrue((debug_dir / "har_timing_map.json").is_file())
            self.assertTrue((debug_dir / "file_to_har_map.json").is_file())

    def test_integrate_artifact_bundle_no_debug_skips_debug_directory(self) -> None:
        integrate_dep = load_module("integrate_dep", "src/arkts_migration_visualizer/build/integrate_dep.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            bundle_dir = Path(temp_dir)
            output_path = bundle_dir / "out.json"
            write_minimal_input_bundle(bundle_dir)

            integrate_dep.integrate_artifact_bundle(
                integrate_dep.IntegrationRequest(
                    input_dir=bundle_dir,
                    output_path=output_path,
                    debug_enabled=False,
                )
            )

            self.assertTrue(output_path.is_file())
            self.assertFalse((bundle_dir / "debug").exists())

if __name__ == "__main__":
    unittest.main()
