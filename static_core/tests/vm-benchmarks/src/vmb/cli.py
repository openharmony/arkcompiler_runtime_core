#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import sys
import logging
import re
from typing import Any, List, Dict, Set
from pathlib import Path
from vmb.helpers import StringEnum, split_params, read_list_file, die
from vmb.tool import ToolMode, OptFlags

LOG_LEVELS = ('fatal', 'pass', 'error', 'warn', 'info', 'debug', 'trace')
log = logging.getLogger('vmb')


class Command(StringEnum):
    """VMB commands enum."""

    GEN = 'gen'
    RUN = 'run'
    REPORT = 'report'
    ALL = 'all'
    VERSION = 'version'
    LIST = 'list'


def comma_separated_list(arg_val: str) -> Set[str]:
    if not arg_val:
        return set()
    return set(split_params(arg_val))


def add_measurement_opts(parser: argparse.ArgumentParser) -> None:
    """Add options wich should be processed in @Benchmarks too."""
    parser.add_argument('-wi', '--warmup-iters', default=None, type=int,
                        help='Number of warmup iterations')
    parser.add_argument('-mi', '--measure-iters', default=None, type=int,
                        help='Number of measurement iterations')
    parser.add_argument('-it', '--iter-time', default=None, type=int,
                        help='Iteration time, sec')
    parser.add_argument('-wt', '--warmup-time', default=None, type=int,
                        help='Warmup iteration time, sec')
    parser.add_argument('-fi', '--fast-iters', default=None, type=int,
                        help='Number of fast iterations')
    parser.add_argument('-gc', '--sys-gc-pause', default=None, type=int,
                        help='If <val> >= 0 invoke GC twice '
                        'and wait <val> ms before iteration')
    parser.add_argument("-aot-co", "--aot-compiler-options", default=[],
                        type=str, action="append",
                        help="Sets ahead-of-time compiler options")
    parser.add_argument("-aot-lib-co", "--aot-lib-compiler-options", default=[],
                        type=str, action="append",
                        help="Sets ahead-of-time compiler options for libraries")
    parser.add_argument("-c", "--concurrency-level",
                        default=None, type=str,
                        help="Concurrency level (DEPRECATED)")
    parser.add_argument("-compiler-inlning", "--compiler-inlining",
                        default=None, type=str,
                        help="enable compiler inlining")


def add_gen_opts(parser: argparse.ArgumentParser, command: Command) -> None:
    parser.add_argument('-l', '--langs',
                        type=comma_separated_list,
                        default=set(),
                        required=(command == Command.GEN),
                        help='Comma-separated list of lang plugins')
    parser.add_argument('-o', '--outdir', default='generated', type=str,
                        help='Dir for generated benches')
    parser.add_argument('-t', '--tests',
                        default=set(),
                        type=comma_separated_list,
                        help='Filter by name (comma-separated list)')
    parser.add_argument('-L', '--src-langs',
                        default=set(),
                        type=comma_separated_list,
                        help='Override src file extentions '
                             '(comma-separated list)')


def add_run_opts(parser: argparse.ArgumentParser) -> None:
    parser.add_argument('-p', '--platform', type=str, required=True,
                        help='Platform plugin name')
    parser.add_argument('-m', '--mode', type=str,
                        default='default', choices=ToolMode.getall(), help='Run VM mode')
    parser.add_argument('-n', '--name', type=str,
                        default='', help='Description of run')
    parser.add_argument('--timeout', default=None, type=float, help='Timeout (seconds)')
    parser.add_argument('--device', type=str, default='', help='Device ID (serial)')
    parser.add_argument('--device-host', type=str, default='',
                        help='device server in form server:port in case you use remote device')
    parser.add_argument('--device-dir', type=str, default='/data/local/tmp/vmb',
                        help='Base dir on device (%(default)s)')
    parser.add_argument('--hooks', type=str, default='',
                        help='Comma-separated list of hook plugins')
    parser.add_argument('-A', '--aot-skip-libs', action='store_true',
                        help='Skip AOT compilation of stdlib')
    parser.add_argument('-g', '--enable-gc-logs', action='store_true',
                        help='Runs benchmark with GC logs enabled')
    parser.add_argument('--dry-run', action='store_true',
                        help='Generate and compile, no execution')
    parser.add_argument('--report-json', default='', type=str, metavar='FILE_NAME',
                        help='Save json report as FILE_NAME')
    parser.add_argument('--report-json-compact', action='store_true',
                        help='Json file without indentation')
    parser.add_argument('--report-csv', default='', type=str,
                        metavar='FILE_NAME', help='Save csv report as FILE_NAME')
    parser.add_argument('--exclude-list', default='', type=str,
                        metavar='EXCLUDE_LIST', help='Path to exclude list')
    parser.add_argument('--fail-logs', default='', type=str,
                        metavar='FAIL_LOGS_DIR', help='Save failure messages to folder')
    parser.add_argument('--cpumask', default='', type=str,
                        help='Use cores mask in hex or bin format. '
                             'E.g., 0x38 or 0b111000 = high cores')
    parser.add_argument('--aot-stats', action='store_true',
                        help='Collect aot compilation data')
    parser.add_argument('--jit-stats', action='store_true',
                        help='Collect jit compilation data')
    parser.add_argument('--aot-whitelist', default='', type=str,
                        metavar='FILE_NAME', help='Get methods names from FILE_NAME')
    parser.add_argument('--tests-per-batch', default=25, type=int,
                        help='Test count per one batch run (%(default)s)')


def add_report_opts(parser: argparse.ArgumentParser) -> None:
    """Add options specific to vmb report."""
    parser.add_argument('--full', action='store_true',
                        help='Produce full report')
    parser.add_argument('--json', default='', type=str,
                        help='Save out as JSON')
    parser.add_argument('--aot-stats-json', default='', type=str,
                        help='File path to save aot stats comparison')
    parser.add_argument('--aot-passes-json', default='', type=str,
                        help='File path to save aot passes comparison')
    parser.add_argument("--gc-stats-json", default='', type=str,
                        help='File path to save GC stats comparison')
    parser.add_argument('--compare', action='store_true',
                        help='Compare 2 reports')
    parser.add_argument('--flaky-list', default='', type=str,
                        help='Exclude list file')
    parser.add_argument('--tolerance', default=0.5, type=float,
                        help='Percentage of tolerance in comparison')


def add_filter_opts(parser: argparse.ArgumentParser) -> None:
    parser.add_argument('-T', '--tags',
                        default=set(),
                        type=comma_separated_list,
                        help='Filter by tag (comma-separated list)')
    parser.add_argument('-ST', '--skip-tags',
                        default=set(),
                        type=comma_separated_list,
                        help='Skip if tagged (comma-separated list)')


class _ArgumentParser(argparse.ArgumentParser):

    def __init__(self, command: Command) -> None:
        super().__init__(
            prog=f'vmb {command.value}',
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=self.__epilog()
        )
        # Options common for all commands
        self.add_argument('paths', nargs='*', help='Dirs or files')
        self.add_argument('-e', '--extra-plugins', default=None,
                          help='Path to extra plugins')
        self.add_argument('-v', '--log-level', default='info',
                          choices=LOG_LEVELS,
                          help='Log level (default: %(default)s)')
        self.add_argument('--abort-on-fail', action='store_true',
                          help='Abort run on first error')
        self.add_argument('--no-color', action='store_true',
                          help='Disable color logging')
        # Generator-specific options
        if command in (Command.GEN, Command.ALL):
            add_gen_opts(self, command)
            add_measurement_opts(self)
            add_filter_opts(self)
        # Runner-specific options
        if command in (Command.RUN, Command.ALL):
            add_run_opts(self)
        if command in (Command.REPORT,):
            add_report_opts(self)
            add_filter_opts(self)

    @staticmethod
    def __epilog() -> str:
        return ''


class Args(argparse.Namespace):
    """Args parser for VMB."""

    def __init__(self) -> None:
        super().__init__()
        self.command = None
        self.custom_opts: Dict[str, List[str]] = {}
        if len(sys.argv) < 2 \
            or sys.argv[1] == 'help' \
                or sys.argv[1] not in Command.getall():
            print('Usage: vmb COMMAND [options] [paths]')
            print(f'       COMMAND {{{",".join(Command.getall())}}}')
            for c in Command:
                print('=' * 80)
                try:
                    Args.print_help(c)
                except SystemExit:
                    continue
            print('=' * 80)
            sys.exit(1)
        self.command = Command(sys.argv[1])
        args = sys.argv[2:]
        _, unknown = _ArgumentParser(self.command).parse_known_args(
            args, namespace=self)
        self.process_custom_opts(unknown)
        # provide some defaults
        self.paths: List[Path] = [Path(p) for p in self.paths] if self.paths \
            else [Path.cwd()]
        self.src_langs = {f'.{x.lstrip(".")}'
                          for x in self.get('src_langs', []) if x}
        # pylint: disable-next=access-member-before-definition
        if self.extra_plugins:  # type: ignore
            # pylint: disable-next=access-member-before-definition
            self.extra_plugins = Path(self.extra_plugins)  # type: ignore
        excl = self.get('exclude_list', None)
        self.exclude_list = read_list_file(excl) if excl else []
        self.dry_run = self.get('dry_run', False)
        self.fail_logs = self.get('fail_logs', '')
        if self.fail_logs:
            Path(self.fail_logs).mkdir(exist_ok=True)

    def __repr__(self) -> str:
        return '\n'.join(super().__repr__().split(','))

    @staticmethod
    def print_help(cmd: Command) -> None:
        """Print usage for VMB command."""
        _ArgumentParser(cmd).parse_args(['--help'])

    def process_custom_opts(self, custom: List[str]) -> None:
        re_custom = re.compile(r'^--(?P<tool>\w+)-custom-option=(?P<opt>.+)$')
        for opt in custom:
            m = re.search(re_custom, opt)
            if m:
                tool = m.group('tool')
                custom_opt = m.group('opt')
                if not self.custom_opts.get(tool):
                    self.custom_opts[tool] = [custom_opt]
                else:
                    self.custom_opts[tool].append(custom_opt)
            else:
                die(not m, f'Unknown option: {opt}')

    def get(self, arg: str, default=None) -> Any:
        return vars(self).get(arg, default)

    def get_opts_flags(self) -> OptFlags:
        flags = OptFlags.NONE
        mode = ToolMode(self.get('mode'))
        if ToolMode.AOT == mode:
            flags |= OptFlags.AOT
        elif ToolMode.AOTPGO == mode:
            flags |= OptFlags.AOTPGO
        elif ToolMode.LLVMAOT == mode:
            flags |= OptFlags.AOT | OptFlags.LLVMAOT
        elif ToolMode.INT == mode:
            flags |= OptFlags.INT
        elif ToolMode.INT_CPP == mode:
            flags |= OptFlags.INT | OptFlags.INT_CPP
        elif ToolMode.INT_IRTOC == mode:
            flags |= OptFlags.INT | OptFlags.INT_IRTOC
        elif ToolMode.INT_LLVM == mode:
            flags |= OptFlags.INT | OptFlags.INT_LLVM
        elif ToolMode.JIT == mode:
            flags |= OptFlags.JIT
        if self.get('dry_run', False):
            flags |= OptFlags.DRY_RUN
        if self.get('aot_skip_libs', False):
            flags |= OptFlags.AOT_SKIP_LIBS
        if self.get('enable_gc_logs', False):
            flags |= OptFlags.GC_STATS
        if self.get('aot_stats', False):
            flags |= OptFlags.AOT_STATS
        if self.get('jit_stats', False):
            flags |= OptFlags.JIT_STATS
        # backward compatibility
        if 'false' == self.get('compiler_inlining', ''):
            flags |= OptFlags.DISABLE_INLINING
        return flags

    def get_shared_path(self) -> str:
        path = self.get('outdir', '')
        if path:
            return path
        return self.get('path', ['.'])[0]
