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

import csv
import dataclasses
import math
import pathlib
import re
import statistics

from src.log import get_logger
from src.metadata import AppMetadataFile, ArtifactMetadataFile
from src.plot import generate_averaged_plots, generate_plots, read_snapshot_timestamps
from src.result import ResultStore
from src.smaps import MemProfile, SmapsSummary, parse_smaps_text
from src.smaps import SUMMARY_METRICS


@dataclasses.dataclass(frozen=True)
class AveragedStatistics:
    mean: float
    geomean: float
    median: float
    std: float
    min: int
    max: int


SUMMARY_STATISTICS = [
    field.name for field in dataclasses.fields(AveragedStatistics)]

OUTLIER_TRIM_FRACTION = 0.20
OUTLIER_MIN_RETAINED_ITERATIONS = 2
OUTLIER_MIN_RETAINED_FRACTION = 0.60


@dataclasses.dataclass(frozen=True)
class AppSnapshotLabelPair:
    app_label: str
    snapshot_label: str


CellKey = tuple[AppSnapshotLabelPair, str]


@dataclasses.dataclass(frozen=True)
class SummaryRow:
    pair: AppSnapshotLabelPair
    profile: MemProfile

    @property
    def app_label(self) -> str:
        return self.pair.app_label

    @property
    def snapshot_label(self) -> str:
        return self.pair.snapshot_label


@dataclasses.dataclass(frozen=True)
class IterationSummaryTable:
    name: str
    store: ResultStore
    rows: list[SummaryRow]


@dataclasses.dataclass(frozen=True)
class AveragedRow:
    pair: AppSnapshotLabelPair
    n_samples: int
    metrics: dict[str, AveragedStatistics]

    @property
    def app_label(self) -> str:
        return self.pair.app_label

    @property
    def snapshot_label(self) -> str:
        return self.pair.snapshot_label


def write_summary_csv(rows: list[SummaryRow], path: pathlib.Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["app_label", "snapshot_label", *SUMMARY_METRICS])
        for row in rows:
            writer.writerow(
                [row.app_label, row.snapshot_label]
                + [getattr(row.profile, metric) for metric in SUMMARY_METRICS]
            )


def write_breakdown_csv(summary: SmapsSummary, path: pathlib.Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["tag", *SUMMARY_METRICS])
        for tag, profile in sorted(summary.breakdown.items(),
                                   key=lambda item: (-item[1].size_kb, item[0])):
            writer.writerow([tag] + [getattr(profile, metric)
                            for metric in SUMMARY_METRICS])


def generate_reports(store: ResultStore,
                     smaps_filter: re.Pattern[str] | None = None) -> None:
    rows: list[SummaryRow] = []
    app_metadata = AppMetadataFile.model_validate_json(
        store.local_app_metadata_path().read_text(encoding="utf-8"))
    snapshot_metadata_path = store.local_artifact_metadata_path(
        store.local_snapshots_dir())
    if snapshot_metadata_path.exists():
        snapshot_metadata = ArtifactMetadataFile.model_validate_json(
            snapshot_metadata_path.read_text(encoding="utf-8"))
    else:
        snapshot_metadata = ArtifactMetadataFile(artifacts=[])

    for snapshot in snapshot_metadata.artifacts:
        for app in app_metadata.apps:
            snapshot_path = store.local_snapshot_path(
                snapshot.label, app.label)
            if not snapshot_path.exists():
                continue
            smaps_text = snapshot_path.read_text(encoding="utf-8")
            summary = parse_smaps_text(smaps_text, smaps_filter)
            if summary is None:
                get_logger().warn(f"no values parsed from the {snapshot_path}")
                continue
            rows.append(
                SummaryRow(
                    pair=AppSnapshotLabelPair(app.label, snapshot.label),
                    profile=summary.total,
                )
            )
            write_breakdown_csv(
                summary,
                store.local_breakdown_path(snapshot.label, app.label),
            )

    write_summary_csv(rows, store.local_summary_path())
    generate_plots(rows, store)


def average_reports(
    out_dir: pathlib.Path,
    out_dirs: list[ResultStore],
) -> None:
    tables: list[IterationSummaryTable] = []
    for store in out_dirs:
        summary_path = store.local_summary_path()
        if not summary_path.exists():
            get_logger().warn(
                f"skipping iteration without summary.csv: {summary_path}")
            continue
        rows = _read_summary_rows(summary_path)
        tables.append(IterationSummaryTable(
            name=store.local_out_dir().name,
            store=store,
            rows=rows,
        ))

    if not tables:
        raise RuntimeError(f"no iteration summary.csv found under {out_dir}")

    out_dir_avg = out_dir.joinpath("summary")
    out_dir_avg.mkdir(parents=True, exist_ok=True)
    summary_store = ResultStore(out_dir_avg, pathlib.PurePosixPath("/"))
    _write_averaged_summary(out_dir_avg.joinpath("summary.csv"), tables)
    discarded = _select_discarded_iterations(tables)
    retained = [
        table for index, table in enumerate(tables)
        if index not in discarded
    ]
    retained_rows = _compute_averaged_rows(retained)
    _write_averaged_rows(
        out_dir_avg.joinpath("summary_filtered.csv"),
        retained_rows,
    )
    _write_outliers_csv(
        out_dir_avg.joinpath("outliers.csv"),
        [tables[index].name for index in sorted(discarded)],
    )

    retained_stores = [table.store for table in retained]
    if retained:
        timestamps = read_snapshot_timestamps(retained[0].store)
        generate_averaged_plots(
            retained_rows,
            summary_store,
            timestamps,
        )
    _write_averaged_breakdowns(summary_store, retained_stores)


def _write_averaged_summary(path: pathlib.Path,
                            tables: list[IterationSummaryTable]) -> None:
    _write_averaged_rows(path, _compute_averaged_rows(tables))


def _write_averaged_rows(path: pathlib.Path, rows: list[AveragedRow]) -> None:
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(
            ["app_label", "snapshot_label", "n_samples"]
            + [f"{metric}_{stat}" for metric in SUMMARY_METRICS for stat in SUMMARY_STATISTICS]
        )
        for row in rows:
            row_out: list[str] = [row.app_label,
                                  row.snapshot_label, str(row.n_samples)]
            for metric in SUMMARY_METRICS:
                stats = row.metrics[metric]
                row_out.extend([
                    f"{stats.mean:.2f}",
                    f"{stats.geomean:.2f}",
                    f"{stats.median:.2f}",
                    f"{stats.std:.2f}",
                    str(stats.min),
                    str(stats.max),
                ])
            writer.writerow(row_out)


def _compute_averaged_rows(
        tables: list[IterationSummaryTable]) -> list[AveragedRow]:
    rows_by_table = [
        {row.pair: row for row in table.rows}
        for table in tables
    ]
    rows: list[AveragedRow] = []
    for pair in _union_keys(tables):
        values_by_metric: dict[str, list[int]] = {
            metric: [] for metric in SUMMARY_METRICS
        }
        for rows_by_pair in rows_by_table:
            row = rows_by_pair.get(pair)
            if row is None:
                continue
            for metric in SUMMARY_METRICS:
                values_by_metric[metric].append(getattr(row.profile, metric))
        n_samples = len(values_by_metric[SUMMARY_METRICS[0]])
        metrics = {
            metric: _format_statistics(values)
            for metric, values in values_by_metric.items()
        }
        rows.append(AveragedRow(
            pair=pair,
            n_samples=n_samples,
            metrics=metrics,
        ))
    return rows


def _write_outliers_csv(path: pathlib.Path,
                        discarded_names: list[str]) -> None:
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["discarded_iteration"])
        for name in discarded_names:
            writer.writerow([name])


def _select_discarded_iterations(
        tables: list[IterationSummaryTable]) -> set[int]:
    discard_count = _allowed_discard_count(len(tables))
    if discard_count == 0:
        return set()
    scores = _iteration_outlier_scores(tables)
    ordered = sorted(scores, key=lambda item: (-item[1], item[0]))
    return {index for index, _score in ordered[:discard_count]}


def _allowed_discard_count(iteration_count: int) -> int:
    requested_drop = math.floor(iteration_count * OUTLIER_TRIM_FRACTION)
    max_drop_by_min_iterations = iteration_count - OUTLIER_MIN_RETAINED_ITERATIONS
    max_drop_by_retained_fraction = iteration_count - math.ceil(
        iteration_count * OUTLIER_MIN_RETAINED_FRACTION)
    return max(
        0,
        min(
            requested_drop,
            max_drop_by_min_iterations,
            max_drop_by_retained_fraction,
        ),
    )


def _iteration_outlier_scores(
        tables: list[IterationSummaryTable]) -> list[tuple[int, float]]:
    medians = _cell_medians(tables)
    scores: list[tuple[int, float]] = []
    for index, table in enumerate(tables):
        deviations: list[float] = []
        for row in table.rows:
            for metric in SUMMARY_METRICS:
                key = (row.pair, metric)
                median = medians[key]
                deviations.append(0 if median == 0 else
                                  abs(getattr(row.profile, metric) - median) / median)
        score = statistics.median(deviations) if deviations else 0.0
        scores.append((index, float(score)))
    return scores


def _cell_medians(tables: list[IterationSummaryTable]) -> dict[CellKey, float]:
    values_by_cell: dict[CellKey, list[int]] = {}
    for table in tables:
        for row in table.rows:
            for metric in SUMMARY_METRICS:
                key = (row.pair, metric)
                values_by_cell.setdefault(key, []).append(
                    getattr(row.profile, metric))
    return {
        key: float(statistics.median(values))
        for key, values in values_by_cell.items()
    }


def _read_summary_rows(path: pathlib.Path) -> list[SummaryRow]:
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream)
        header = next(reader)
        if header[:2] != ["app_label",
                          "snapshot_label"] or header[2:] != SUMMARY_METRICS:
            raise ValueError(f"unexpected summary.csv header: {header}")
        rows: list[SummaryRow] = []
        for line in reader:
            if not line:
                continue
            expected_width = 2 + len(SUMMARY_METRICS)
            if len(line) != expected_width:
                raise ValueError(
                    f"unexpected summary.csv row width: expected {expected_width}, got {len(line)}")
            values = {
                metric: int(value)
                for metric, value in zip(SUMMARY_METRICS, line[2:])
            }
            rows.append(SummaryRow(
                pair=AppSnapshotLabelPair(line[0], line[1]),
                profile=MemProfile(**values),
            ))
        return rows


def _union_keys(tables: list[IterationSummaryTable]
                ) -> list[AppSnapshotLabelPair]:
    keys: list[AppSnapshotLabelPair] = []
    seen: set[AppSnapshotLabelPair] = set()
    for table in tables:
        for row in table.rows:
            if row.pair not in seen:
                seen.add(row.pair)
                keys.append(row.pair)
    return keys


def _format_statistics(values: list[int]) -> AveragedStatistics:
    n = len(values)
    mean = statistics.mean(values)
    if any(value == 0 for value in values):
        geomean = 0.0
    else:
        geomean = statistics.geometric_mean(values)
    median = statistics.median(values)
    if n >= 2:
        std = statistics.stdev(values)
    else:
        std = 0.0
    return AveragedStatistics(
        mean=mean,
        geomean=geomean,
        median=median,
        std=std,
        min=min(values),
        max=max(values),
    )


def _read_breakdown_rows(path: pathlib.Path) -> dict[str, MemProfile]:
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream)
        header = next(reader)
        if header != ["tag", *SUMMARY_METRICS]:
            raise ValueError(f"unexpected breakdown header: {header}")
        rows: dict[str, MemProfile] = {}
        for line in reader:
            if not line:
                continue
            expected_width = 1 + len(SUMMARY_METRICS)
            if len(line) != expected_width:
                raise ValueError(
                    f"unexpected breakdown row width: expected {expected_width}, got {len(line)}")
            values = {
                metric: int(value)
                for metric, value in zip(SUMMARY_METRICS, line[1:])
            }
            rows[line[0]] = MemProfile(
                size_kb=values["size_kb"],
                rss_kb=values["rss_kb"],
                pss_kb=values["pss_kb"],
                referenced_kb=values["referenced_kb"],
                shared_kb=values["shared_kb"],
                private_kb=values["private_kb"],
                swap_kb=values["swap_kb"],
                swap_pss_kb=values["swap_pss_kb"],
                anonymous_kb=values["anonymous_kb"],
            )
        return rows


def _breakdown_pairs(
    retained_stores: list[ResultStore],
) -> list[AppSnapshotLabelPair]:
    pairs: list[AppSnapshotLabelPair] = []
    seen: set[AppSnapshotLabelPair] = set()
    for store in retained_stores:
        breakdowns_dir = store.local_breakdowns_dir()
        if not breakdowns_dir.is_dir():
            continue
        for snapshot_dir in sorted(breakdowns_dir.iterdir()):
            if not snapshot_dir.is_dir():
                continue
            for path in sorted(snapshot_dir.iterdir()):
                if path.suffix != ".csv":
                    continue
                pair = AppSnapshotLabelPair(
                    app_label=path.stem.removesuffix(f"-{snapshot_dir.name}"),
                    snapshot_label=snapshot_dir.name,
                )
                if pair not in seen:
                    seen.add(pair)
                    pairs.append(pair)
    return pairs


def _write_averaged_breakdowns(
    summary_store: ResultStore,
    retained_stores: list[ResultStore],
) -> None:
    for pair in _breakdown_pairs(retained_stores):
        per_tag_values: dict[str, dict[str, list[int]]] = {}
        for store in retained_stores:
            path = store.local_breakdown_path(
                pair.snapshot_label, pair.app_label)
            if not path.exists():
                continue
            for tag, profile in _read_breakdown_rows(path).items():
                tag_values = per_tag_values.setdefault(
                    tag, {metric: [] for metric in SUMMARY_METRICS})
                for metric in SUMMARY_METRICS:
                    tag_values[metric].append(getattr(profile, metric))
        rows: list[list[str]] = []
        for tag, tag_values in per_tag_values.items():
            n_samples = len(tag_values[SUMMARY_METRICS[0]])
            row: list[str] = [tag, str(n_samples)]
            for metric in SUMMARY_METRICS:
                stats = _format_statistics(tag_values[metric])
                row.extend([
                    f"{stats.mean:.2f}",
                    f"{stats.geomean:.2f}",
                    f"{stats.median:.2f}",
                    f"{stats.std:.2f}",
                    str(stats.min),
                    str(stats.max),
                ])
            rows.append(row)
        if not rows:
            continue
        rows.sort(key=lambda row: (-float(row[2]), row[0]))
        path = summary_store.local_breakdown_path(
            pair.snapshot_label, pair.app_label)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("w", encoding="utf-8", newline="") as stream:
            writer = csv.writer(stream)
            writer.writerow(
                ["tag", "n_samples"]
                + [f"{metric}_{stat}" for metric in SUMMARY_METRICS for stat in SUMMARY_STATISTICS]
            )
            writer.writerows(rows)
