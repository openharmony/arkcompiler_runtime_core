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
import io
import pathlib
import tempfile
import unittest

from src.log import configure_logger, reset_logger
from src.plot import METRICS, generate_averaged_plots, generate_plots
from src.report import (
    AppSnapshotLabelPair,
    AveragedRow,
    AveragedStatistics,
    SummaryRow,
)
from src.result import ResultStore
from src.metadata import ArtifactMetadata, ArtifactMetadataFile
from src.smaps import SUMMARY_METRICS, MemProfile


class PlotTest(unittest.TestCase):
    def test_generate_plots_writes_linear_and_log_svg_per_app_metric(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            store = ResultStore(root, pathlib.PurePosixPath("/remote"))
            _write_snapshot_metadata(
                store, {"start": "1000000000", "later": "2000000000"})
            rows = [
                _row("App", "later", size_kb=10),
                _row("App", "start", size_kb=5),
            ]

            generate_plots(rows, store)

            self.assertTrue(store.local_plot_path("App", "size").is_file())
            self.assertTrue(store.local_plot_path("App", "rss").is_file())
            self.assertTrue(
                store.local_plot_path("App", "swap_pss").is_file())
            self.assertFalse(root.joinpath("plots", "size.svg").exists())
            content = store.local_plot_path(
                "App", "size").read_text(encoding="utf-8")
            self.assertIn("start", content)
            self.assertIn("later", content)

    def test_snapshot_labels_are_unwrapped_x_axis_ticks_with_grid(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store = ResultStore(pathlib.Path(directory),
                                pathlib.PurePosixPath("/remote"))
            _write_snapshot_metadata(store, {"very_long_snapshot_label": "1000000000",
                                             "later_snapshot_label": "2000000000"})
            rows = [
                _row("App", "very_long_snapshot_label", size_kb=5),
                _row("App", "later_snapshot_label", size_kb=10),
            ]

            generate_plots(rows, store)

            content = store.local_plot_path(
                "App", "size").read_text(encoding="utf-8")
            self.assertIn("very_long_snapshot_label", content)
            self.assertIn("later_snapshot_label", content)
            self.assertIn("rotate", content)
            self.assertIn("stroke-dasharray", content)
            self.assertGreaterEqual(content.count('<g id="ytick_'), 10)

    def test_negative_metric_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store = ResultStore(pathlib.Path(directory),
                                pathlib.PurePosixPath("/remote"))
            _write_snapshot_metadata(store, {"bad": "1000000000"})

            with self.assertRaisesRegex(RuntimeError, "non-negative"):
                generate_plots(
                    [_row("App", "bad", size_kb=-1)], store)

    def test_missing_snapshot_metadata_skips_plots(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store = ResultStore(pathlib.Path(directory),
                                pathlib.PurePosixPath("/remote"))

            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                configure_logger("warn")

            generate_plots([_row("App", "bad")], store)

            self.assertIn(
                "warning: skipping plot generation without snapshot metadata",
                stdout.getvalue(),
            )
            reset_logger()
            self.assertFalse(store.local_plot_path("App", "size").is_file())

    def test_generate_averaged_plots_writes_svg_with_error_bars(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store = ResultStore(pathlib.Path(directory),
                                pathlib.PurePosixPath("/remote"))
            rows = [
                _averaged_row("App", "later", geomean=10.0, std=2.0),
                _averaged_row("App", "start", geomean=5.0, std=1.0),
            ]

            generate_averaged_plots(rows, store, {
                "start": 1000000000,
                "later": 2000000000,
            })

            self.assertTrue(store.local_plot_path("App", "size").is_file())
            self.assertTrue(store.local_plot_path("App", "rss").is_file())
            self.assertTrue(
                store.local_plot_path("App", "swap_pss").is_file())
            content = store.local_plot_path(
                "App", "size").read_text(encoding="utf-8")
            self.assertIn("start", content)
            self.assertIn("later", content)
            self.assertIn("LineCollection", content)

    def test_generate_averaged_plots_uses_only_present_snapshot_labels(
            self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store = ResultStore(pathlib.Path(directory),
                                pathlib.PurePosixPath("/remote"))
            rows = [
                _averaged_row("App", "start", geomean=5.0, std=1.0),
                _averaged_row("App", "missing", geomean=9.0, std=1.0),
            ]

            generate_averaged_plots(rows, store, {
                "start": 1000000000,
            })

            content = store.local_plot_path(
                "App", "size").read_text(encoding="utf-8")
            self.assertIn("start", content)
            self.assertNotIn("missing", content)

    def test_generate_averaged_plots_skips_without_timestamps(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            store = ResultStore(pathlib.Path(directory),
                                pathlib.PurePosixPath("/remote"))

            stdout = io.StringIO()
            with contextlib.redirect_stdout(stdout):
                configure_logger("warn")

            generate_averaged_plots(
                [_averaged_row("App", "start")], store, {})

            self.assertIn(
                "warning: skipping averaged plot generation without snapshot metadata",
                stdout.getvalue(),
            )
            reset_logger()
            self.assertFalse(store.local_plot_path("App", "size").is_file())

    def test_metrics_derive_from_summary_metrics(self) -> None:
        self.assertEqual(
            [metric for metric, _attribute in METRICS],
            [metric_name.removesuffix("_kb")
             for metric_name in SUMMARY_METRICS],
        )
        self.assertIn(("swap_pss", "swap_pss_kb"), METRICS)


def _write_snapshot_metadata(
        store: ResultStore, timestamps: dict[str, str]) -> None:
    snapshots_dir = store.local_snapshots_dir()
    snapshots_dir.mkdir(parents=True)
    metadata = ArtifactMetadataFile(artifacts=[
        ArtifactMetadata(label=label, timestamp=timestamp)
        for label, timestamp in timestamps.items()
    ])
    store.local_artifact_metadata_path(snapshots_dir).write_text(
        metadata.model_dump_json(), encoding="utf-8")


def _row(
    app_label: str,
    snapshot_label: str,
    size_kb: int = 1,
) -> SummaryRow:
    return SummaryRow(
        pair=AppSnapshotLabelPair(app_label, snapshot_label),
        profile=MemProfile(
            size_kb=size_kb,
            rss_kb=2,
            pss_kb=3,
            referenced_kb=4,
            shared_kb=5,
            private_kb=6,
            swap_kb=7,
            swap_pss_kb=8,
            anonymous_kb=9,
        ),
    )


def _averaged_row(
    app_label: str,
    snapshot_label: str,
    geomean: float = 1.0,
    std: float = 0.0,
) -> AveragedRow:
    statistics = AveragedStatistics(
        mean=geomean,
        geomean=geomean,
        median=geomean,
        std=std,
        min=1,
        max=2,
    )
    return AveragedRow(
        pair=AppSnapshotLabelPair(app_label, snapshot_label),
        n_samples=2,
        metrics={metric: statistics for metric in SUMMARY_METRICS},
    )


if __name__ == "__main__":
    unittest.main()
