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
import pathlib
import re
import shutil
import tempfile
import unittest

from src.log import configure_logger, reset_logger
from src.metadata import (
    AppMetadataFile,
    AppMetadata,
    ArtifactMetadata,
    ArtifactMetadataFile,
)
from src.report import (
    AppSnapshotLabelPair,
    SUMMARY_STATISTICS,
    SummaryRow,
    average_reports,
    generate_reports,
    write_breakdown_csv,
    write_summary_csv,
)
from src.result import ResultStore
from src.smaps import MemProfile, SmapsSummary
from src.smaps import SUMMARY_METRICS


_FIXTURES = pathlib.Path(__file__).parent.joinpath("fixtures")
_REMOTE_OUT_DIR = pathlib.PurePosixPath("/data/local/tmp/memmem-run")


def _store(out_dir: pathlib.Path) -> ResultStore:
    return ResultStore(out_dir, _REMOTE_OUT_DIR)


class ReportTest(unittest.TestCase):
    def test_generate_reports_writes_summary_and_breakdowns_in_metadata_order(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            out_dir = self._create_result_tree(pathlib.Path(directory))

            generate_reports(_store(out_dir))

            with out_dir.joinpath("summary.csv").open(encoding="utf-8", newline="") as stream:
                rows = list(csv.reader(stream))
            self.assertEqual(len(rows), 4)
            self.assertEqual(
                rows[0],
                [
                    "app_label",
                    "snapshot_label",
                    "size_kb",
                    "rss_kb",
                    "pss_kb",
                    "referenced_kb",
                    "shared_kb",
                    "private_kb",
                    "swap_kb",
                    "swap_pss_kb",
                    "anonymous_kb",
                ],
            )
            self.assertEqual(rows[1][:2], ["App", "after_start"])
            self.assertEqual(rows[2][:2], ["Other", "after_start"])
            self.assertEqual(rows[3][:2], ["App", "after_wait"])
            self.assertTrue(out_dir.joinpath(
                "breakdowns", "after_start", "App-after_start.csv").is_file())
            self.assertTrue(out_dir.joinpath(
                "breakdowns", "after_start", "Other-after_start.csv").is_file())
            self.assertTrue(out_dir.joinpath(
                "breakdowns", "after_wait", "App-after_wait.csv").is_file())
            self.assertTrue(out_dir.joinpath(
                "plots", "App", "App-size.svg").is_file())
            self.assertTrue(out_dir.joinpath(
                "plots", "Other", "Other-rss.svg").is_file())
            self.assertFalse(out_dir.joinpath("plots", "size.svg").exists())

    def test_generate_reports_skips_missing_expected_snapshot(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            out_dir = self._create_result_tree(pathlib.Path(directory))
            out_dir.joinpath("snapshots", "after_start",
                             "Other-after_start.smaps").unlink()

            generate_reports(_store(out_dir))

            with out_dir.joinpath("summary.csv").open(encoding="utf-8", newline="") as stream:
                rows = list(csv.reader(stream))
            self.assertEqual([row[:2] for row in rows[1:]], [
                ["App", "after_start"],
                ["App", "after_wait"],
            ])

    def test_generate_reports_smaps_filter_skips_non_matching_snapshots(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            out_dir = self._create_result_tree(pathlib.Path(directory))
            shutil.copyfile(
                _FIXTURES.joinpath("smaps_shell.smaps"),
                out_dir.joinpath(
                    "snapshots", "after_start", "App-after_start.smaps"),
            )

            generate_reports(_store(out_dir), re.compile(r".*libtinfo"))

            with out_dir.joinpath("summary.csv").open(
                    encoding="utf-8", newline="") as stream:
                rows = list(csv.reader(stream))
            self.assertEqual(
                [[row[0], row[1]] for row in rows[1:]],
                [["App", "after_start"]],
            )
            self.assertTrue(out_dir.joinpath(
                "breakdowns", "after_start", "App-after_start.csv").is_file())
            self.assertTrue(out_dir.joinpath(
                "plots", "App", "App-size.svg").is_file())
            self.assertFalse(out_dir.joinpath(
                "plots", "Other", "Other-rss.svg").exists())
            self.assertFalse(out_dir.joinpath(
                "breakdowns", "after_start",
                "Other-after_start.csv").exists())

    def test_generate_reports_smaps_filter_warns_once_per_filtered_file(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            out_dir = self._create_result_tree(pathlib.Path(directory))
            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                configure_logger("warn")

                generate_reports(_store(out_dir), re.compile(r"nomatch"))

                reset_logger()

            output = stdout.getvalue()
            self.assertEqual(output.count(
                "warning: no values parsed from the"), 3)
            self.assertIn("snapshots/after_start/App-after_start.smaps",
                          output)

    def test_write_breakdown_csv(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory).joinpath("breakdown.csv")
            summary = SmapsSummary(
                total=MemProfile(3, 4, 5, 6, 7, 8, 9, 0, 10),
                breakdown={
                    "b": MemProfile(1, 2, 3, 4, 5, 6, 7, 0, 8),
                    "a": MemProfile(2, 2, 2, 2, 2, 2, 2, 0, 2),
                    "c": MemProfile(2, 1, 1, 1, 1, 1, 1, 0, 1),
                },
            )

            write_breakdown_csv(summary, path)

            with path.open(encoding="utf-8", newline="") as stream:
                rows = list(csv.reader(stream))
            self.assertEqual(
                rows,
                [
                    ["tag", *SUMMARY_METRICS],
                    ["a", "2", "2", "2", "2", "2", "2", "2", "0", "2"],
                    ["c", "2", "1", "1", "1", "1", "1", "1", "0", "1"],
                    ["b", "1", "2", "3", "4", "5", "6", "7", "0", "8"],
                ],
            )

    def _create_result_tree(self, root: pathlib.Path) -> pathlib.Path:
        return _create_result_tree(root)


def _create_result_tree(root: pathlib.Path) -> pathlib.Path:
    out_dir = root.joinpath("out")
    store = _store(out_dir)
    store.local_out_dir().mkdir(parents=True)
    store.local_snapshots_dir().mkdir()
    store.local_screenshots_dir().mkdir()
    store.local_screenshot_path("visual").write_text(
        "not smaps", encoding="utf-8")
    store.local_artifact_metadata_path(store.local_screenshots_dir()).write_text(
        ArtifactMetadataFile(
            artifacts=[
                ArtifactMetadata(
                    label="visual",
                    timestamp="1700000000000000002",
                )
            ]
        ).model_dump_json(indent=2)
        + "\n",
        encoding="utf-8",
    )
    store.local_app_metadata_path().write_text(
        AppMetadataFile(
            apps=[
                AppMetadata(
                    pid=123, label="App", bundle="com.example", ability="EntryAbility"),
                AppMetadata(pid=456, label="Other",
                            bundle="com.other", ability="EntryAbility"),
            ]
        ).model_dump_json(indent=2)
        + "\n",
        encoding="utf-8",
    )
    store.local_artifact_metadata_path(store.local_snapshots_dir()).write_text(
        ArtifactMetadataFile(
            artifacts=[
                ArtifactMetadata(
                    label="after_start",
                    timestamp="1700000000000000000",
                ),
                ArtifactMetadata(
                    label="after_wait",
                    timestamp="1700000000000000001",
                ),
            ]
        ).model_dump_json(indent=2)
        + "\n",
        encoding="utf-8",
    )
    snapshots = [
        store.local_snapshot_path("after_start", "App"),
        store.local_snapshot_path("after_start", "Other"),
        store.local_snapshot_path("after_wait", "App"),
    ]
    for snapshot_path in snapshots:
        snapshot_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(_FIXTURES.joinpath(
            "smaps_self.smaps"), snapshot_path)
    return out_dir


class AverageReportsTest(unittest.TestCase):
    def test_cell_wise_statistics_across_identical_rows(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [300, 400]),
                ]),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            self.assertEqual(
                rows[0],
                ["app_label", "snapshot_label", "n_samples"]
                + [f"{metric}_{stat}" for metric in SUMMARY_METRICS for stat in SUMMARY_STATISTICS],
            )
            self.assertEqual(len(rows), 2)
            data = rows[1]
            self.assertEqual(data[:3], ["App", "after_start", "2"])
            size_idx = _column_index(rows[0], "size_kb")
            self.assertEqual(data[size_idx + 0], "200.00")
            self.assertEqual(data[size_idx + 1], "173.21")
            self.assertEqual(data[size_idx + 2], "200.00")
            self.assertEqual(data[size_idx + 3], "141.42")
            self.assertEqual(data[size_idx + 4], "100")
            self.assertEqual(data[size_idx + 5], "300")

    def test_geomean_collapses_on_zero(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [0, 1]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [100, 1]),
                ]),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            size_idx = _column_index(rows[0], "size_kb")
            rss_idx = _column_index(rows[0], "rss_kb")
            self.assertEqual(rows[1][size_idx + 1], "0.00")
            self.assertEqual(rows[1][rss_idx + 1], "1.00")

    def test_std_is_zero_for_single_sample(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            size_idx = _column_index(rows[0], "size_kb")
            self.assertEqual(rows[1][:3], ["App", "after_start", "1"])
            self.assertEqual(rows[1][size_idx + 0], "100.00")
            self.assertEqual(rows[1][size_idx + 1], "100.00")
            self.assertEqual(rows[1][size_idx + 2], "100.00")
            self.assertEqual(rows[1][size_idx + 3], "0.00")
            self.assertEqual(rows[1][size_idx + 4], "100")
            self.assertEqual(rows[1][size_idx + 5], "100")

    def test_missing_iteration_summary_is_skipped(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            present = _write_iteration(root, "iteration_0", [
                ("App", "after_start", [
                 100, 200, 100, 200, 100, 200, 100, 200]),
            ])
            missing = root.joinpath("iteration_1")
            missing.mkdir()
            stores = [present, _store(missing)]
            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                configure_logger("warn")

            average_reports(run_out, stores)

            self.assertIn(
                "warning: skipping iteration without summary.csv", stdout.getvalue())
            self.assertNotIn("warning: warning:", stdout.getvalue())
            reset_logger()
            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            self.assertEqual(rows[1][:3], ["App", "after_start", "1"])

    def test_average_reports_treats_filtered_snapshot_as_absent(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            iteration_0 = _write_iteration(root, "iteration_0", [
                ("App", "after_start",
                 [100, 100, 100, 100, 100, 100, 100, 100]),
                ("App", "after_wait",
                 [200, 200, 200, 200, 200, 200, 200, 200]),
            ])
            filtered_out = _create_result_tree(root.joinpath("iteration_1"))
            generate_reports(_store(filtered_out), re.compile(r"nomatch"))

            average_reports(run_out, [iteration_0, _store(filtered_out)])

            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            self.assertEqual(len(rows), 3)
            self.assertEqual(rows[1][:3], ["App", "after_start", "1"])
            self.assertEqual(rows[2][:3], ["App", "after_wait", "1"])
            size_idx = _column_index(rows[0], "size_kb")
            self.assertEqual(rows[1][size_idx + 0], "100.00")
            self.assertEqual(rows[2][size_idx + 0], "200.00")

    def test_differing_snapshot_rows_merged_via_n_samples(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                    ("Other", "after_start", [10, 20]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [300, 400]),
                ]),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            self.assertEqual(len(rows), 3)
            self.assertEqual(rows[1][:3], ["App", "after_start", "2"])
            self.assertEqual(rows[2][:3], ["Other", "after_start", "1"])

    def test_no_iteration_summary_files_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _store(root.joinpath("iteration_0")),
                _store(root.joinpath("iteration_1")),
            ]
            stores[0].local_out_dir().mkdir()
            stores[1].local_out_dir().mkdir()

            with self.assertRaises(RuntimeError):
                average_reports(run_out, stores)

    def test_rows_ordered_by_first_appearance(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("Other", "after_start", [10, 20]),
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [300, 400]),
                ]),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath("summary", "summary.csv"))
            self.assertEqual([row[0] for row in rows[1:]], ["Other", "App"])

    def test_unexpected_metric_header_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            store = _write_iteration(root, "iteration_0", [
                ("App", "after_start", [
                 100, 200, 100, 200, 100, 200, 100, 200]),
            ])
            with store.local_summary_path().open("w", encoding="utf-8", newline="") as stream:
                writer = csv.writer(stream)
                writer.writerow(
                    ["app_label", "snapshot_label", "size_kb", "rss_kb"])
                writer.writerow(["App", "after_start", "100", "200"])

            with self.assertRaises(ValueError):
                average_reports(run_out, [store])

    def test_unexpected_row_width_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            store = _write_iteration(root, "iteration_0", [
                ("App", "after_start", [
                 100, 200, 100, 200, 100, 200, 100, 200]),
            ])
            with store.local_summary_path().open("w", encoding="utf-8", newline="") as stream:
                writer = csv.writer(stream)
                writer.writerow(
                    ["app_label", "snapshot_label", *SUMMARY_METRICS])
                writer.writerow(["App", "after_start", "100"])

            with self.assertRaises(ValueError):
                average_reports(run_out, [store])

    def test_filtering_discards_highest_scoring_iteration(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000]),
                ]),
            ]

            average_reports(run_out, stores)

            outliers = _read_csv(run_out.joinpath("summary", "outliers.csv"))
            self.assertEqual(outliers[0], ["discarded_iteration"])
            self.assertEqual(outliers[1:], [["iteration_4"]])
            filtered = _read_csv(
                run_out.joinpath("summary", "summary_filtered.csv"))
            size_idx = _column_index(filtered[0], "size_kb")
            self.assertEqual(filtered[1][size_idx + 0], "100.00")

    def test_filtering_writes_raw_and_filtered_when_no_discard(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
            ]

            average_reports(run_out, stores)

            outliers = _read_csv(run_out.joinpath("summary", "outliers.csv"))
            self.assertEqual(outliers, [["discarded_iteration"]])
            raw = _read_csv(run_out.joinpath("summary", "summary.csv"))
            filtered = _read_csv(
                run_out.joinpath("summary", "summary_filtered.csv"))
            self.assertEqual(filtered, raw)

    def test_retention_limits_prevent_discard_below_minimum(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [500, 500]),
                ]),
            ]

            average_reports(run_out, stores)

            outliers = _read_csv(run_out.joinpath("summary", "outliers.csv"))
            self.assertEqual(outliers, [["discarded_iteration"]])

    def test_zero_median_cells_contribute_zero_deviation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 0, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 0, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 0, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 0, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     9000, 500, 9000, 9000, 9000, 9000, 9000, 9000]),
                ]),
            ]

            average_reports(run_out, stores)

            outliers = _read_csv(run_out.joinpath("summary", "outliers.csv"))
            self.assertEqual(outliers[1:], [["iteration_4"]])

    def test_discarded_iteration_excluded_from_every_metric(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 200, 100, 200, 100, 200, 100, 200]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     5000, 7000, 5000, 7000, 5000, 7000, 5000, 7000]),
                ]),
            ]

            average_reports(run_out, stores)

            filtered = _read_csv(
                run_out.joinpath("summary", "summary_filtered.csv"))
            size_idx = _column_index(filtered[0], "size_kb")
            rss_idx = _column_index(filtered[0], "rss_kb")
            self.assertEqual(filtered[1][size_idx + 0], "100.00")
            self.assertEqual(filtered[1][rss_idx + 0], "200.00")
            self.assertEqual(filtered[1][size_idx + 5], "100")
            self.assertEqual(filtered[1][rss_idx + 5], "200")

    def test_iteration_score_ties_use_run_order(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [500, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [500, 100]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
            ]

            average_reports(run_out, stores)

            outliers = _read_csv(run_out.joinpath("summary", "outliers.csv"))
            self.assertEqual(outliers[1:], [["iteration_0"]])

    def test_missing_rows_ignored_for_iteration_scoring(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                    ("Other", "after_start", [
                     50, 50, 50, 50, 50, 50, 50, 50, 50]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                    ("Other", "after_start", [
                     50, 50, 50, 50, 50, 50, 50, 50, 50]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                    ("Other", "after_start", [
                     50, 50, 50, 50, 50, 50, 50, 50, 50]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000]),
                    ("Other", "after_start", [
                     50, 50, 50, 50, 50, 50, 50, 50, 50]),
                ]),
            ]

            average_reports(run_out, stores)

            outliers = _read_csv(run_out.joinpath("summary", "outliers.csv"))
            self.assertEqual(outliers[1:], [["iteration_4"]])
            filtered = _read_csv(
                run_out.joinpath("summary", "summary_filtered.csv"))
            self.assertEqual(
                [row[:3] for row in filtered[1:]],
                [["App", "after_start", "4"], ["Other", "after_start", "3"]],
            )

    def test_row_only_in_discarded_iteration_omitted_from_filtered(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000]),
                    ("Lonely", "after_wait", [
                     9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000]),
                ]),
            ]

            average_reports(run_out, stores)

            filtered = _read_csv(
                run_out.joinpath("summary", "summary_filtered.csv"))
            self.assertNotIn(
                ["Lonely", "after_wait"],
                [row[:2] for row in filtered[1:]],
            )

    def test_averaged_breakdown_tag_union_and_n_samples(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "a": [100, 200, 100, 200, 100, 200, 100, 200],
                        "b": [300, 300, 300, 300, 300, 300, 300, 300],
                    },
                }),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "a": [500, 500, 500, 500, 500, 500, 500, 500],
                        "c": [700, 700, 700, 700, 700, 700, 700, 700],
                    },
                }),
            ]

            average_reports(run_out, stores)

            path = run_out.joinpath(
                "summary", "breakdowns", "after_start", "App-after_start.csv")
            rows = _read_csv(path)
            self.assertEqual(
                rows[0],
                ["tag", "n_samples"]
                + [f"{metric}_{stat}" for metric in SUMMARY_METRICS for stat in SUMMARY_STATISTICS],
            )
            self.assertEqual(rows[1][:2], ["c", "1"])
            self.assertEqual(rows[2][:2], ["a", "2"])
            self.assertEqual(rows[3][:2], ["b", "1"])
            size_idx = _column_index(rows[0], "size_kb")
            self.assertEqual(rows[2][size_idx + 0], "300.00")
            self.assertEqual(rows[2][size_idx + 3], "282.84")
            self.assertEqual(rows[3][size_idx + 0], "300.00")
            self.assertEqual(rows[1][size_idx + 0], "700.00")

    def test_averaged_breakdown_sorted_by_size_mean_descending(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "low": [100, 100, 100, 100, 100, 100, 100, 100],
                        "high": [900, 900, 900, 900, 900, 900, 900, 900],
                    },
                }),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "low": [100, 100, 100, 100, 100, 100, 100, 100],
                        "high": [900, 900, 900, 900, 900, 900, 900, 900],
                    },
                }),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath(
                "summary", "breakdowns", "after_start", "App-after_start.csv"))
            self.assertEqual(
                [row[0] for row in rows[1:]],
                ["high", "low"],
            )

    def test_averaged_breakdown_missing_file_skipped(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "a": [100, 100, 100, 100, 100, 100, 100, 100],
                    },
                }),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
            ]

            average_reports(run_out, stores)

            path = run_out.joinpath(
                "summary", "breakdowns", "after_start", "App-after_start.csv")
            rows = _read_csv(path)
            self.assertEqual(rows[1][:2], ["a", "1"])
            size_idx = _column_index(rows[0], "size_kb")
            self.assertEqual(rows[1][size_idx + 3], "0.00")

    def test_averaged_breakdown_omits_discarded_only_pair(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "after_start", [
                     9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000]),
                ], breakdowns={
                    ("after_wait", "Lonely"): {
                        "tag": [9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000],
                    },
                }),
            ]

            average_reports(run_out, stores)

            self.assertFalse(run_out.joinpath(
                "summary", "breakdowns", "after_wait",
                "Lonely-after_wait.csv").exists())

    def test_averaged_breakdown_geomean_zero_collapse(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "a": [0, 100, 100, 100, 100, 100, 100, 100],
                    },
                }),
                _write_iteration(root, "iteration_1", [
                    ("App", "after_start", [
                     100, 100, 100, 100, 100, 100, 100, 100]),
                ], breakdowns={
                    ("after_start", "App"): {
                        "a": [900, 100, 100, 100, 100, 100, 100, 100],
                    },
                }),
            ]

            average_reports(run_out, stores)

            rows = _read_csv(run_out.joinpath(
                "summary", "breakdowns", "after_start", "App-after_start.csv"))
            size_idx = _column_index(rows[0], "size_kb")
            self.assertEqual(rows[1][size_idx + 0], "450.00")
            self.assertEqual(rows[1][size_idx + 1], "0.00")

    def test_invalid_breakdown_header_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            store = _write_iteration(root, "iteration_0", [
                ("App", "after_start", [
                 100, 100, 100, 100, 100, 100, 100, 100]),
            ], breakdowns={
                ("after_start", "App"): {
                    "a": [100, 100, 100, 100, 100, 100, 100, 100],
                },
            })
            with store.local_breakdown_path("after_start", "App").open(
                "w", encoding="utf-8", newline=""
            ) as stream:
                writer = csv.writer(stream)
                writer.writerow(["tag", "Size_total_for_tag"])
                writer.writerow(["a", "100"])

            with self.assertRaises(ValueError):
                average_reports(run_out, [store])

    def test_invalid_breakdown_row_width_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            store = _write_iteration(root, "iteration_0", [
                ("App", "after_start", [
                 100, 100, 100, 100, 100, 100, 100, 100]),
            ], breakdowns={
                ("after_start", "App"): {
                    "a": [100, 100, 100, 100, 100, 100, 100, 100],
                },
            })
            with store.local_breakdown_path("after_start", "App").open(
                "w", encoding="utf-8", newline=""
            ) as stream:
                writer = csv.writer(stream)
                writer.writerow(["tag", *SUMMARY_METRICS])
                writer.writerow(["a", "100"])

            with self.assertRaises(ValueError):
                average_reports(run_out, [store])

    def test_average_reports_generates_plots_from_retained_rows(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            run_out = root.joinpath("run")
            run_out.mkdir()
            stores = [
                _write_iteration(root, "iteration_0", [
                    ("App", "start", [100, 100, 100,
                     100, 100, 100, 100, 100, 100]),
                    ("App", "later", [200, 200, 200,
                     200, 200, 200, 200, 200, 200]),
                ]),
                _write_iteration(root, "iteration_1", [
                    ("App", "start", [100, 100, 100,
                     100, 100, 100, 100, 100, 100]),
                    ("App", "later", [200, 200, 200,
                     200, 200, 200, 200, 200, 200]),
                ]),
                _write_iteration(root, "iteration_2", [
                    ("App", "start", [100, 100, 100,
                     100, 100, 100, 100, 100, 100]),
                    ("App", "later", [200, 200, 200,
                     200, 200, 200, 200, 200, 200]),
                ]),
                _write_iteration(root, "iteration_3", [
                    ("App", "start", [100, 100, 100,
                     100, 100, 100, 100, 100, 100]),
                    ("App", "later", [200, 200, 200,
                     200, 200, 200, 200, 200, 200]),
                ]),
                _write_iteration(root, "iteration_4", [
                    ("App", "start", [9000, 9000, 9000,
                     9000, 9000, 9000, 9000, 9000]),
                    ("App", "discarded_only", [
                     9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000, 9000]),
                ]),
            ]
            for store in stores:
                _write_snapshot_metadata(store, {
                    "start": "1000000000",
                    "later": "2000000000",
                    "discarded_only": "3000000000",
                })

            average_reports(run_out, stores)

            path = run_out.joinpath("summary", "plots", "App", "App-size.svg")
            self.assertTrue(path.is_file())
            content = path.read_text(encoding="utf-8")
            self.assertIn("start", content)
            self.assertIn("later", content)
            self.assertNotIn("discarded_only", content)

    def test_app_snapshot_label_pair_is_shared_identity(self) -> None:
        row = SummaryRow(
            pair=AppSnapshotLabelPair("App", "start"),
            profile=MemProfile(
                size_kb=1,
                rss_kb=2,
                pss_kb=3,
                referenced_kb=4,
                shared_kb=5,
                private_kb=6,
                swap_kb=7,
                swap_pss_kb=7,
                anonymous_kb=8,
            ),
        )

        self.assertEqual(row.pair, AppSnapshotLabelPair("App", "start"))


def _write_iteration(
    root: pathlib.Path,
    name: str,
    rows: list[tuple[str, str, list[int]]],
    breakdowns: dict[tuple[str, str], dict[str, list[int]]] | None = None,
) -> ResultStore:
    out_dir = root.joinpath(name)
    out_dir.mkdir()
    store = _store(out_dir)
    summary_rows = [
        SummaryRow(
            pair=AppSnapshotLabelPair(app_label, snapshot_label),
            profile=MemProfile(
                size_kb=_metric(values, 0),
                rss_kb=_metric(values, 1),
                pss_kb=_metric(values, 2),
                referenced_kb=_metric(values, 3),
                shared_kb=_metric(values, 4),
                private_kb=_metric(values, 5),
                swap_kb=_metric(values, 6),
                anonymous_kb=_metric(values, 7),
                swap_pss_kb=_metric(values, 8),
            ),
        )
        for app_label, snapshot_label, values in rows
    ]
    write_summary_csv(summary_rows, store.local_summary_path())
    if breakdowns:
        for (snapshot_label, app_label), tags in breakdowns.items():
            profiles = {
                tag: MemProfile(
                    size_kb=_metric(values, 0),
                    rss_kb=_metric(values, 1),
                    pss_kb=_metric(values, 2),
                    referenced_kb=_metric(values, 3),
                    shared_kb=_metric(values, 4),
                    private_kb=_metric(values, 5),
                    swap_kb=_metric(values, 6),
                    anonymous_kb=_metric(values, 7),
                    swap_pss_kb=_metric(values, 8),
                )
                for tag, values in tags.items()
            }
            write_breakdown_csv(
                SmapsSummary(
                    total=MemProfile(0, 0, 0, 0, 0, 0, 0, 0, 0),
                    breakdown=profiles,
                ),
                store.local_breakdown_path(snapshot_label, app_label),
            )
    return store


def _metric(values: list[int], index: int) -> int:
    if index < len(values):
        return values[index]
    return 0


def _write_snapshot_metadata(
        store: ResultStore, timestamps: dict[str, str]) -> None:
    snapshots_dir = store.local_snapshots_dir()
    snapshots_dir.mkdir(parents=True, exist_ok=True)
    metadata = ArtifactMetadataFile(artifacts=[
        ArtifactMetadata(label=label, timestamp=timestamp)
        for label, timestamp in timestamps.items()
    ])
    store.local_artifact_metadata_path(snapshots_dir).write_text(
        metadata.model_dump_json(), encoding="utf-8")


def _read_csv(path: pathlib.Path) -> list[list[str]]:
    with path.open(encoding="utf-8", newline="") as stream:
        return list(csv.reader(stream))


def _column_index(header: list[str], metric: str) -> int:
    return header.index(f"{metric}_mean")


if __name__ == "__main__":
    unittest.main()
