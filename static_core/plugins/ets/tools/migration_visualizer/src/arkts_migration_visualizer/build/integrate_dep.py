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
Internal integration stage used by arkts_migration_visualizer.cli.
"""

from __future__ import annotations

import json
import os
import sys
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Any, DefaultDict, Dict, Iterable, List, Optional, Sequence, Set, Tuple


DEBUG_DIR: Optional[str] = None
DEBUG_ENABLED = True
SOURCE_PRIMARY_MARKERS = ("src", "source")
SOURCE_SECONDARY_MARKERS = ("resources",)
SOURCE_TERTIARY_MARKERS = ("test", "tests")


@dataclass
class IntegrationRequest:
    input_dir: Path
    output_path: Optional[Path] = None
    debug_enabled: bool = True
    debug_dir: Optional[Path] = None


def _debug_path(file_name: str) -> Optional[str]:
    if not DEBUG_ENABLED:
        return None
    if DEBUG_DIR:
        os.makedirs(DEBUG_DIR, exist_ok=True)
        return os.path.join(DEBUG_DIR, file_name)
    return file_name


def save_map_to_file(data: Any, file_name: str) -> None:
    path = _debug_path(file_name)
    if not path:
        return
    try:
        with open(path, "w", encoding="utf-8") as handle:
            json.dump(data, handle, ensure_ascii=False, indent=2)
        print(f"[debug] Saved: {path}")
    except IOError as err:
        print(f"Error: failed to write debug file '{path}'. Details: {err}", file=sys.stderr)


def split_path_parts(raw_path: str) -> List[str]:
    normalized = str(raw_path or "").strip().replace("\\", "/")
    return [part for part in normalized.split("/") if part and part != "."]


def find_source_anchor_index(lower_parts: Sequence[str]) -> Optional[int]:
    for marker_group in (
        SOURCE_PRIMARY_MARKERS,
        SOURCE_SECONDARY_MARKERS,
        SOURCE_TERTIARY_MARKERS,
    ):
        hits = [idx for idx, part in enumerate(lower_parts) if part in marker_group]
        if hits:
            return hits[-1]
    return None


def last_path_part(raw_path: str) -> str:
    parts = split_path_parts(raw_path)
    return parts[-1] if parts else str(raw_path or "")


def is_non_source_artifact_file(raw_path: str) -> bool:
    parts = [part.lower() for part in split_path_parts(raw_path)]
    if not parts:
        return False

    if ".preview" in parts:
        return True

    if any("previewarkts" in part for part in parts):
        return True

    if "build" not in parts:
        return False

    build_idx = parts.index("build")
    tail = parts[build_idx + 1 :]
    return "cache" in tail or "generated" in tail


def normalize_har_name(raw_name: str) -> str:
    name = str(raw_name or "").strip().replace("\\", "/")
    if not name:
        return ""

    if name.startswith("@"):
        if "/" in name:
            return name
        parts = [part for part in name.split("/") if part]
        if len(parts) >= 2:
            return parts[1]
        if name.startswith("@ohos."):
            return name[len("@ohos.") :]
        return name

    candidate = last_path_part(name)
    lower_candidate = candidate.lower()
    for suffix in (".z.so", ".har", ".hsp", ".hap", ".ets", ".ts", ".js", ".so"):
        if lower_candidate.endswith(suffix):
            return candidate[: -len(suffix)]
    return candidate


def normalize_path_key(raw_path: str) -> str:
    parts = split_path_parts(raw_path)
    return "/".join(part.lower() for part in parts)


def safe_int(value: Any) -> int:
    try:
        return int(value or 0)
    except Exception:
        return 0


def get_hapray_steps(report: Dict[str, Any]) -> List[Dict[str, Any]]:
    perf = report.get("perf", {})
    steps = perf.get("steps", [])
    return steps if isinstance(steps, list) else []


def get_hapray_data_list(report: Dict[str, Any]) -> List[Dict[str, Any]]:
    merged: List[Dict[str, Any]] = []
    for step in get_hapray_steps(report):
        data = step.get("data", [])
        if isinstance(data, list):
            for item in data:
                if isinstance(item, dict):
                    merged.append(item)
    if not merged:
        print("Warning: perf.steps[*].data could not be found in hapray_report.json.")
    return merged


def get_hapray_har_items(report: Dict[str, Any]) -> List[Dict[str, Any]]:
    perf = report.get("perf", {})
    har_items = perf.get("har", [])
    return har_items if isinstance(har_items, list) else []


def iter_nodes(graph: Dict[str, Any]) -> Iterable[Dict[str, Any]]:
    nodes = graph.get("nodes", [])
    return nodes if isinstance(nodes, list) else []


def iter_edges(graph: Dict[str, Any]) -> Iterable[Dict[str, Any]]:
    edges = graph.get("edges", [])
    return edges if isinstance(edges, list) else []


def load_category_map(graph: Dict[str, Any]) -> Dict[int, str]:
    result: Dict[int, str] = {}
    for item in graph.get("categories", []):
        if not isinstance(item, dict):
            continue
        category_id = item.get("id")
        category_name = item.get("name")
        if isinstance(category_id, int) and isinstance(category_name, str):
            result[category_id] = category_name
    return result


def is_file_node(node: Dict[str, Any], category_map: Dict[int, str]) -> bool:
    kind = node.get("kind")
    if isinstance(kind, int):
        return category_map.get(kind) == "FILE"
    if isinstance(kind, str):
        return kind.upper() == "FILE"
    return False


def compute_common_dir_prefix(paths: Sequence[str]) -> List[str]:
    dirs = [split_path_parts(path)[:-1] for path in paths if split_path_parts(path)]
    if not dirs:
        return []
    prefix = list(dirs[0])
    for current in dirs[1:]:
        limit = min(len(prefix), len(current))
        idx = 0
        while idx < limit and prefix[idx].lower() == current[idx].lower():
            idx += 1
        prefix = prefix[:idx]
        if not prefix:
            break
    return prefix


def derive_root_har_name(common_prefix: Sequence[str]) -> str:
    if common_prefix:
        return normalize_har_name(common_prefix[-1])
    return "workspace"


def infer_internal_har_name(
    file_path: str,
    common_prefix: Sequence[str],
    root_har_name: str,
    preferred_module_names: Sequence[str] = (),
) -> str:
    parts = split_path_parts(file_path)
    if not parts:
        return root_har_name

    lower_parts = [part.lower() for part in parts]
    file_name = lower_parts[-1]
    preferred_lookup = {
        str(name).lower(): str(name) for name in preferred_module_names if str(name).strip()
    }

    for part in reversed(parts[:-1]):
        canonical = normalize_har_name(part)
        if canonical and canonical.lower() in preferred_lookup:
            return preferred_lookup[canonical.lower()]

    config_like_files = {
        "hvigorfile.js",
        "hvigor-wrapper.js",
        "module.json5",
        "build-profile.json5",
        "oh-package.json5",
        "package.json",
    }
    if file_name in config_like_files:
        if len(parts) >= 2 and common_prefix and len(parts) - 1 > len(common_prefix):
            parent = parts[-2]
            if parent.lower() == "hvigor":
                return root_har_name
            if parent.lower() != common_prefix[-1].lower():
                return normalize_har_name(parent)
        return root_har_name

    for marker in ("src", "source", "test", "tests", "resources"):
        if marker in lower_parts:
            idx = lower_parts.index(marker)
            if idx > 0:
                candidate = parts[idx - 1]
                if not common_prefix or idx - 1 >= len(common_prefix):
                    return normalize_har_name(candidate)

    if common_prefix:
        prefix_len = len(common_prefix)
        if len(parts) > prefix_len + 1:
            return normalize_har_name(parts[prefix_len])

    if len(parts) >= 2:
        return normalize_har_name(parts[-2])
    return root_har_name


def build_leaf_module_names(module_raw_paths: Sequence[str]) -> Tuple[List[str], Set[str]]:
    module_parts = [(raw_path, split_path_parts(raw_path)) for raw_path in module_raw_paths if raw_path]
    leaf_names: List[str] = []
    container_names: Set[str] = set()
    seen_leaf_names: Set[str] = set()

    for raw_path, parts in module_parts:
        if not parts:
            continue
        is_container = False
        for other_raw_path, other_parts in module_parts:
            if raw_path == other_raw_path:
                continue
            if len(parts) >= len(other_parts):
                continue
            if all(left.lower() == right.lower() for left, right in zip(parts, other_parts)):
                is_container = True
                break

        canonical = normalize_har_name(raw_path)
        if not canonical:
            continue
        if is_container:
            container_names.add(canonical)
            continue
        if canonical not in seen_leaf_names:
            leaf_names.append(canonical)
            seen_leaf_names.add(canonical)

    return leaf_names, container_names


def build_path_leakage_filters(
    common_prefix: Sequence[str],
    file_paths: Sequence[str],
    module_raw_paths: Sequence[str],
    module_names: Sequence[str],
    module_container_names: Sequence[str],
) -> Tuple[Set[str], str]:
    """
    Infer "path leakage" nodes from the shared prefix of internal source files.

    The goal is to filter out host-path ancestors that were mistakenly treated
    as HAR nodes, for example:
    - Windows drive prefixes / WSL mount segments
    - Users / usernames / DevEcoStudioProjects
    - the project root folder if it is just a container, not a real module
    """

    module_name_set = {str(name).lower() for name in module_names if name}
    prefix_names = [normalize_har_name(part) for part in common_prefix]
    prefix_names = [name for name in prefix_names if name]

    blocked_names: Set[str] = {
        name for name in prefix_names[:-1] if name.lower() not in module_name_set
    }

    for raw_path in module_raw_paths:
        parts = split_path_parts(raw_path)
        if len(parts) < 2:
            continue
        for part in parts[:-1]:
            canonical = normalize_har_name(part)
            if canonical and canonical.lower() not in module_name_set:
                blocked_names.add(canonical)

    prefix_len = len(common_prefix)
    for raw_path in file_paths:
        parts = split_path_parts(raw_path)
        if len(parts) <= prefix_len:
            continue
        canonical = normalize_har_name(parts[prefix_len])
        if canonical and canonical.lower() not in module_name_set:
            blocked_names.add(canonical)

    for container_name in module_container_names:
        if container_name and container_name.lower() not in module_name_set:
            blocked_names.add(container_name)

    project_container_name = ""
    if module_container_names:
        project_container_name = sorted(
            (name for name in module_container_names if name),
            key=lambda name: (-len(name), name),
        )[0]
    elif prefix_names:
        root_candidate = prefix_names[-1]
        if root_candidate and root_candidate.lower() not in module_name_set:
            project_container_name = root_candidate
    if project_container_name:
        blocked_names.add(project_container_name)

    return blocked_names, project_container_name


def build_har_timing_map(component_timings: List[Dict[str, Any]], hapray_report: Dict[str, Any]) -> Dict[str, int]:
    """Official HAR timing source: component_timings.json first, then perf.har."""
    result: Dict[str, int] = defaultdict(int)
    source_items = component_timings or get_hapray_har_items(hapray_report)
    for item in source_items:
        if not isinstance(item, dict) or "name" not in item:
            continue
        canonical = normalize_har_name(str(item["name"]))
        if canonical:
            result[canonical] += safe_int(item.get("count", 0))
    return dict(result)


def build_source_file_index(file_graph: Dict[str, Any]) -> Dict[str, Any]:
    category_map = load_category_map(file_graph)
    by_path_key: Dict[str, str] = {}
    by_relative_suffix: DefaultDict[str, Set[str]] = defaultdict(set)
    by_module_relative_suffix: DefaultDict[str, Set[str]] = defaultdict(set)

    for node in iter_nodes(file_graph):
        if not isinstance(node, dict) or "name" not in node or not is_file_node(node, category_map):
            continue
        raw_file = str(node["name"])
        if is_non_source_artifact_file(raw_file):
            continue
        parts = split_path_parts(raw_file)
        if not parts:
            continue

        lower_parts = [part.lower() for part in parts]
        source_idx = find_source_anchor_index(lower_parts)
        if source_idx is None:
            continue

        build_idx = next((idx for idx, part in enumerate(lower_parts) if part == "build"), None)
        if build_idx is not None and build_idx < source_idx:
            continue

        path_key = normalize_path_key(raw_file)
        if not path_key:
            continue
        by_path_key[path_key] = raw_file

        relative_key = "/".join(lower_parts[source_idx:])
        if relative_key:
            by_relative_suffix[relative_key].add(raw_file)

        if source_idx > 0:
            module_relative_key = "/".join([lower_parts[source_idx - 1]] + lower_parts[source_idx:])
            if module_relative_key:
                by_module_relative_suffix[module_relative_key].add(raw_file)

    return {
        "by_path_key": by_path_key,
        "by_relative_suffix": {key: set(value) for key, value in by_relative_suffix.items()},
        "by_module_relative_suffix": {
            key: set(value) for key, value in by_module_relative_suffix.items()
        },
    }


def looks_like_version_segment(raw_part: str) -> bool:
    text = str(raw_part or "").strip()
    if not text:
        return False
    dot_parts = [part for part in text.replace("-", ".").split(".") if part]
    return len(dot_parts) >= 2 and all(part.isdigit() for part in dot_parts)


def resolve_source_variants(relative_parts: Sequence[str]) -> List[List[str]]:
    parts = list(relative_parts)
    if not parts:
        return []
    variants: List[List[str]] = [parts]
    last_name = parts[-1]
    if last_name.lower().endswith(".ts"):
        ets_parts = list(parts)
        ets_parts[-1] = last_name[:-3] + ".ets"
        variants.insert(0, ets_parts)
    return variants


def map_build_cache_file_to_source(raw_file: str, source_file_index: Dict[str, Any]) -> str:
    parts = split_path_parts(raw_file)
    if not parts:
        return raw_file

    lower_parts = [part.lower() for part in parts]
    file_name = lower_parts[-1]
    if not file_name.endswith(".ts"):
        return raw_file

    build_idx = next((idx for idx, part in enumerate(lower_parts) if part == "build"), None)
    if build_idx is None:
        return raw_file
    if "cache" not in lower_parts[build_idx + 1 :]:
        return raw_file

    source_idx = next(
        (idx for idx in range(build_idx + 1, len(lower_parts)) if lower_parts[idx] in ("src", "source", "test", "tests", "resources")),
        None,
    )
    if source_idx is None:
        return raw_file

    prefix_parts = parts[:build_idx]
    if not prefix_parts:
        return raw_file

    relative_parts = list(parts[source_idx:])
    if not relative_parts:
        return raw_file

    by_path_key = source_file_index.get("by_path_key", {})
    by_relative_suffix = source_file_index.get("by_relative_suffix", {})
    by_module_relative_suffix = source_file_index.get("by_module_relative_suffix", {})
    module_name = prefix_parts[-1].lower()

    for candidate_relative_parts in resolve_source_variants(relative_parts):
        candidate_full_path = "/".join(prefix_parts + candidate_relative_parts)
        candidate_key = normalize_path_key(candidate_full_path)
        if candidate_key in by_path_key:
            return by_path_key[candidate_key]

        module_relative_key = "/".join([module_name] + [part.lower() for part in candidate_relative_parts])
        module_hits = by_module_relative_suffix.get(module_relative_key, set())
        if len(module_hits) == 1:
            return next(iter(module_hits))

        relative_key = "/".join(part.lower() for part in candidate_relative_parts)
        relative_hits = by_relative_suffix.get(relative_key, set())
        if len(relative_hits) == 1:
            return next(iter(relative_hits))

    return raw_file


def map_component_relative_file_to_source(raw_file: str, source_file_index: Dict[str, Any]) -> str:
    parts = split_path_parts(raw_file)
    if not parts:
        return raw_file
    if raw_file.startswith("/") or (parts and parts[0].endswith(":")):
        return raw_file

    lower_parts = [part.lower() for part in parts]
    if not lower_parts[-1].endswith(".ts"):
        return raw_file

    source_idx = find_source_anchor_index(lower_parts)
    if source_idx is None or source_idx == 0:
        return raw_file

    module_idx = source_idx - 1
    if looks_like_version_segment(lower_parts[module_idx]) and module_idx > 0:
        module_idx -= 1
    module_name = lower_parts[module_idx]
    if not module_name:
        return raw_file

    by_relative_suffix = source_file_index.get("by_relative_suffix", {})
    by_module_relative_suffix = source_file_index.get("by_module_relative_suffix", {})
    relative_parts = list(parts[source_idx:])

    for candidate_relative_parts in resolve_source_variants(relative_parts):
        module_relative_key = "/".join([module_name] + [part.lower() for part in candidate_relative_parts])
        module_hits = by_module_relative_suffix.get(module_relative_key, set())
        if len(module_hits) == 1:
            return next(iter(module_hits))

        relative_key = "/".join(part.lower() for part in candidate_relative_parts)
        relative_hits = by_relative_suffix.get(relative_key, set())
        if len(relative_hits) == 1:
            return next(iter(relative_hits))

    return raw_file


def map_hapray_file_to_source(raw_file: str, source_file_index: Dict[str, Any]) -> str:
    resolved = map_build_cache_file_to_source(raw_file, source_file_index)
    if resolved != raw_file:
        return resolved
    return map_component_relative_file_to_source(raw_file, source_file_index)


def build_file_backsource_map(
    file_graph: Dict[str, Any],
    source_file_index: Dict[str, Any],
) -> Dict[str, str]:
    category_map = load_category_map(file_graph)
    by_path_key = source_file_index.get("by_path_key", {})
    result: Dict[str, str] = {}

    for node in iter_nodes(file_graph):
        if not isinstance(node, dict) or "name" not in node or not is_file_node(node, category_map):
            continue
        raw_file = str(node["name"])
        resolved_file = map_build_cache_file_to_source(raw_file, source_file_index)
        if resolved_file == raw_file:
            continue
        if normalize_path_key(resolved_file) in by_path_key:
            result[raw_file] = resolved_file

    return result


def collect_hapray_file_data(
    hapray_report: Dict[str, Any],
    source_file_index: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    Aggregate file timings using multiple views:
    - self: keep only self cost where symbolEvents > 0, deduplicated by
      (step, pid, tid, file, symbol) using max
    - total: aggregate fileEvents, deduplicated by (step, pid, tid, file)
      using max
    - raw: keep the legacy view by summing fileEvents directly per item, which
      is useful for comparing duplication amplification
    - backsource: try to map build/cache/*.ts back to src/*.ets and merge the
      timing into source files directly
    """

    path_raw_timing: Dict[str, int] = defaultdict(int)
    path_total_timing: Dict[str, int] = defaultdict(int)
    path_self_timing: Dict[str, int] = defaultdict(int)

    basename_raw_timing: Dict[str, int] = defaultdict(int)
    basename_total_timing: Dict[str, int] = defaultdict(int)
    basename_self_timing: Dict[str, int] = defaultdict(int)
    basename_owners: DefaultDict[str, Set[str]] = defaultdict(set)

    component_files_raw: DefaultDict[str, Dict[str, int]] = defaultdict(lambda: defaultdict(int))
    component_files_total: DefaultDict[str, Dict[str, int]] = defaultdict(lambda: defaultdict(int))
    component_files_self: DefaultDict[str, Dict[str, int]] = defaultdict(lambda: defaultdict(int))

    report_component_raw_timing: Dict[str, int] = defaultdict(int)
    report_component_total_timing: Dict[str, int] = defaultdict(int)
    report_component_self_timing: Dict[str, int] = defaultdict(int)

    file_total_buckets: Dict[Tuple[Any, ...], int] = {}
    file_self_buckets: Dict[Tuple[Any, ...], int] = {}
    component_total_buckets: Dict[Tuple[Any, ...], int] = {}
    component_self_buckets: Dict[Tuple[Any, ...], int] = {}

    path_display_names: Dict[str, str] = {}
    build_cache_backsource_map: Dict[str, str] = {}

    for item in get_hapray_data_list(hapray_report):
        raw_file = str(item.get("file", "") or "").strip()
        raw_component = str(item.get("componentName", "") or "").strip()
        canonical_component = normalize_har_name(raw_component) if raw_component else ""
        file_events = safe_int(item.get("fileEvents", item.get("count", item.get("cycles", 0))))
        symbol_events = safe_int(item.get("symbolEvents", 0))

        step_idx = item.get("stepIdx")
        pid = item.get("pid")
        tid = item.get("tid")
        process_name = str(item.get("processName", "") or "")
        symbol_name = str(
            item.get("symbol", "")
            or item.get("thirdCategoryName", "")
            or item.get("subCategoryName", "")
            or ""
        )

        if canonical_component:
            report_component_raw_timing[canonical_component] += file_events

        if not raw_file:
            continue

        resolved_file = raw_file
        if source_file_index:
            resolved_file = map_hapray_file_to_source(raw_file, source_file_index)
            if resolved_file != raw_file:
                build_cache_backsource_map[raw_file] = resolved_file

        path_key = normalize_path_key(resolved_file)
        if not path_key:
            continue

        path_display_names.setdefault(path_key, resolved_file)
        path_raw_timing[path_key] += file_events

        basename = last_path_part(resolved_file).lower()
        if basename:
            basename_owners[basename].add(path_key)

        file_group_key = (step_idx, pid, tid, process_name, path_key)
        file_total_buckets[file_group_key] = max(file_total_buckets.get(file_group_key, 0), file_events)

        if symbol_events > 0:
            symbol_group_key = (step_idx, pid, tid, process_name, path_key, symbol_name)
            file_self_buckets[symbol_group_key] = max(file_self_buckets.get(symbol_group_key, 0), symbol_events)

        if canonical_component:
            component_total_key = (canonical_component, step_idx, pid, tid, process_name, path_key)
            component_total_buckets[component_total_key] = max(
                component_total_buckets.get(component_total_key, 0),
                file_events,
            )

            if symbol_events > 0:
                component_self_key = (
                    canonical_component,
                    step_idx,
                    pid,
                    tid,
                    process_name,
                    path_key,
                    symbol_name,
                )
                component_self_buckets[component_self_key] = max(
                    component_self_buckets.get(component_self_key, 0),
                    symbol_events,
                )

            if not resolved_file.startswith("["):
                component_files_raw[canonical_component][resolved_file] += file_events

    for (_, _, _, _, path_key), timing in file_total_buckets.items():
        path_total_timing[path_key] += timing
    for (_, _, _, _, path_key, _), timing in file_self_buckets.items():
        path_self_timing[path_key] += timing

    for path_key, timing in path_raw_timing.items():
        basename = last_path_part(path_display_names.get(path_key, path_key)).lower()
        if basename:
            basename_raw_timing[basename] += timing
    for path_key, timing in path_total_timing.items():
        basename = last_path_part(path_display_names.get(path_key, path_key)).lower()
        if basename:
            basename_total_timing[basename] += timing
    for path_key, timing in path_self_timing.items():
        basename = last_path_part(path_display_names.get(path_key, path_key)).lower()
        if basename:
            basename_self_timing[basename] += timing

    for (component, _, _, _, _, path_key), timing in component_total_buckets.items():
        report_component_total_timing[component] += timing
        raw_file = path_display_names.get(path_key, path_key)
        if not raw_file.startswith("["):
            component_files_total[component][raw_file] += timing

    for (component, _, _, _, _, path_key, _), timing in component_self_buckets.items():
        report_component_self_timing[component] += timing
        raw_file = path_display_names.get(path_key, path_key)
        if not raw_file.startswith("["):
            component_files_self[component][raw_file] += timing

    return {
        "path_timing": {
            "self": dict(path_self_timing),
            "total": dict(path_total_timing),
            "raw": dict(path_raw_timing),
        },
        "basename_timing": {
            "self": dict(basename_self_timing),
            "total": dict(basename_total_timing),
            "raw": dict(basename_raw_timing),
        },
        "basename_counts": {name: len(owners) for name, owners in basename_owners.items()},
        "component_files": {
            "self": {name: dict(files) for name, files in component_files_self.items()},
            "total": {name: dict(files) for name, files in component_files_total.items()},
            "raw": {name: dict(files) for name, files in component_files_raw.items()},
        },
        "report_component_timing": {
            "self": dict(report_component_self_timing),
            "total": dict(report_component_total_timing),
            "raw": dict(report_component_raw_timing),
        },
        "build_cache_backsource_map": dict(build_cache_backsource_map),
    }


def build_suffix_lookup(path_timing: Dict[str, int]) -> Dict[str, Tuple[int, Set[str]]]:
    suffix_lookup: Dict[str, Tuple[int, Set[str]]] = {}
    owners: DefaultDict[str, Set[str]] = defaultdict(set)
    values: DefaultDict[str, int] = defaultdict(int)

    for path_key, timing in path_timing.items():
        parts = path_key.split("/")
        max_len = min(len(parts), 8)
        for size in range(2, max_len + 1):
            suffix = "/".join(parts[-size:])
            owners[suffix].add(path_key)
            values[suffix] += timing

    for suffix, owned_paths in owners.items():
        suffix_lookup[suffix] = (values[suffix], owned_paths)
    return suffix_lookup


def resolve_file_timing(
    file_path: str,
    path_timing: Dict[str, int],
    basename_timing: Dict[str, int],
    basename_counts: Dict[str, int],
    suffix_lookup: Dict[str, Tuple[int, Set[str]]],
) -> int:
    path_key = normalize_path_key(file_path)
    if path_key in path_timing:
        return path_timing[path_key]

    parts = path_key.split("/") if path_key else []
    max_len = min(len(parts), 8)
    for size in range(max_len, 1, -1):
        suffix = "/".join(parts[-size:])
        timing, owners = suffix_lookup.get(suffix, (0, set()))
        if len(owners) == 1:
            return timing

    basename = last_path_part(file_path).lower()
    if basename and basename_counts.get(basename) == 1:
        return basename_timing.get(basename, 0)
    return 0


def choose_unified_timing(
    self_cycles: int,
    total_cycles: int,
    raw_cycles: int,
    official_cycles: int = 0,
    *,
    prefer_official: bool = False,
) -> Tuple[int, str]:
    candidates: List[Tuple[str, int]] = []
    if prefer_official:
        candidates.append(("official", official_cycles))
    candidates.extend(
        [
            ("total", total_cycles),
            ("self", self_cycles),
            ("raw", raw_cycles),
        ]
    )
    for source, value in candidates:
        normalized = safe_int(value)
        if normalized > 0:
            return normalized, source
    return 0, ""


def apply_timing_metrics(node: Dict[str, Any], metric_maps: Dict[str, Dict[str, int]], key: str) -> None:
    self_cycles = safe_int(metric_maps.get("self", {}).get(key, 0))
    total_cycles = safe_int(metric_maps.get("total", {}).get(key, 0))
    raw_cycles = safe_int(metric_maps.get("raw", {}).get(key, 0))
    official_cycles = safe_int(metric_maps.get("official", {}).get(key, 0))
    unified_cycles, unified_source = choose_unified_timing(
        self_cycles,
        total_cycles,
        raw_cycles,
        official_cycles,
        prefer_official="official" in metric_maps,
    )

    node["timing_cycles"] = unified_cycles
    node["timing_unified_cycles"] = unified_cycles
    node["timing_unified_source"] = unified_source
    node["timing_self_cycles"] = self_cycles
    node["timing_total_cycles"] = total_cycles
    node["timing_raw_cycles"] = raw_cycles
    if "official" in metric_maps:
        node["timing_official_cycles"] = official_cycles

    baseline = total_cycles or self_cycles
    if baseline > 0 and raw_cycles > baseline:
        node["timing_duplication_factor"] = round(raw_cycles / baseline, 4)


def infer_file_to_har_map(
    file_graph: Dict[str, Any],
    module_graph: Dict[str, Any],
) -> Tuple[Dict[str, str], List[str], List[str], Set[str], str]:
    category_map = load_category_map(file_graph)
    internal_paths = [
        str(node["name"])
        for node in iter_nodes(file_graph)
        if (
            isinstance(node, dict)
            and "name" in node
            and is_file_node(node, category_map)
            and not is_non_source_artifact_file(str(node["name"]))
        )
    ]
    common_prefix = compute_common_dir_prefix(internal_paths)
    root_har_name = derive_root_har_name(common_prefix)

    file_to_har: Dict[str, str] = {}
    inferred_hars: List[str] = []
    seen_hars: Set[str] = set()

    module_raw_paths: List[str] = []
    for node in iter_nodes(module_graph):
        if not isinstance(node, dict) or "name" not in node:
            continue
        module_raw_paths.append(str(node["name"]))

    module_names, module_container_names = build_leaf_module_names(module_raw_paths)
    seen_hars.update(module_names)

    for node in iter_nodes(file_graph):
        if not isinstance(node, dict) or "name" not in node or not is_file_node(node, category_map):
            continue
        file_path = str(node["name"])
        if is_non_source_artifact_file(file_path):
            continue
        har_name = infer_internal_har_name(
            file_path,
            common_prefix,
            root_har_name,
            module_names,
        )
        file_to_har[file_path] = har_name
        if har_name not in seen_hars:
            inferred_hars.append(har_name)
            seen_hars.add(har_name)

    path_leakage_names, project_container_name = build_path_leakage_filters(
        common_prefix,
        internal_paths,
        module_raw_paths,
        module_names,
        module_container_names,
    )

    return (
        file_to_har,
        module_names,
        inferred_hars,
        path_leakage_names,
        project_container_name,
    )


def build_output(
    file_dep_graph: Dict[str, Any],
    module_dep_graph: Dict[str, Any],
    har_metric_maps: Dict[str, Dict[str, int]],
    file_to_har_map: Dict[str, str],
    file_backsource_map: Dict[str, str],
    file_metric_maps: Dict[str, Dict[str, int]],
    component_file_metric_maps: Dict[str, Dict[str, Dict[str, int]]],
    component_names: Sequence[str],
    runtime_component_names: Sequence[str],
    path_leakage_names: Set[str],
) -> Dict[str, Any]:
    file_category_map = load_category_map(file_dep_graph)
    file_nodes_by_id: Dict[Any, Dict[str, Any]] = {}
    file_id_to_har: Dict[Any, str] = {}
    file_id_alias: Dict[Any, Any] = {}
    file_graphs: DefaultDict[str, Dict[str, List[Dict[str, Any]]]] = defaultdict(
        lambda: {"nodes": [], "edges": []}
    )

    har_nodes_by_name: Dict[str, Dict[str, Any]] = {}
    har_edges_seen: Set[Tuple[str, str]] = set()
    har_edges: List[Dict[str, Any]] = []
    runtime_component_set = set(runtime_component_names)
    static_component_names = [name for name in component_names if name not in runtime_component_set]
    path_leakage_name_set = {name.lower() for name in path_leakage_names if name}
    representative_id_by_path: Dict[str, Any] = {}
    added_file_node_ids: Set[Any] = set()

    def is_path_leakage_name(name: str) -> bool:
        return bool(name) and name.lower() in path_leakage_name_set

    for node in iter_nodes(file_dep_graph):
        if not isinstance(node, dict) or "id" not in node or "name" not in node:
            continue
        if not is_file_node(node, file_category_map):
            continue
        raw_name = str(node["name"])
        if is_non_source_artifact_file(raw_name):
            continue
        display_name = file_backsource_map.get(raw_name, raw_name)
        if display_name == raw_name and display_name not in representative_id_by_path:
            representative_id_by_path[display_name] = node["id"]

    for node in iter_nodes(file_dep_graph):
        if not isinstance(node, dict) or "id" not in node or "name" not in node:
            continue
        if not is_file_node(node, file_category_map):
            continue
        raw_name = str(node["name"])
        display_name = file_backsource_map.get(raw_name, raw_name)
        representative_id_by_path.setdefault(display_name, node["id"])

    def ensure_har_node(name: str, *, external: bool = False, base_node: Optional[Dict[str, Any]] = None) -> None:
        if not name:
            return
        current = har_nodes_by_name.get(name)
        if current is None:
            node = dict(base_node or {})
            node["id"] = base_node.get("id") if base_node and "id" in base_node else f"har:{name}"
            node["name"] = name
            apply_timing_metrics(node, har_metric_maps, name)
            if external:
                node["kind"] = node.get("kind", "EXTERNAL")
            har_nodes_by_name[name] = node
            return
        apply_timing_metrics(current, har_metric_maps, name)
        if base_node and "id" in base_node and str(current.get("id", "")).startswith("har:"):
            current["id"] = base_node["id"]
        if external and "kind" not in current:
            current["kind"] = "EXTERNAL"

    def ensure_har_edge(source_name: str, target_name: str, count: int = 1) -> None:
        if not source_name or not target_name or source_name == target_name:
            return
        key = (source_name, target_name)
        if key not in har_edges_seen:
            har_edges_seen.add(key)
            har_edges.append(
                {
                    "source": har_nodes_by_name[source_name]["id"],
                    "target": har_nodes_by_name[target_name]["id"],
                    "count": count,
                }
            )
            return
        for edge in har_edges:
            if edge["source"] == har_nodes_by_name[source_name]["id"] and edge["target"] == har_nodes_by_name[target_name]["id"]:
                edge["count"] = int(edge.get("count", 1) or 1) + count
                return

    for node in iter_nodes(module_dep_graph):
        if not isinstance(node, dict) or "name" not in node:
            continue
        canonical = normalize_har_name(str(node["name"]))
        if is_path_leakage_name(canonical):
            continue
        ensure_har_node(canonical, base_node=node)

    for name in static_component_names:
        if is_path_leakage_name(name):
            continue
        ensure_har_node(name, external=True)

    for node in iter_nodes(file_dep_graph):
        if not isinstance(node, dict) or "id" not in node or "name" not in node:
            continue
        if is_file_node(node, file_category_map):
            raw_name = str(node["name"])
            display_name = file_backsource_map.get(raw_name, raw_name)
            representative_id = representative_id_by_path.get(display_name, node["id"])
            file_nodes_by_id[node["id"]] = node
            file_id_alias[node["id"]] = representative_id

            if is_non_source_artifact_file(raw_name):
                continue

            har_name = file_to_har_map.get(display_name, file_to_har_map.get(raw_name, ""))
            if not har_name:
                continue
            if is_path_leakage_name(har_name):
                continue
            file_id_to_har[node["id"]] = har_name
            file_id_to_har[representative_id] = har_name
            ensure_har_node(har_name)

            if representative_id != node["id"]:
                continue
            if representative_id in added_file_node_ids:
                continue
            added_file_node_ids.add(representative_id)

            current = dict(node)
            current["id"] = representative_id
            current["name"] = display_name
            if display_name != raw_name:
                current["origin"] = current.get("origin", "backsource")
                current["backsourceFrom"] = raw_name
            apply_timing_metrics(current, file_metric_maps, display_name)
            file_graphs[har_name]["nodes"].append(current)
    for edge in iter_edges(module_dep_graph):
        if not isinstance(edge, dict):
            continue
        source_id = edge.get("source")
        target_id = edge.get("target")
        source_name = ""
        target_name = ""
        for har_name, har_node in har_nodes_by_name.items():
            if har_node.get("id") == source_id:
                source_name = har_name
            if har_node.get("id") == target_id:
                target_name = har_name
        if source_name and target_name:
            ensure_har_edge(source_name, target_name)

    cross_tmp: DefaultDict[Any, DefaultDict[str, int]] = defaultdict(lambda: defaultdict(int))
    for edge in iter_edges(file_dep_graph):
        if not isinstance(edge, dict):
            continue
        original_source_id = edge.get("source")
        original_target_id = edge.get("target")
        original_source_node = file_nodes_by_id.get(original_source_id)
        original_target_node = file_nodes_by_id.get(original_target_id)
        if original_source_node and is_non_source_artifact_file(str(original_source_node.get("name", ""))):
            continue
        if original_target_node and is_non_source_artifact_file(str(original_target_node.get("name", ""))):
            continue

        source_id = file_id_alias.get(original_source_id, original_source_id)
        target_id = file_id_alias.get(original_target_id, original_target_id)
        if source_id == target_id:
            continue
        if source_id not in file_id_to_har:
            continue

        source_har = file_id_to_har[source_id]
        target_har = ""
        if target_id in file_id_to_har:
            target_har = file_id_to_har[target_id]
        else:
            target_node = file_nodes_by_id.get(target_id)
            if target_node:
                target_har = file_to_har_map.get(str(target_node["name"]), "")
            else:
                for node in iter_nodes(file_dep_graph):
                    if isinstance(node, dict) and node.get("id") == target_id and "name" in node:
                        target_har = normalize_har_name(str(node["name"]))
                        break

        if not target_har:
            continue
        if is_path_leakage_name(source_har) or is_path_leakage_name(target_har):
            continue

        ensure_har_node(source_har)
        ensure_har_node(target_har, external=target_id not in file_id_to_har)
        ensure_har_edge(source_har, target_har)

        if target_id in file_id_to_har and source_har == target_har:
            file_graphs[source_har]["edges"].append({"source": source_id, "target": target_id})
            continue

        cross_tmp[source_id][target_har] += 1

    allowed_component_files = {
        name for name in static_component_names if not is_path_leakage_name(name)
    }
    comp_self = component_file_metric_maps.get("self", {})
    comp_total = component_file_metric_maps.get("total", {})
    comp_raw = component_file_metric_maps.get("raw", {})
    for har_name in allowed_component_files:
        file_names = (
            set(comp_self.get(har_name, {}))
            | set(comp_total.get(har_name, {}))
            | set(comp_raw.get(har_name, {}))
        )
        if not file_names:
            continue

        ensure_har_node(har_name, external=True)
        existing_names = {str(node["name"]) for node in file_graphs[har_name]["nodes"] if "name" in node}
        metric_maps_for_component = {
            "self": comp_self.get(har_name, {}),
            "total": comp_total.get(har_name, {}),
            "raw": comp_raw.get(har_name, {}),
        }
        sorted_names = sorted(
            file_names,
            key=lambda raw_file, maps=metric_maps_for_component: (
                -choose_unified_timing(
                    safe_int(maps["self"].get(raw_file, 0)),
                    safe_int(maps["total"].get(raw_file, 0)),
                    safe_int(maps["raw"].get(raw_file, 0)),
                    prefer_official=False,
                )[0],
                raw_file,
            ),
        )
        for raw_file in sorted_names:
            if raw_file in existing_names:
                continue
            node = {
                "id": f"hapray:{har_name}:{raw_file}",
                "name": raw_file,
                "origin": "hapray",
            }
            apply_timing_metrics(node, metric_maps_for_component, raw_file)
            file_graphs[har_name]["nodes"].append(node)

    cross_har_dependencies: List[Dict[str, Any]] = []
    for source_id, target_map in cross_tmp.items():
        for target_har, count in sorted(target_map.items()):
            cross_har_dependencies.append(
                {"sourceId": source_id, "targetHarId": target_har, "count": count}
            )

    return {
        "metricInfo": {
            "defaultMetric": "unified",
            "metrics": {
                "unified": {
                    "field": "timing_unified_cycles",
                    "label": "Time cost",
                    "description": "Shown using the current time-cost metric. cycles = CPU cycle count.",
                },
            },
            "supplementaryMetrics": {
                "self": {
                    "field": "timing_self_cycles",
                    "label": "Self cycles",
                    "description": "Aggregated from de-duplicated symbolEvents by (step,pid,tid,file,symbol); counts self cost only.",
                },
                "total": {
                    "field": "timing_total_cycles",
                    "label": "Total cycles",
                    "description": "Aggregated from de-duplicated fileEvents by (step,pid,tid,file); represents total file hotness.",
                },
                "official": {
                    "field": "timing_official_cycles",
                    "label": "Official HAR cycles",
                    "description": "Official HAR-level statistics from component_timings.json / perf.har.",
                },
                "raw": {
                    "field": "timing_raw_cycles",
                    "label": "Raw fileEvents sum",
                    "description": "Legacy metric: direct sum of all fileEvents, which may be amplified by repeated symbols.",
                },
            },
        },
        "filters": {
            "pathLeakageNames": sorted(path_leakage_name_set),
        },
        "harGraph": {
            "nodes": sorted(har_nodes_by_name.values(), key=lambda item: (-(item.get("timing_cycles", 0) or 0), item["name"])),
            "edges": har_edges,
        },
        "fileGraphs": dict(file_graphs),
        "crossHarDependencies": cross_har_dependencies,
    }


def analyze_and_integrate_dependencies(
    file_dep_graph: Dict[str, Any],
    module_dep_graph: Dict[str, Any],
    hapray_report: Dict[str, Any],
    component_timings: List[Dict[str, Any]],
    output_path: str,
) -> None:
    har_official_timing_map = build_har_timing_map(component_timings, hapray_report)
    source_file_index = build_source_file_index(file_dep_graph)
    file_backsource_map = build_file_backsource_map(file_dep_graph, source_file_index)
    collected = collect_hapray_file_data(hapray_report, source_file_index)
    save_map_to_file(collected["build_cache_backsource_map"], "build_cache_backsource_map.json")
    save_map_to_file(file_backsource_map, "file_graph_backsource_map.json")

    har_metric_maps = {
        "self": collected["report_component_timing"]["self"],
        "total": collected["report_component_timing"]["total"],
        "raw": collected["report_component_timing"]["raw"],
        "official": har_official_timing_map,
    }
    har_unified_timing_map: Dict[str, int] = {}
    har_unified_source_map: Dict[str, str] = {}
    for har_name in (
        set(har_metric_maps["self"])
        | set(har_metric_maps["total"])
        | set(har_metric_maps["raw"])
        | set(har_metric_maps["official"])
    ):
        unified_cycles, unified_source = choose_unified_timing(
            safe_int(har_metric_maps["self"].get(har_name, 0)),
            safe_int(har_metric_maps["total"].get(har_name, 0)),
            safe_int(har_metric_maps["raw"].get(har_name, 0)),
            safe_int(har_metric_maps["official"].get(har_name, 0)),
            prefer_official=True,
        )
        har_unified_timing_map[har_name] = unified_cycles
        if unified_source:
            har_unified_source_map[har_name] = unified_source

    save_map_to_file(har_metric_maps["self"], "har_timing_map.json")
    save_map_to_file(har_metric_maps["total"], "har_timing_total_map.json")
    save_map_to_file(har_metric_maps["raw"], "har_timing_raw_map.json")
    save_map_to_file(har_metric_maps["official"], "har_timing_official_map.json")
    save_map_to_file(har_unified_timing_map, "har_timing_unified_map.json")
    save_map_to_file(har_unified_source_map, "har_timing_unified_source_map.json")

    (
        file_to_har_map,
        module_hars,
        inferred_hars,
        path_leakage_names,
        project_container_name,
    ) = infer_file_to_har_map(file_dep_graph, module_dep_graph)
    save_map_to_file(file_to_har_map, "file_to_har_map.json")
    save_map_to_file(sorted(path_leakage_names), "path_leakage_names.json")
    save_map_to_file(project_container_name, "project_container_name.json")
    path_leakage_name_set = {name.lower() for name in path_leakage_names if name}

    basename_counts = collected["basename_counts"]
    suffix_lookup_self = build_suffix_lookup(collected["path_timing"]["self"])
    suffix_lookup_total = build_suffix_lookup(collected["path_timing"]["total"])
    suffix_lookup_raw = build_suffix_lookup(collected["path_timing"]["raw"])

    full_file_self_timing_map: Dict[str, int] = {}
    full_file_total_timing_map: Dict[str, int] = {}
    full_file_raw_timing_map: Dict[str, int] = {}
    full_file_unified_timing_map: Dict[str, int] = {}
    full_file_unified_source_map: Dict[str, str] = {}
    for node in iter_nodes(file_dep_graph):
        if not isinstance(node, dict) or "name" not in node:
            continue
        file_path = str(node["name"])
        self_cycles = resolve_file_timing(
            file_path,
            collected["path_timing"]["self"],
            collected["basename_timing"]["self"],
            basename_counts,
            suffix_lookup_self,
        )
        total_cycles = resolve_file_timing(
            file_path,
            collected["path_timing"]["total"],
            collected["basename_timing"]["total"],
            basename_counts,
            suffix_lookup_total,
        )
        raw_cycles = resolve_file_timing(
            file_path,
            collected["path_timing"]["raw"],
            collected["basename_timing"]["raw"],
            basename_counts,
            suffix_lookup_raw,
        )
        unified_cycles, unified_source = choose_unified_timing(
            self_cycles,
            total_cycles,
            raw_cycles,
            prefer_official=False,
        )
        full_file_self_timing_map[file_path] = self_cycles
        full_file_total_timing_map[file_path] = total_cycles
        full_file_raw_timing_map[file_path] = raw_cycles
        full_file_unified_timing_map[file_path] = unified_cycles
        if unified_source:
            full_file_unified_source_map[file_path] = unified_source
    save_map_to_file(full_file_self_timing_map, "full_file_to_timing_map.json")
    save_map_to_file(full_file_total_timing_map, "full_file_to_total_timing_map.json")
    save_map_to_file(full_file_raw_timing_map, "full_file_to_raw_fileevents_map.json")
    save_map_to_file(full_file_unified_timing_map, "full_file_to_unified_timing_map.json")
    save_map_to_file(full_file_unified_source_map, "full_file_to_unified_timing_source_map.json")

    component_file_metric_maps = collected["component_files"]
    component_file_unified_maps: Dict[str, Dict[str, int]] = {}
    for component_name in (
        set(component_file_metric_maps["self"])
        | set(component_file_metric_maps["total"])
        | set(component_file_metric_maps["raw"])
    ):
        unified_files: Dict[str, int] = {}
        file_names = (
            set(component_file_metric_maps["self"].get(component_name, {}))
            | set(component_file_metric_maps["total"].get(component_name, {}))
            | set(component_file_metric_maps["raw"].get(component_name, {}))
        )
        for file_name in file_names:
            unified_cycles, _ = choose_unified_timing(
                safe_int(component_file_metric_maps["self"].get(component_name, {}).get(file_name, 0)),
                safe_int(component_file_metric_maps["total"].get(component_name, {}).get(file_name, 0)),
                safe_int(component_file_metric_maps["raw"].get(component_name, {}).get(file_name, 0)),
                prefer_official=False,
            )
            if unified_cycles > 0:
                unified_files[file_name] = unified_cycles
        if unified_files:
            component_file_unified_maps[component_name] = unified_files

    def build_debug_component_payload(metric_name: str) -> Dict[str, List[Dict[str, Any]]]:
        component_files = (
            component_file_unified_maps
            if metric_name == "unified"
            else component_file_metric_maps[metric_name]
        )
        return {
            name: [
                {"name": file_name, "timing_cycles": timing}
                for file_name, timing in sorted(files.items(), key=lambda item: item[1], reverse=True)
            ]
            for name, files in component_files.items()
        }

    save_map_to_file(build_debug_component_payload("self"), "hapray_component_files.json")
    save_map_to_file(build_debug_component_payload("total"), "hapray_component_files_total.json")
    save_map_to_file(build_debug_component_payload("raw"), "hapray_component_files_raw.json")
    save_map_to_file(build_debug_component_payload("unified"), "hapray_component_files_unified.json")

    component_names = sorted(
        set(module_hars)
        | set(inferred_hars)
        | set(har_metric_maps["self"])
        | set(har_metric_maps["total"])
        | set(har_metric_maps["raw"])
        | set(har_metric_maps["official"])
    )
    component_names = [
        name for name in component_names if name and name.lower() not in path_leakage_name_set
    ]
    save_map_to_file(component_names, "har_candidates.json")

    static_component_names = set(module_hars) | set(inferred_hars)
    runtime_component_names = sorted(
        (
            set(har_metric_maps["self"])
            | set(har_metric_maps["total"])
            | set(har_metric_maps["raw"])
            | set(har_metric_maps["official"])
        )
        - static_component_names
    )
    runtime_component_names = [
        name
        for name in runtime_component_names
        if name and name.lower() not in path_leakage_name_set
    ]
    save_map_to_file(runtime_component_names, "runtime_component_names.json")

    file_metric_maps = {
        "unified": full_file_unified_timing_map,
        "self": full_file_self_timing_map,
        "total": full_file_total_timing_map,
        "raw": full_file_raw_timing_map,
    }

    final_output = build_output(
        file_dep_graph,
        module_dep_graph,
        {
            "unified": har_unified_timing_map,
            "unified_source": har_unified_source_map,
            **har_metric_maps,
        },
        file_to_har_map,
        file_backsource_map,
        file_metric_maps,
        {
            "unified": component_file_unified_maps,
            **component_file_metric_maps,
        },
        component_names,
        runtime_component_names,
        path_leakage_names,
    )
    with open(output_path, "w", encoding="utf-8") as handle:
        json.dump(final_output, handle, ensure_ascii=False, indent=2)
    print(f"\n✓ Generated {output_path}")


def integrate_artifact_bundle(request: IntegrationRequest) -> Path:
    input_dir = request.input_dir.resolve()
    if not input_dir.is_dir():
        raise FileNotFoundError(f"Directory does not exist: {input_dir}")

    global DEBUG_DIR
    global DEBUG_ENABLED
    if not request.debug_enabled:
        DEBUG_ENABLED = False
        DEBUG_DIR = None
    elif request.debug_dir is None:
        DEBUG_ENABLED = True
        DEBUG_DIR = str(input_dir / "debug")
    else:
        DEBUG_ENABLED = True
        DEBUG_DIR = str(request.debug_dir)

    def file_path(name: str) -> Path:
        return input_dir / name

    files = {
        "fileDepGraph": file_path("fileDepGraph.json"),
        "moduleDepGraph": file_path("moduleDepGraph.json"),
        "hapray_report": file_path("hapray_report.json"),
        "component_timings": file_path("component_timings.json"),
    }

    loaded: Dict[str, Any] = {}
    for key, path in files.items():
        try:
            with path.open("r", encoding="utf-8") as handle:
                loaded[key] = json.load(handle)
        except FileNotFoundError:
            raise FileNotFoundError(f"Missing file: {path}") from None
        except json.JSONDecodeError as err:
            raise ValueError(f"{path} is not valid JSON: {err}") from err

    out_path = request.output_path or (input_dir / "hierarchical_integrated_data.json")
    analyze_and_integrate_dependencies(
        loaded["fileDepGraph"],
        loaded["moduleDepGraph"],
        loaded["hapray_report"],
        loaded["component_timings"],
        str(out_path),
    )
    return Path(out_path)


def main() -> None:
    sys.exit("integrate_dep.py is an internal pipeline stage. Use arkts-migration-visualizer instead.")


if __name__ == "__main__":
    main()
