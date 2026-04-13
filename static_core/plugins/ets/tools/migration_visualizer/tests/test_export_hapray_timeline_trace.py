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
import sqlite3
import tempfile
import unittest
from pathlib import Path

from support import load_module


def create_minimal_perf_db(db_path: Path) -> None:
    conn = sqlite3.connect(str(db_path))
    try:
        conn.executescript(
            """
            CREATE TABLE perf_report (
                id INTEGER PRIMARY KEY,
                report_value TEXT
            );
            CREATE TABLE perf_sample (
                id INTEGER PRIMARY KEY,
                callchain_id INTEGER,
                thread_id INTEGER,
                event_count INTEGER,
                cpu_id INTEGER,
                event_type_id INTEGER,
                timestamp_trace INTEGER
            );
            CREATE TABLE perf_thread (
                thread_id INTEGER,
                process_id INTEGER,
                thread_name TEXT
            );
            CREATE TABLE perf_callchain (
                callchain_id INTEGER,
                depth INTEGER,
                file_id INTEGER,
                name INTEGER
            );
            CREATE TABLE perf_files (
                file_id INTEGER,
                path TEXT
            );
            CREATE TABLE data_dict (
                id INTEGER,
                data TEXT
            );
            """
        )
        conn.executemany(
            "INSERT INTO perf_report(id, report_value) VALUES(?, ?)",
            [
                (1, "instructions"),
                (2, "cpu-cycles"),
            ],
        )
        conn.executemany(
            "INSERT INTO perf_thread(thread_id, process_id, thread_name) VALUES(?, ?, ?)",
            [
                (100, 100, "demo_process"),
                (101, 100, "RenderThread"),
            ],
        )
        conn.executemany(
            "INSERT INTO perf_files(file_id, path) VALUES(?, ?)",
            [
                (1, "/system/lib64/libroot.so"),
                (2, "/data/app/el1/bundle/public/demo/lib/arm64/libleaf.so"),
            ],
        )
        conn.executemany(
            "INSERT INTO data_dict(id, data) VALUES(?, ?)",
            [
                (10, "rootLoop"),
                (11, "leafWork"),
                (12, "handleInput"),
                (13, "dispatchFrame"),
            ],
        )
        conn.executemany(
            "INSERT INTO perf_callchain(callchain_id, depth, file_id, name) VALUES(?, ?, ?, ?)",
            [
                (200, 2, 2, 11),
                (200, 1, 1, 10),
                (201, 2, 2, 13),
                (201, 1, 1, 12),
            ],
        )
        conn.executemany(
            """
            INSERT INTO perf_sample(id, callchain_id, thread_id, event_count, cpu_id, event_type_id, timestamp_trace)
            VALUES(?, ?, ?, ?, ?, ?, ?)
            """,
            [
                (1, 200, 101, 7, 0, 1, 1_000_000),
                (2, 200, 101, 8, 0, 1, 2_000_000),
                (3, 201, 101, 9, 0, 1, 3_000_000),
                (4, 200, 101, 10, 0, 2, 1_500_000),
            ],
        )
        conn.commit()
    finally:
        conn.close()


class ExportHaprayTimelineTraceTest(unittest.TestCase):
    def test_main_rejects_standalone_entrypoint(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")

        with self.assertRaises(SystemExit) as ctx:
            exporter.main()

        self.assertIn("Use arkts-migration-visualizer export-timeline instead.", str(ctx.exception))

    def test_default_repo_root_points_to_project_root(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")

        self.assertEqual(exporter.default_repo_root(), Path(__file__).resolve().parents[1])

    def test_resolve_perf_db_path_can_auto_pick_latest_report_from_repo_config(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "repo"
            deps_root = repo_root / "deps"
            reports_root = deps_root / "sources" / "ArkAnalyzer-HapRay" / "perf_testing" / "reports"
            latest_perf = reports_root / "20260409110000" / "case_b" / "hiperf" / "step2" / "perf.db"
            older_perf = reports_root / "20260408110000" / "case_a" / "hiperf" / "step1" / "perf.db"
            (repo_root / "configs").mkdir(parents=True)
            (repo_root / "configs" / "config.json").write_text(
                json.dumps({"deps_root": "deps"}, ensure_ascii=False, indent=2),
                encoding="utf-8",
            )
            older_perf.parent.mkdir(parents=True)
            latest_perf.parent.mkdir(parents=True)
            older_perf.write_text("", encoding="utf-8")
            latest_perf.write_text("", encoding="utf-8")

            resolved = exporter.resolve_perf_db_path_with_repo(Path("perf.db"), step=None, repo_root=repo_root)

            self.assertEqual(resolved, latest_perf.resolve())

    def test_resolve_perf_db_path_can_fallback_to_recursive_search_root_scan(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "repo"
            deps_root = repo_root / "deps"
            perf_db = (
                deps_root
                / "sources"
                / "ArkAnalyzer-HapRay"
                / "perf_testing"
                / "output"
                / "reports"
                / "20260409110000"
                / "case_b"
                / "hiperf"
                / "step2"
                / "perf.db"
            )
            (repo_root / "configs").mkdir(parents=True)
            (repo_root / "configs" / "config.json").write_text(
                json.dumps({"deps_root": "deps"}, ensure_ascii=False, indent=2),
                encoding="utf-8",
            )
            perf_db.parent.mkdir(parents=True)
            perf_db.write_text("", encoding="utf-8")

            resolved = exporter.resolve_perf_db_path_with_repo(Path("perf.db"), step=2, repo_root=repo_root)

            self.assertEqual(resolved, perf_db.resolve())

    def test_resolve_perf_db_path_missing_auto_discovery_includes_diagnostics(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "repo"
            (repo_root / "configs").mkdir(parents=True)
            (repo_root / "configs" / "config.json").write_text(
                json.dumps({"deps_root": "deps"}, ensure_ascii=False, indent=2),
                encoding="utf-8",
            )

            with self.assertRaises(FileNotFoundError) as ctx:
                exporter.resolve_perf_db_path_with_repo(Path("perf.db"), step=None, repo_root=repo_root)

            message = str(ctx.exception)
            self.assertIn("Auto-discovery summary:", message)
            self.assertIn("No original Hapray perf.db files were found.", message)
            self.assertIn("deps/sources/ArkAnalyzer-HapRay/perf_testing/reports", message)

    def test_resolve_perf_db_path_accepts_case_dir_with_step(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            perf_db = case_dir / "hiperf" / "step2" / "perf.db"
            perf_db.parent.mkdir(parents=True)
            perf_db.write_text("", encoding="utf-8")

            self.assertEqual(
                exporter.resolve_perf_db_path(case_dir, step=2),
                perf_db.resolve(),
            )

    def test_output_path_for_perf_db_defaults_to_project_artifacts(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "repo"
            perf_db = (
                repo_root
                / "deps"
                / "sources"
                / "ArkAnalyzer-HapRay"
                / "perf_testing"
                / "reports"
                / "20260409141549"
                / "PerfLoad_Manual_demo"
                / "hiperf"
                / "step1"
                / "perf.db"
            )
            perf_db.parent.mkdir(parents=True)
            perf_db.write_text("", encoding="utf-8")

            original_default_repo_root = exporter.default_repo_root
            try:
                exporter.default_repo_root = lambda: repo_root
                output_path = exporter.output_path_for_perf_db(perf_db, "")
            finally:
                exporter.default_repo_root = original_default_repo_root

            self.assertEqual(
                output_path,
                (
                    repo_root
                    / "artifacts"
                    / "hapray_timeline_trace"
                    / "20260409141549"
                    / "PerfLoad_Manual_demo"
                    / "hiperf"
                    / "step1"
                    / "hapray_timeline_trace.json"
                ).resolve(),
            )

    def test_export_timeline_trace_artifact_returns_resolved_output_metadata(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            perf_db = Path(temp_dir) / "perf.db"
            create_minimal_perf_db(perf_db)
            request = exporter.TimelineExportRequest(
                input_path=perf_db,
                output_path=str(Path(temp_dir) / "out.json"),
                event_name="instructions",
                thread_ids=(101,),
                merge_samples=False,
            )

            result = exporter.export_timeline_trace_artifact(request)

            self.assertEqual(result.perf_db_path, perf_db.resolve())
            self.assertEqual(result.output_path, (Path(temp_dir) / "out.json").resolve())
            self.assertEqual(result.trace_obj["metadata"]["eventName"], "instructions")
            self.assertTrue(result.output_path.is_file())

    def test_load_callchains_deduplicates_repeated_perf_files_rows(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            perf_db = Path(temp_dir) / "perf.db"
            create_minimal_perf_db(perf_db)
            conn = sqlite3.connect(str(perf_db))
            try:
                conn.executemany(
                    "INSERT INTO perf_files(file_id, path) VALUES(?, ?)",
                    [
                        (1, "/duplicate/root.so"),
                        (2, "/duplicate/leaf.so"),
                    ],
                )
                conn.commit()
                callchains = exporter.load_callchains(conn, [200, 201])
            finally:
                conn.close()

            self.assertEqual(len(callchains[200]), 2)
            self.assertEqual(len(callchains[201]), 2)
            self.assertEqual(callchains[200][0].file_id, 2)
            self.assertEqual(callchains[200][1].file_id, 1)

    def test_export_perf_db_to_timeline_trace_emits_metadata_and_merged_callstacks(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            perf_db = temp_path / "perf.db"
            output_path = temp_path / "trace.json"
            create_minimal_perf_db(perf_db)

            trace_obj = exporter.export_perf_db_to_timeline_trace(perf_db, output_path)
            trace_events = trace_obj["traceEvents"]
            begin_names = [event["name"] for event in trace_events if event.get("ph") == "B"]

            self.assertTrue(output_path.is_file())
            self.assertEqual(trace_obj["metadata"]["eventName"], "instructions")
            self.assertIn("process_name", [event["name"] for event in trace_events if event.get("ph") == "M"])
            self.assertIn("thread_name", [event["name"] for event in trace_events if event.get("ph") == "M"])
            self.assertEqual(begin_names.count("rootLoop"), 1)
            self.assertEqual(begin_names.count("leafWork"), 1)
            self.assertEqual(begin_names.count("handleInput"), 1)
            self.assertEqual(begin_names.count("dispatchFrame"), 1)
            self.assertEqual(trace_obj["metadata"]["intervalCount"], 2)

            parsed = json.loads(output_path.read_text(encoding="utf-8"))
            self.assertEqual(parsed["metadata"]["eventName"], "instructions")

    def test_export_perf_db_to_timeline_trace_can_filter_specific_event_name(self) -> None:
        exporter = load_module("export_hapray_timeline_trace", "src/arkts_migration_visualizer/collect/export_hapray_timeline_trace.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            perf_db = temp_path / "perf.db"
            output_path = temp_path / "trace_cycles.json"
            create_minimal_perf_db(perf_db)

            trace_obj = exporter.export_perf_db_to_timeline_trace(
                perf_db,
                output_path,
                event_name="cpu-cycles",
            )

            self.assertEqual(trace_obj["metadata"]["eventName"], "cpu-cycles")
            self.assertEqual(trace_obj["metadata"]["intervalCount"], 1)


if __name__ == "__main__":
    unittest.main()
