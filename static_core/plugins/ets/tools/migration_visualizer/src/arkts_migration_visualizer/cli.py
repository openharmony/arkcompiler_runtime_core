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

r"""
Package entrypoint for: collect four artifacts -> build visualization input
-> optionally start a local server and open the browser.
"""

from __future__ import annotations

import argparse
import os
import socket
import sys
import webbrowser
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Callable, List, Optional, TypeVar


PACKAGE_ROOT = Path(__file__).resolve().parent
SRC_ROOT = PACKAGE_ROOT.parent
PROJECT_ROOT = SRC_ROOT.parent
ARTIFACTS_DIR = PROJECT_ROOT / "artifacts"
WEB_DIR = SRC_ROOT / "web"

REQUIRED_FILES = (
    "fileDepGraph.json",
    "moduleDepGraph.json",
    "hapray_report.json",
    "component_timings.json",
)

DEFAULT_COMMAND = "run"
EXPORT_TIMELINE_COMMAND = "export-timeline"
_T = TypeVar("_T")


def log(msg, quiet=False):
    if not quiet:
        print(msg, flush=True)


def shell_python_command() -> str:
    return "python" if os.name == "nt" else "python3"


def run_stage_or_exit(stage_name: str, action: Callable[[], _T]) -> _T:
    try:
        return action()
    except SystemExit as exc:
        message = str(exc).strip()
        raise SystemExit(f"{stage_name} failed. {message}" if message else exc.code)
    except Exception as exc:
        raise SystemExit(f"{stage_name} failed. {exc}")


def run_collection_stage(
    testcase_regex: str,
    *,
    deps_root: str = "",
    manual_package: Optional[str] = None,
    manual_ability: Optional[str] = None,
    manual_duration: int = 30,
) -> Path:
    from arkts_migration_visualizer.collect.perf_runner import (
        CollectionRequest,
        collect_artifacts,
    )

    request = CollectionRequest(
        testcase_regex=None if manual_package else testcase_regex,
        manual_package=manual_package,
        manual_ability=manual_ability,
        manual_duration=manual_duration,
        deps_root=deps_root,
    )
    return Path(collect_artifacts(request))


def run_build_stage(run_dir: Path, output_path: Path) -> Path:
    from arkts_migration_visualizer.build.integrate_dep import (
        IntegrationRequest,
        integrate_artifact_bundle,
    )

    request = IntegrationRequest(
        input_dir=run_dir,
        output_path=output_path,
        debug_enabled=True,
    )
    return Path(integrate_artifact_bundle(request))


def run_timeline_export_stage(
    input_path: str,
    *,
    output_path: str = "",
    step: Optional[int] = None,
    event_name: str = "",
    thread_ids: Optional[List[int]] = None,
    merge_samples: bool = True,
):
    from arkts_migration_visualizer.collect.export_hapray_timeline_trace import (
        TimelineExportRequest,
        export_timeline_trace_artifact,
    )

    request = TimelineExportRequest(
        input_path=Path(input_path),
        output_path=output_path,
        step=step,
        event_name=event_name,
        thread_ids=tuple(thread_ids or ()),
        merge_samples=merge_samples,
    )
    return export_timeline_trace_artifact(request)


def find_latest_run_dir() -> Optional[Path]:
    if not ARTIFACTS_DIR.is_dir():
        return None
    candidates = sorted(
        [p for p in ARTIFACTS_DIR.glob("test_run_*") if p.is_dir()],
        key=lambda p: p.name,
        reverse=True,
    )
    return candidates[0] if candidates else None


def check_run_dir_has_required_files(run_dir: Path) -> bool:
    for name in REQUIRED_FILES:
        if not (run_dir / name).is_file():
            return False
    return True


def choose_port(preferred: int) -> int:
    port = preferred
    for _ in range(15):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            try:
                sock.bind(("127.0.0.1", port))
                return port
            except OSError:
                port += 1
    return preferred


def serve_web(port: int, open_browser: bool, quiet: bool):
    os.chdir(WEB_DIR)
    httpd = ThreadingHTTPServer(("0.0.0.0", port), SimpleHTTPRequestHandler)
    url = f"http://localhost:{port}/"
    log(f"\n🌐 Local server started: {url}", quiet)
    if open_browser:
        try:
            webbrowser.open(url)
        except Exception:
            pass
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        log("\n⏹️  Stopping server", quiet)
    finally:
        httpd.server_close()


def add_run_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--test", "-t", default=".*", help="Hapray testcase regex (default '.*')")
    parser.add_argument("--manual-package", default=None, help="Manual listening mode: target application bundle name")
    parser.add_argument("--manual-ability", default=None, help="Manual listening mode: target application main ability name")
    parser.add_argument(
        "--manual-duration",
        type=int,
        default=30,
        help="Manual listening mode: collection duration in seconds (default 30)",
    )
    parser.add_argument("--deps-root", default="", help="Temporarily override the dependency root (higher priority than config.json)")
    parser.add_argument("--skip-collect", action="store_true", help="Skip collection and build directly from existing outputs")
    parser.add_argument("--run-dir", help="Use an existing run directory (artifacts/test_run_...)")
    parser.add_argument("--serve", dest="serve", action="store_true", default=True, help="Start the local server (default)")
    parser.add_argument("--no-serve", dest="serve", action="store_false", help="Do not start the local server")
    parser.add_argument("--port", type=int, default=8000, help="Local server port (default 8000)")
    parser.add_argument("--no-open", dest="open_browser", action="store_false", help="Do not open the browser automatically")
    parser.add_argument("--quiet", action="store_true", help="Reduce log output")


def add_export_timeline_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("input", help="Path to a perf.db file, a step directory, or a Hapray case directory")
    parser.add_argument(
        "-o",
        "--output",
        default="",
        help="Output JSON path, default is PROJECT_ROOT/artifacts/hapray_timeline_trace/<source-relative>/hapray_timeline_trace.json",
    )
    parser.add_argument("--step", type=int, default=None, help="When input is a directory with multiple step subdirectories, choose step N")
    parser.add_argument(
        "--event-name",
        default="",
        help="Perf event to export, default picks the first available item from the preferred instruction/cycle list",
    )
    parser.add_argument(
        "--thread-id",
        action="append",
        type=int,
        default=[],
        help="Limit export to one or more thread ids; repeat this option to pass multiple threads",
    )
    parser.add_argument(
        "--no-merge-samples",
        action="store_true",
        help="Do not merge adjacent samples with the same call stack",
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="ArkTS Migration Visualizer packaged entrypoint",
    )
    subparsers = parser.add_subparsers(dest="command")
    run_parser = subparsers.add_parser(DEFAULT_COMMAND, help="Collect, build, and optionally serve visualization data")
    add_run_arguments(run_parser)
    export_parser = subparsers.add_parser(
        EXPORT_TIMELINE_COMMAND,
        help="Export Hapray perf.db sampling data as a timeline trace JSON file",
    )
    add_export_timeline_arguments(export_parser)
    return parser


def normalize_cli_argv(argv: List[str]) -> List[str]:
    if not argv:
        return [DEFAULT_COMMAND]
    if argv[0] in {DEFAULT_COMMAND, EXPORT_TIMELINE_COMMAND}:
        return argv
    if argv[0] in {"-h", "--help"}:
        return argv
    return [DEFAULT_COMMAND, *argv]


def run_workflow_command(args: argparse.Namespace) -> None:
    if args.manual_package and args.manual_duration <= 0:
        raise SystemExit("--manual-duration must be greater than 0")

    quiet = args.quiet

    WEB_DIR.mkdir(parents=True, exist_ok=True)
    ARTIFACTS_DIR.mkdir(parents=True, exist_ok=True)

    run_dir = None
    if not args.skip_collect:
        log("==> Starting collection (Homecheck + Hapray)", quiet)
        run_dir = run_stage_or_exit(
            "Collection",
            lambda: run_collection_stage(
                args.test,
                deps_root=args.deps_root,
                manual_package=args.manual_package,
                manual_ability=args.manual_ability,
                manual_duration=args.manual_duration,
            ),
        )
    else:
        log("==> Skipping collection", quiet)

    if args.run_dir:
        resolved_run_dir = Path(args.run_dir).resolve()
        if not resolved_run_dir.is_dir():
            sys.exit(f"Specified run directory does not exist: {resolved_run_dir}")
        run_dir = resolved_run_dir
    if run_dir is None:
        run_dir = find_latest_run_dir()
        if not run_dir:
            sys.exit("No artifacts/test_run_* directory was found. Run collection first or specify --run-dir.")

    log(f"Using run directory: {run_dir}", quiet)

    if not check_run_dir_has_required_files(run_dir):
        missing = [name for name in REQUIRED_FILES if not (run_dir / name).is_file()]
        sys.exit(f"Run directory is missing files: {missing}")

    out_json = WEB_DIR / "hierarchical_integrated_data.json"
    log("==> Building visualization input from four JSON files", quiet)
    generated_output = run_stage_or_exit("Build", lambda: run_build_stage(run_dir, out_json))
    if not generated_output.is_file():
        sys.exit("src/web/hierarchical_integrated_data.json was not generated")

    log(f"✓ Generated: {generated_output}", quiet)

    if args.serve:
        port = choose_port(args.port)
        if port != args.port:
            log(f"Note: port {args.port} is occupied, using {port} instead", quiet)
        log("==> Starting local server (Ctrl+C to stop)", quiet)
        serve_web(port=port, open_browser=args.open_browser, quiet=quiet)
    else:
        log("\nDone. The local server was not started. You can run:", quiet)
        log(f"  cd {WEB_DIR}", quiet)
        log(f"  {shell_python_command()} -m http.server 8000", quiet)


def run_export_timeline_command(args: argparse.Namespace) -> None:
    result = run_stage_or_exit(
        "Timeline export",
        lambda: run_timeline_export_stage(
            args.input,
            output_path=args.output,
            step=args.step,
            event_name=args.event_name,
            thread_ids=args.thread_id,
            merge_samples=not args.no_merge_samples,
        ),
    )

    print(f"Loading Hapray perf.db: {result.perf_db_path}")
    print(f"Exporting timeline trace to: {result.output_path}")
    print(f"Exported timeline trace: {result.output_path}")
    print(
        "Summary: "
        f"event={result.trace_obj['metadata']['eventName']} | "
        f"intervals={result.trace_obj['metadata']['intervalCount']} | "
        f"events={len(result.trace_obj['traceEvents'])}"
    )


def main(argv: Optional[List[str]] = None):
    raw_argv = list(argv) if argv is not None else sys.argv[1:]
    parser = build_parser()
    if raw_argv and raw_argv[0] in {"-h", "--help"}:
        parser.print_help()
        return

    args = parser.parse_args(normalize_cli_argv(raw_argv))
    if args.command == EXPORT_TIMELINE_COMMAND:
        run_export_timeline_command(args)
        return

    run_workflow_command(args)
