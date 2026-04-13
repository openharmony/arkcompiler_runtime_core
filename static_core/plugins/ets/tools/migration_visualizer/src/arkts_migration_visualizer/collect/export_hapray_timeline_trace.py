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

"""
Export Hapray perf sampling data to a timeline trace JSON file.

This is an experimental bridge for viewing sampled call stacks in a timeline
viewer that accepts trace-event JSON (`B`/`E` events). The exporter reads a
Hapray `perf.db`, reconstructs per-thread call stacks, estimates time slices
from adjacent samples, and serializes them as trace events.
"""

from __future__ import annotations

import json
import os
import sqlite3
import statistics
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple


PREFERRED_EVENT_NAMES = (
    "hw-instructions",
    "instructions",
    "raw-instruction-retired",
    "hw-cpu-cycles",
    "cpu-cycles",
    "raw-cpu-cycles",
)
DEFAULT_TRACE_BASENAME = "hapray_timeline_trace.json"
DEFAULT_TRACE_ARTIFACTS_DIRNAME = "hapray_timeline_trace"
AUTO_DISCOVERY_SKIP_DIRS = {
    ".git",
    ".hg",
    ".svn",
    "__pycache__",
    ".venv",
    "node_modules",
    ".bootstrap",
}


@dataclass(frozen=True)
class ThreadInfo:
    thread_id: int
    process_id: int
    thread_name: str
    process_name: str


@dataclass(frozen=True)
class Frame:
    depth: int
    file_id: int
    file_path: str
    symbol_id: int
    symbol_name: str


@dataclass(frozen=True)
class Sample:
    sample_id: int
    callchain_id: int
    thread_id: int
    event_count: int
    cpu_id: int
    event_name: str
    timestamp_ns: int


@dataclass(frozen=True)
class SampleInterval:
    thread: ThreadInfo
    event_name: str
    start_ns: int
    end_ns: int
    stack: Tuple[Frame, ...]
    sample_count: int
    event_count: int


@dataclass(frozen=True)
class TimelineExportRequest:
    input_path: Path
    output_path: str = ""
    step: Optional[int] = None
    event_name: str = ""
    thread_ids: Tuple[int, ...] = ()
    merge_samples: bool = True


@dataclass(frozen=True)
class TimelineExportResult:
    perf_db_path: Path
    output_path: Path
    trace_obj: Dict[str, object]


def resolve_perf_db_path(input_path: Path, step: Optional[int] = None) -> Path:
    return resolve_perf_db_path_with_repo(input_path, step=step, repo_root=default_repo_root())


def default_repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def resolve_path(base_dir: Path, raw: str) -> Path:
    path = Path(raw).expanduser()
    if path.is_absolute():
        return path.resolve()
    return (base_dir / path).resolve()


def load_configured_deps_root(repo_root: Path) -> Optional[Path]:
    config_path = repo_root / "configs" / "config.json"
    if not config_path.is_file():
        return None
    try:
        config = json.loads(config_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    raw_deps_root = str(config.get("deps_root", "")).strip()
    if not raw_deps_root:
        return None
    return resolve_path(repo_root, raw_deps_root)


def load_repo_config(repo_root: Path) -> Dict[str, object]:
    config_path = repo_root / "configs" / "config.json"
    if not config_path.is_file():
        return {}
    try:
        return json.loads(config_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}


def dedupe_paths(paths: Iterable[Path]) -> List[Path]:
    deduped: List[Path] = []
    seen = set()
    for candidate in paths:
        resolved = candidate.resolve()
        key = str(resolved)
        if key in seen:
            continue
        seen.add(key)
        deduped.append(resolved)
    return deduped


def candidate_hapray_report_roots(repo_root: Path) -> List[Path]:
    candidates: List[Path] = []
    config = load_repo_config(repo_root)

    deps_root = load_configured_deps_root(repo_root)
    if deps_root:
        candidates.extend(
            [
                deps_root / "sources" / "ArkAnalyzer-HapRay" / "perf_testing" / "reports",
                deps_root / "sources" / "ArkAnalyzer-HapRay" / "reports",
            ]
        )

    for raw_name in ("hapray_path", "hapray_root"):
        raw_path = str(config.get(raw_name, "")).strip()
        if not raw_path:
            continue
        hapray_root = resolve_path(repo_root, raw_path)
        candidates.extend(
            [
                hapray_root / "reports",
                hapray_root / "perf_testing" / "reports",
            ]
        )

    return dedupe_paths(candidates)


def candidate_hapray_search_roots(repo_root: Path) -> List[Path]:
    candidates: List[Path] = []
    config = load_repo_config(repo_root)

    deps_root = load_configured_deps_root(repo_root)
    if deps_root:
        candidates.extend(
            [
                deps_root / "sources" / "ArkAnalyzer-HapRay" / "perf_testing",
                deps_root / "sources" / "ArkAnalyzer-HapRay",
                deps_root / "artifacts",
            ]
        )

    for raw_name in ("hapray_path", "hapray_root"):
        raw_path = str(config.get(raw_name, "")).strip()
        if not raw_path:
            continue
        hapray_root = resolve_path(repo_root, raw_path)
        candidates.extend(
            [
                hapray_root / "perf_testing",
                hapray_root,
            ]
        )

    candidates.extend(
        [
            repo_root / "artifacts",
            repo_root / "reports",
            Path.cwd() / "reports",
        ]
    )
    return dedupe_paths(candidates)


def perf_db_candidate_matches(path: Path, step: Optional[int]) -> bool:
    if path.name != "perf.db":
        return False
    if step is None:
        return True
    return f"step{step}" in path.parts


def recursive_perf_db_candidates(search_root: Path, step: Optional[int]) -> List[Path]:
    matches: List[Path] = []
    if not search_root.is_dir():
        return matches

    for current_root, dirnames, filenames in os.walk(search_root):
        dirnames[:] = [name for name in dirnames if name not in AUTO_DISCOVERY_SKIP_DIRS]
        if "perf.db" not in filenames:
            continue
        candidate = Path(current_root) / "perf.db"
        if perf_db_candidate_matches(candidate, step=step):
            matches.append(candidate.resolve())
    return matches


def auto_discovery_status_lines(repo_root: Path) -> List[str]:
    lines = ["Auto-discovery summary:"]
    report_roots = candidate_hapray_report_roots(repo_root)
    if report_roots:
        lines.append("  Report roots:")
        for path in report_roots:
            state = "exists" if path.is_dir() else "missing"
            lines.append(f"    - {path} [{state}]")
    else:
        lines.append("  Report roots: none configured")

    search_roots = candidate_hapray_search_roots(repo_root)
    if search_roots:
        lines.append("  Search roots:")
        for path in search_roots:
            state = "exists" if path.is_dir() else "missing"
            lines.append(f"    - {path} [{state}]")
    else:
        lines.append("  Search roots: none")
    return lines


def find_latest_perf_db_from_repo(repo_root: Path, step: Optional[int] = None) -> Optional[Path]:
    matches: List[Path] = []
    for reports_root in candidate_hapray_report_roots(repo_root):
        if not reports_root.is_dir():
            continue
        if step is not None:
            matches.extend(reports_root.glob(f"*/*/hiperf/step{step}/perf.db"))
        else:
            matches.extend(reports_root.glob("*/*/hiperf/step*/perf.db"))
    if not matches:
        for search_root in candidate_hapray_search_roots(repo_root):
            matches.extend(recursive_perf_db_candidates(search_root, step=step))
    existing = [path.resolve() for path in matches if path.is_file()]
    if not existing:
        return None
    return max(existing, key=lambda path: (path.stat().st_mtime_ns, str(path)))


def resolve_perf_db_path_with_repo(input_path: Path, step: Optional[int], repo_root: Path) -> Path:
    candidate = input_path.expanduser().resolve()
    if candidate.is_file():
        if candidate.name != "perf.db":
            raise FileNotFoundError(f"expected a perf.db file, got: {candidate}")
        return candidate
    if not candidate.is_dir():
        if input_path.name == "perf.db" and not input_path.is_absolute():
            auto_found = find_latest_perf_db_from_repo(repo_root, step=step)
            if auto_found:
                print(f"Auto-selected latest Hapray perf.db: {auto_found}")
                return auto_found
            details = "\n".join(auto_discovery_status_lines(repo_root))
            missing_hint = (
                "No original Hapray perf.db files were found. "
                "The exporter needs the original Hapray reports tree; "
                "artifacts/test_run_* alone only keep hapray_report.json and component_timings.json."
            )
        raise FileNotFoundError(
            f"input path does not exist: {candidate}\n"
            + (f"{details}\n{missing_hint}\n" if input_path.name == "perf.db" and not input_path.is_absolute() else "")
            + "Hint: pass a concrete perf.db path, a Hapray case directory, "
            "or run the command with a repo config that points deps_root to an existing Hapray reports tree."
        )

    direct_candidates = [candidate / "perf.db"]
    if step is not None:
        direct_candidates.extend(
            [
                candidate / f"step{step}" / "perf.db",
                candidate / "hiperf" / f"step{step}" / "perf.db",
            ]
        )
    else:
        direct_candidates.extend(
            sorted(candidate.glob("step*/perf.db")) + sorted(candidate.glob("hiperf/step*/perf.db"))
        )

    existing = [path.resolve() for path in direct_candidates if path.is_file()]
    if len(existing) == 1:
        return existing[0]
    if len(existing) > 1:
        raise FileNotFoundError(
            "multiple perf.db candidates were found; pass --step or point to a concrete perf.db file:\n"
            + "\n".join(str(path) for path in existing)
        )

    recursive = sorted(candidate.rglob("perf.db"))
    if len(recursive) == 1:
        return recursive[0].resolve()
    if len(recursive) > 1:
        raise FileNotFoundError(
            "multiple perf.db files were found recursively; pass --step or point to a concrete perf.db file:\n"
            + "\n".join(str(path.resolve()) for path in recursive[:20])
        )
    raise FileNotFoundError(f"could not locate perf.db under: {candidate}")


def output_path_for_perf_db(perf_db_path: Path, output_path: str) -> Path:
    if output_path:
        return Path(output_path).expanduser().resolve()
    repo_root = default_repo_root()
    artifacts_root = repo_root / "artifacts" / DEFAULT_TRACE_ARTIFACTS_DIRNAME
    relative = perf_db_artifact_relative_path(perf_db_path)
    return (artifacts_root / relative).resolve()


def perf_db_artifact_relative_path(perf_db_path: Path) -> Path:
    parts = perf_db_path.parts
    if "reports" in parts:
        reports_index = max(index for index, part in enumerate(parts) if part == "reports")
        tail = [part for part in parts[reports_index + 1 : -1] if part and part not in (".", "..")]
        if tail:
            return Path(*tail) / DEFAULT_TRACE_BASENAME

    fallback_parts = [part for part in perf_db_path.parts[-4:-1] if part and part not in (".", "..")]
    if fallback_parts:
        return Path("manual") / Path(*fallback_parts) / DEFAULT_TRACE_BASENAME
    return Path("manual") / DEFAULT_TRACE_BASENAME


def choose_event_name(event_names: Sequence[str], requested: str) -> str:
    names = list(dict.fromkeys(name for name in event_names if name))
    if not names:
        raise ValueError("no supported perf sample events were found in perf.db")
    if requested:
        if requested not in names:
            raise ValueError(f"requested event name was not found: {requested}\navailable: {', '.join(names)}")
        return requested
    for preferred in PREFERRED_EVENT_NAMES:
        if preferred in names:
            return preferred
    return names[0]


def available_event_names(conn: sqlite3.Connection) -> List[str]:
    rows = conn.execute(
        """
        SELECT DISTINCT perf_report.report_value
        FROM perf_sample
        INNER JOIN perf_report ON perf_report.id = perf_sample.event_type_id
        WHERE perf_report.report_value IS NOT NULL
        ORDER BY perf_report.report_value
        """
    ).fetchall()
    return [str(row[0]) for row in rows]


def load_threads(conn: sqlite3.Connection) -> Dict[int, ThreadInfo]:
    process_rows = conn.execute(
        "SELECT process_id, COALESCE(NULLIF(thread_name, ''), 'process') FROM perf_thread WHERE thread_id = process_id"
    ).fetchall()
    process_names = {int(process_id): str(name) for process_id, name in process_rows}

    thread_rows = conn.execute(
        "SELECT thread_id, process_id, COALESCE(NULLIF(thread_name, ''), 'thread') FROM perf_thread ORDER BY thread_id"
    ).fetchall()
    threads: Dict[int, ThreadInfo] = {}
    for thread_id, process_id, thread_name in thread_rows:
        pid = int(process_id)
        tid = int(thread_id)
        threads[tid] = ThreadInfo(
            thread_id=tid,
            process_id=pid,
            thread_name=str(thread_name) or f"thread-{tid}",
            process_name=process_names.get(pid, f"process-{pid}"),
        )
    return threads


def load_samples(conn: sqlite3.Connection, event_name: str, thread_ids: Sequence[int]) -> List[Sample]:
    sql = """
        SELECT
            perf_sample.id,
            perf_sample.callchain_id,
            perf_sample.thread_id,
            perf_sample.event_count,
            perf_sample.cpu_id,
            perf_report.report_value AS event_name,
            perf_sample.timestamp_trace AS timestamp_trace
        FROM perf_sample
        INNER JOIN perf_report ON perf_report.id = perf_sample.event_type_id
        WHERE perf_report.report_value = ?
    """
    params: List[object] = [event_name]
    if thread_ids:
        placeholders = ", ".join("?" for _ in thread_ids)
        sql += f" AND perf_sample.thread_id IN ({placeholders})"
        params.extend(int(thread_id) for thread_id in thread_ids)
    sql += " ORDER BY perf_sample.thread_id, perf_sample.timestamp_trace, perf_sample.id"

    rows = conn.execute(sql, params).fetchall()
    samples: List[Sample] = []
    for row in rows:
        timestamp = row[6]
        if timestamp is None:
            continue
        samples.append(
            Sample(
                sample_id=int(row[0]),
                callchain_id=int(row[1]),
                thread_id=int(row[2]),
                event_count=int(row[3] or 0),
                cpu_id=int(row[4] or 0),
                event_name=str(row[5]),
                timestamp_ns=int(timestamp),
            )
        )
    return samples


def load_callchains(conn: sqlite3.Connection, callchain_ids: Iterable[int]) -> Dict[int, Tuple[Frame, ...]]:
    ids = sorted(set(int(callchain_id) for callchain_id in callchain_ids))
    if not ids:
        return {}

    placeholders = ", ".join("?" for _ in ids)
    rows = conn.execute(
        f"""
        SELECT
            perf_callchain.callchain_id,
            perf_callchain.depth,
            perf_callchain.file_id,
            COALESCE(perf_files_by_id.path, '<unknown file>') AS file_path,
            perf_callchain.name AS symbol_id,
            COALESCE(data_dict_by_id.data, '<unknown symbol>') AS symbol_name
        FROM perf_callchain
        LEFT JOIN (
            SELECT file_id, MIN(path) AS path
            FROM perf_files
            GROUP BY file_id
        ) AS perf_files_by_id ON perf_files_by_id.file_id = perf_callchain.file_id
        LEFT JOIN (
            SELECT id, MIN(data) AS data
            FROM data_dict
            GROUP BY id
        ) AS data_dict_by_id ON data_dict_by_id.id = perf_callchain.name
        WHERE perf_callchain.callchain_id IN ({placeholders})
        ORDER BY perf_callchain.callchain_id, perf_callchain.depth DESC
        """,
        ids,
    ).fetchall()

    callchains: Dict[int, List[Frame]] = {}
    for row in rows:
        callchain_id = int(row[0])
        callchains.setdefault(callchain_id, []).append(
            Frame(
                depth=int(row[1]),
                file_id=int(row[2]),
                file_path=str(row[3]),
                symbol_id=int(row[4]),
                symbol_name=str(row[5]),
            )
        )
    return {callchain_id: tuple(frames) for callchain_id, frames in callchains.items()}


def default_half_span_ns(samples: Sequence[Sample]) -> int:
    timestamps = [sample.timestamp_ns for sample in samples]
    deltas = [
        timestamps[index + 1] - timestamps[index]
        for index in range(len(timestamps) - 1)
        if timestamps[index + 1] > timestamps[index]
    ]
    if deltas:
        return max(1, int(statistics.median(deltas) // 2))
    return 500_000


def estimate_intervals(
    samples: Sequence[Sample],
    threads: Dict[int, ThreadInfo],
    callchains: Dict[int, Tuple[Frame, ...]],
) -> List[SampleInterval]:
    grouped: Dict[int, List[Sample]] = {}
    for sample in samples:
        grouped.setdefault(sample.thread_id, []).append(sample)

    intervals: List[SampleInterval] = []
    for thread_id, group in grouped.items():
        thread = threads.get(
            thread_id,
            ThreadInfo(thread_id=thread_id, process_id=thread_id, thread_name=f"thread-{thread_id}", process_name=f"process-{thread_id}"),
        )
        half_span = default_half_span_ns(group)
        for index, sample in enumerate(group):
            prev_sample = group[index - 1] if index > 0 else None
            next_sample = group[index + 1] if index + 1 < len(group) else None
            start_ns = sample.timestamp_ns - (
                (sample.timestamp_ns - prev_sample.timestamp_ns) // 2 if prev_sample else half_span
            )
            end_ns = sample.timestamp_ns + (
                (next_sample.timestamp_ns - sample.timestamp_ns) // 2 if next_sample else half_span
            )
            if end_ns <= start_ns:
                end_ns = start_ns + 1
            stack = callchains.get(sample.callchain_id, ())
            if not stack:
                continue
            intervals.append(
                SampleInterval(
                    thread=thread,
                    event_name=sample.event_name,
                    start_ns=start_ns,
                    end_ns=end_ns,
                    stack=stack,
                    sample_count=1,
                    event_count=sample.event_count,
                )
            )
    return intervals


def merge_intervals(intervals: Sequence[SampleInterval]) -> List[SampleInterval]:
    if not intervals:
        return []
    ordered = sorted(intervals, key=lambda item: (item.thread.thread_id, item.start_ns, item.end_ns))
    merged: List[SampleInterval] = [ordered[0]]
    for current in ordered[1:]:
        previous = merged[-1]
        if (
            previous.thread.thread_id == current.thread.thread_id
            and previous.event_name == current.event_name
            and previous.stack == current.stack
            and current.start_ns <= previous.end_ns
        ):
            merged[-1] = SampleInterval(
                thread=previous.thread,
                event_name=previous.event_name,
                start_ns=previous.start_ns,
                end_ns=max(previous.end_ns, current.end_ns),
                stack=previous.stack,
                sample_count=previous.sample_count + current.sample_count,
                event_count=previous.event_count + current.event_count,
            )
            continue
        merged.append(current)
    return merged


def ns_to_trace_us(timestamp_ns: int) -> int:
    return max(0, int(round(timestamp_ns / 1000)))


def frame_display_name(frame: Frame) -> str:
    symbol = frame.symbol_name.strip()
    if symbol and symbol != "<unknown symbol>":
        return symbol
    file_name = Path(frame.file_path).name.strip()
    return file_name or "<unknown frame>"


def metadata_events(threads: Dict[int, ThreadInfo]) -> List[Dict[str, object]]:
    events: List[Dict[str, object]] = []
    seen_processes = set()
    for thread in sorted(threads.values(), key=lambda item: (item.process_id, item.thread_id)):
        if thread.process_id not in seen_processes:
            events.append(
                {
                    "name": "process_name",
                    "ph": "M",
                    "pid": thread.process_id,
                    "tid": 0,
                    "args": {"name": thread.process_name},
                }
            )
            seen_processes.add(thread.process_id)
        events.append(
            {
                "name": "thread_name",
                "ph": "M",
                "pid": thread.process_id,
                "tid": thread.thread_id,
                "args": {"name": thread.thread_name},
            }
        )
    return events


def interval_trace_events(intervals: Sequence[SampleInterval]) -> List[Dict[str, object]]:
    sortable: List[Tuple[Tuple[int, int, int, int, int], Dict[str, object]]] = []
    seq = 0
    for interval_index, interval in enumerate(sorted(intervals, key=lambda item: (item.start_ns, item.thread.thread_id))):
        start_us = ns_to_trace_us(interval.start_ns)
        end_us = ns_to_trace_us(interval.end_ns)
        if end_us <= start_us:
            end_us = start_us + 1

        root_to_leaf = list(reversed(interval.stack))
        for frame in root_to_leaf:
            event = {
                "name": frame_display_name(frame),
                "cat": interval.event_name,
                "ph": "B",
                "ts": start_us,
                "pid": interval.thread.process_id,
                "tid": interval.thread.thread_id,
                "args": {
                    "file": frame.file_path,
                    "fileId": frame.file_id,
                    "symbolId": frame.symbol_id,
                    "sampleCount": interval.sample_count,
                    "eventCount": interval.event_count,
                },
            }
            sortable.append(((start_us, interval.thread.process_id, interval.thread.thread_id, 1, seq), event))
            seq += 1

        for frame in interval.stack:
            event = {
                "name": frame_display_name(frame),
                "cat": interval.event_name,
                "ph": "E",
                "ts": end_us,
                "pid": interval.thread.process_id,
                "tid": interval.thread.thread_id,
            }
            sortable.append(((end_us, interval.thread.process_id, interval.thread.thread_id, 0, seq), event))
            seq += 1

    return [event for _, event in sorted(sortable, key=lambda item: item[0])]


def build_timeline_trace(
    perf_db_path: Path,
    event_name: str,
    intervals: Sequence[SampleInterval],
    threads: Dict[int, ThreadInfo],
) -> Dict[str, object]:
    return {
        "traceEvents": metadata_events(threads) + interval_trace_events(intervals),
        "displayTimeUnit": "ms",
        "metadata": {
            "source": "hapray-perf-db",
            "perfDb": str(perf_db_path),
            "eventName": event_name,
            "intervalCount": len(intervals),
        },
    }


def export_perf_db_to_timeline_trace(
    perf_db_path: Path,
    output_path: Path,
    event_name: str = "",
    thread_ids: Sequence[int] = (),
    merge_samples: bool = True,
) -> Dict[str, object]:
    with sqlite3.connect(str(perf_db_path)) as conn:
        available = available_event_names(conn)
        chosen_event = choose_event_name(available, event_name)
        threads = load_threads(conn)
        samples = load_samples(conn, chosen_event, thread_ids)
        callchains = load_callchains(conn, (sample.callchain_id for sample in samples))

    if not samples:
        raise ValueError(f"no samples matched the selected event/thread filters in: {perf_db_path}")

    intervals = estimate_intervals(samples, threads, callchains)
    if merge_samples:
        intervals = merge_intervals(intervals)
    trace_obj = build_timeline_trace(perf_db_path, chosen_event, intervals, threads)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(trace_obj, ensure_ascii=False, indent=2), encoding="utf-8")
    return trace_obj


def export_timeline_trace_artifact(request: TimelineExportRequest) -> TimelineExportResult:
    perf_db_path = resolve_perf_db_path(Path(request.input_path), step=request.step)
    output_path = output_path_for_perf_db(perf_db_path, request.output_path)
    trace_obj = export_perf_db_to_timeline_trace(
        perf_db_path,
        output_path,
        event_name=request.event_name,
        thread_ids=request.thread_ids,
        merge_samples=request.merge_samples,
    )
    return TimelineExportResult(
        perf_db_path=perf_db_path,
        output_path=output_path,
        trace_obj=trace_obj,
    )


def main() -> None:
    raise SystemExit(
        "export_hapray_timeline_trace.py is an internal helper. "
        "Use arkts-migration-visualizer export-timeline instead."
    )


if __name__ == "__main__":
    main()
