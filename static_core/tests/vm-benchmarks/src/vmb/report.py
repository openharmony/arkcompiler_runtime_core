#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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
#

# pylint: disable=too-many-locals
from __future__ import annotations
import sys
import json
import re
import logging
from datetime import datetime, timezone
from typing import Union, Iterable, Optional, List, Dict, Tuple, Any
from dataclasses import dataclass
from statistics import geometric_mean, mean
from pathlib import Path
from vmb.helpers import Jsonable, pad_left, read_list_file, create_file, Timer
from vmb.unit import BenchUnit
from vmb.result import RunReport, TestResult, AotStatEntry, AotPasses, \
    MachineMeta, BUStatus
from vmb.cli import Args, Command


OptList = Optional[List[str]]
# results that differ less than 0.5% treated as the same
TOLERANCE = 0.5
log = logging.getLogger('vmb')


@dataclass
class VMBDiff(Jsonable):
    time: str
    size: str
    rss: str
    status: str

    def __str__(self):
        return f"Time: {self.time}; Size: {self.size}; " \
               f"RSS: {self.rss}; {self.status}"


class VMBComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.summary = ''
        self.tests = {}
        self._regressions = 0
        self._name_len = 0

    def add(self, t, diff) -> None:
        self.tests[t] = diff

    def set_summary(self, t_diff: str, s_diff: str, r_diff: str,
                    new: int, fixed: int, new_fail: int, missed: int) -> None:
        self.summary = f'Time: {t_diff}; Size: {s_diff}; RSS: {r_diff}'
        # pylint: disable-next=protected-access
        if self._regressions > 0:
            self.summary += f' New Fails: {new_fail}; Missed: {missed}'
        if new + fixed > 0:
            self.summary += f' New: {new}; Fixed: {fixed}'

    def print(self, full: bool = False) -> None:
        caption(self.title)
        caption(self.summary)
        for name, diff in self.tests.items():
            if not full and diff.status not in ('new fail', 'missed'):
                continue
            print(f'{name:<{self._name_len}}; {diff}')


@dataclass
class VMBReport(Jsonable):
    report: RunReport
    title: str = ''
    summary: str = ''
    time_gm: float = 0.0
    size_gm: float = 0.0
    rss_gm: float = 0.0
    total_cnt: int = 0
    excluded_cnt: int = 0
    fail_cnt: float = 0.0
    _name_len: int = 0

    @classmethod
    def parse(cls, text: str,
              exclude: OptList = None) -> VMBReport:
        obj = json.loads(text)
        rr = RunReport.from_obj(**obj)
        vmb_rep = VMBReport(report=rr)
        vmb_rep.recalc(exclude=exclude)
        return vmb_rep

    def recalc(self, exclude: OptList = None) -> None:
        times, sizes, rsss = [], [], []
        self.total_cnt, self.excluded_cnt, self.fail_cnt = 0, 0, 0
        if not exclude:
            exclude = []
        for t in self.report.tests:
            self.total_cnt += 1
            name = t.name
            if name in exclude:
                self.excluded_cnt += 1
                continue
            self._name_len = max(self._name_len, len(name))
            if not test_passed(t):
                self.fail_cnt += 1
                continue
            t._status = BUStatus.PASS  # pylint: disable=protected-access
            times.append(mean(e.avg_time for e in t.execution_forks))
            sizes.append(t.code_size)
            rsss.append(t.mem_bytes)
        self.time_gm = safe_geomean(times)
        self.size_gm = safe_geomean(sizes)
        self.rss_gm = safe_geomean(rsss)
        self.title = self.report.machine.name
        self.summary = f'{self.total_cnt} tests; ' \
                       f'{self.fail_cnt} failed; ' \
                       f'{self.excluded_cnt} excluded; ' \
                       f'Time(GM): {self.time_gm:.1f} ' \
                       f'Size(GM): {self.size_gm:.1f} ' \
                       f'RSS(GM): {self.rss_gm:.1f}'

    def text_report(self, full: bool = True,
                    exclude: OptList = None) -> None:
        self.recalc(exclude=exclude)
        caption(f'{self.title}')
        caption(f'{self.summary}')
        print()
        if self.fail_cnt > 0 or full:
            caption(f"{'Test':<{self._name_len}} |   Time   | "
                    f'CodeSize |   RSS    | Status  |')
        for t in sorted(self.report.tests, key=lambda x: x.name):
            if test_passed(t) and not full:
                continue
            print(f'{pad_left(t.name, self._name_len)} | {t} |')
        print()

    def csv_report(self, csv: Union[str, Path],
                   exclude: OptList = None) -> None:
        """Export results in CSV format."""
        self.recalc(exclude=exclude)
        csv_path = Path(csv)
        csv_path.parent.mkdir(parents=True, exist_ok=True)
        with create_file(csv_path) as f:
            f.write('Benchmark name,Time ns/op,Ext Time sec,'
                    "RSS max KB,Compile sec,Size bytes\n")
            for t in sorted(self.report.tests, key=lambda x: x.name):
                if test_passed(t):
                    # pylint: disable-next=protected-access
                    f.write(f'{t.name},{t.get_avg_time()},{t._ext_time},'
                            f"{t.mem_bytes},{t.compile_time},{t.code_size}\n")

    def json_report(self, js: Union[str, Path],
                    pretty: bool = True) -> None:
        js_path = Path(js)
        js_path.parent.mkdir(parents=True, exist_ok=True)
        indent = 4 if pretty else None
        with create_file(js_path) as f:
            f.write(self.report.js(indent=indent))

    @staticmethod
    def status_diff(status1: bool, status2: bool, is_flaky: bool) -> str:
        flaky = '(flaky)' if is_flaky else ''
        status_diff = f'both pass{flaky}'
        if not status1 and not status2:
            status_diff = f'both fail{flaky}'
        if status1 != status2:
            if status2:
                status_diff = f'fixed{flaky}'
            else:
                status_diff = f'new fail{flaky}'
        return status_diff

    @staticmethod
    def compare_vmb(r1: VMBReport, r2: VMBReport,
                    flaky: OptList = None
                    ) -> VMBComparison:
        if not flaky:
            flaky = []
        # create tmp dicts of tests for simple search of test by name
        results1 = {t.name: t for t in r1.report.tests}
        results2 = {t.name: t for t in r2.report.tests}
        tests1, tests2 = set(results1.keys()), set(results2.keys())
        comparison = VMBComparison()
        # pylint: disable-next=protected-access
        comparison._name_len = max(r1._name_len, r2._name_len)
        times1, times2, sizes1, sizes2, rss1, rss2 = [], [], [], [], [], []
        missed, new, fixed, new_fail = 0, 0, 0, 0
        # compare tests present in both reports
        for t in tests1.intersection(tests2):
            t1, t2 = results1[t], results2[t]
            status1, status2 = test_passed(t1), test_passed(t2)
            is_flaky = t in flaky
            status_diff = VMBReport.status_diff(status1, status2, is_flaky)
            if not is_flaky:
                if status2 and not status1:
                    fixed += 1
                elif status1 and not status2:
                    comparison._regressions += 1
                    new_fail += 1
            time_diff = diff_str(t1.mean_time, t2.mean_time)
            size_diff = diff_str(t1.code_size, t2.code_size)
            rss_diff = diff_str(t1.mem_bytes, t2.mem_bytes)
            comparison.add(t, VMBDiff(
                time_diff, size_diff, rss_diff, status_diff))
            if status1 and status2:
                times1.append(t1.mean_time)
                times2.append(t2.mean_time)
                sizes1.append(t1.code_size)
                sizes2.append(t2.code_size)
                rss1.append(t1.mem_bytes)
                rss2.append(t2.mem_bytes)
        for t in tests2.difference(tests1):
            comparison._regressions += 1
            missed += 1
            comparison.add(t, VMBDiff('n/a', 'n/a', 'n/a', 'missed'))
        for t in tests1.difference(tests2):
            comparison.add(t, VMBDiff('n/a', 'n/a', 'n/a', 'new'))
            new += 1
        comparison.title = f'Comparison: {short_title(r1.title, r2.title)}'
        # GM should be recalculated because
        # of possible difference in test set for report1 and report2
        t_diff = diff_str_gm(times1, times2)
        s_diff = diff_str_gm(sizes1, sizes2)
        r_diff = diff_str_gm(rss1, rss2)
        comparison.set_summary(t_diff, s_diff, r_diff,
                               new, fixed, new_fail, missed)
        return comparison


@dataclass
class AOTStatsDiff(Jsonable):
    methods: str
    time: str
    size: str


@dataclass
class AOTPassesDiff(Jsonable):
    runs: str
    ir_mem: str
    local_mem: str
    time: str


class AOTStatsComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.summary = ''
        self.libs = {}

    def add(self, t, diff):
        self.libs[t] = diff


class AOTPassesComparison(Jsonable):
    def __init__(self):
        self.title = ''
        self.optimizations = {}

    def add(self, opt, summary, diff):
        self.optimizations[opt] = {
            'summary': summary,
            'libs': diff
        }


def safe_geomean(numbers: Iterable[float]) -> float:
    if not numbers:
        return 0.0
    res = [x for x in numbers if x]
    if not res:
        return 0.0
    return geometric_mean(res)


def caption(msg):
    print()
    print(msg)
    print('='*len(msg))


def split_name(name):
    name = name[6:] if name.startswith('Panda-') else name
    m = re.search(r'(ECMA|ETS).*(AOT|INT|JIT).*-(Master|MR)$', name)
    if m:
        return f'{m.group(1)} {m.group(2)}', f'{m.group(3)}'
    return name, ''


def short_title(t1, t2):
    n1, m1 = split_name(t1)
    n2, m2 = split_name(t2)
    if n1 == n2:
        return f'{n1} {m1} vs {m2}'
    x1 = f'{n1} {m1}'.rstrip()
    x2 = f'{n2} {m2}'.rstrip()
    return f'{x1} vs {x2}'


def diff_str(x, y, less_is_better=True):
    if not x or not y:
        # cannot compare if one of the results is missing
        return 'n/a'
    try:  # just to make linter happy
        diff_percent = (y/x-1.0)*100
    except ZeroDivisionError:
        return 'n/a'
    if abs(diff_percent) < TOLERANCE:
        return f'{x:.2e}->{y:.2e}(same)'
    better = (less_is_better and diff_percent < 0) \
        or (not less_is_better and diff_percent > 0)
    return f'{x:.2e}->{y:.2e}' \
           f'({"better" if better else "worse"} {diff_percent:+.1f}%)'


def diff_str_gm(xs, ys, less_is_better=True):
    xs_gm = safe_geomean(xs)
    ys_gm = safe_geomean(ys)
    return diff_str(xs_gm, ys_gm, less_is_better)


def test_passed(t: TestResult) -> bool:
    return (
        t._status != BUStatus.ERROR and  # pylint: disable=protected-access
        t.compile_status == 0 and
        t.execution_status == 0 and
        len(t.execution_forks) > 0
    )


def print_flaky(flaky):
    if flaky:
        caption('Flaky tests:')
        for t in flaky:
            print(t)


def get_common_aotstats(r1: VMBReport, r2: VMBReport
                        ) -> Tuple[AotStatEntry, AotStatEntry]:
    libname = 'eststdlib'
    if 'eststdlib' in r1.report.ext_info and 'eststdlib' in r2.report.ext_info:
        return r1.report.ext_info[libname], r2.report.ext_info[libname]
    raise ValueError('Bad ext_info in aot report!')


def get_aot_passes(aot_stats: AotStatEntry) -> Dict[str, AotPasses]:
    """Return { optimization: { lib: [runs, ir_mem, loc_mem, time]...}."""
    passes = {}
    libs = aot_stats.keys()
    for lb in libs:
        lib = aot_stats[lb]
        passes[lb] = lib.aot_stats.passes
    all_optimisations = list(
        set().union(*[set(x.keys()) for x in passes.values()]))
    all_optimisations.sort()
    # number of runs, ir mem, local mem, time
    passes_stats = {opt: {lb: passes[lb][opt]
                          for lb in libs if opt in passes[lb]}
                    for opt in all_optimisations}
    return passes_stats


def compare_aot_stats(r_1: VMBReport, r_2: VMBReport) -> AOTStatsComparison:
    comparison = AOTStatsComparison()
    comparison.title = \
        f'Comparison AOT stats: {short_title(r_1.title, r_2.title)}'
    times1, sizes1, times2, sizes2 = [], [], [], []
    r1, r2 = get_common_aotstats(r_1, r_2)
    # only for libs present in both reports
    for lib in set(r1.keys()).intersection(set(r2.keys())):
        l1 = r1[lib]
        l2 = r2[lib]
        times1.append(l1.time)
        times2.append(l2.time)
        sizes1.append(l1.size)
        sizes2.append(l2.size)
        meth_diff = diff_str(
            l1.aot_stats.number_of_methods,
            l2.aot_stats.number_of_methods, less_is_better=False)
        time_diff = diff_str(l1.time, l2.time)
        size_diff = diff_str(l1.size, l2.size)
        comparison.add(lib, AOTStatsDiff(meth_diff, time_diff, size_diff))
    gm_times1 = safe_geomean(times1)
    gm_times2 = safe_geomean(times2)
    t_diff = diff_str(gm_times1, gm_times2)
    gm_sizes1 = safe_geomean(sizes1)
    gm_sizes2 = safe_geomean(sizes2)
    s_diff = diff_str(gm_sizes1, gm_sizes2)
    comparison.summary = f'Time: {t_diff}; Size: {s_diff}'
    return comparison


def compare_aot_passes(r_1: VMBReport, r_2: VMBReport) -> AOTPassesComparison:
    r1, r2 = get_common_aotstats(r_1, r_2)
    passes1 = get_aot_passes(r1)
    passes2 = get_aot_passes(r2)
    opts1 = set(passes1.keys())
    opts2 = set(passes2.keys())
    opts = opts1.intersection(opts2)
    comparison = AOTPassesComparison()
    comparison.title = \
        f'Comparison AOT passes: {short_title(r_1.title, r_2.title)}'
    for opt in opts:
        opt1 = passes1[opt]
        opt2 = passes2[opt]
        time1, ir1, loc1 = [], [], []
        time2, ir2, loc2 = [], [], []
        opt_diff = {}
        for lib in set(opt1.keys()).intersection(set(opt2.keys())):
            runs_diff = diff_str(
                opt1[lib].runs, opt2[lib].runs, less_is_better=False)
            ir_diff = diff_str(opt1[lib].ir_mem, opt2[lib].ir_mem)
            local_diff = diff_str(opt1[lib].local_mem, opt2[lib].local_mem)
            time_diff = diff_str(opt1[lib].time, opt2[lib].time)
            opt_diff[lib] = AOTPassesDiff(
                runs_diff, ir_diff, local_diff, time_diff)
            time1.append(opt1[lib].time)
            time2.append(opt2[lib].time)
            ir1.append(opt1[lib].ir_mem)
            ir2.append(opt2[lib].ir_mem)
            loc1.append(opt1[lib].local_mem)
            loc2.append(opt2[lib].local_mem)
        tm_gm = diff_str_gm(time1, time2)
        ir_gm = diff_str_gm(ir1, ir2)
        loc_gm = diff_str_gm(loc1, loc2)
        comparison.add(
            opt,
            f'Time: {tm_gm}; IR size: {ir_gm}; Local size: {loc_gm}',
            opt_diff)
    return comparison


def compare_reports(args):
    if len(args.paths) != 2:
        print('Need 2 reports for comparison')
        sys.exit(1)
    else:
        flaky = read_list_file(args.flaky_list) if args.flaky_list else []
        with open(args.paths[0], 'r', encoding="utf-8") as f1:
            r1 = VMBReport.parse(f1.read())
        with open(args.paths[1], 'r', encoding="utf-8") as f2:
            r2 = VMBReport.parse(f2.read())
        cmp_vmb = VMBReport.compare_vmb(r1, r2, flaky=flaky)
        cmp_vmb.print(full=args.full)
        if args.json:
            cmp_vmb.save(args.json)
        if args.aot_stats_json:
            cmp_stats = compare_aot_stats(r1, r2)
            cmp_stats.save(args.aot_stats_json)
            caption(cmp_stats.title)
            print(cmp_stats.summary)
        if args.aot_passes_json:
            cmp_passes = compare_aot_passes(r1, r2)
            cmp_passes.save(args.aot_passes_json)
        print_flaky(flaky)
        sys.exit(  # pylint: disable-next=protected-access
            0 if len(cmp_vmb.tests) > 0 and cmp_vmb._regressions == 0
            else 1)


def report_main(args: Args,
                bus: Optional[List[BenchUnit]] = None,
                ext_info: Optional[Any] = None,
                timer: Optional[Timer] = None) -> None:
    # split into 2 func
    if ext_info is None:
        ext_info = {}
    # if called after run
    if bus:
        name = args.get('name', None)
        if not name:
            name = '-'.join([args.platform, args.mode])
        info = MachineMeta(name=name)
        rep = RunReport(machine=info,
                        tests=[bu.result for bu in bus],
                        ext_info=ext_info)
        if timer:
            rep.run.start_time = str(timer.begin)
            rep.run.end_time = str(timer.end)
        else:
            rep.run.end_time = str(datetime.now(timezone.utc))
        vmb_rep = VMBReport(report=rep)
        if args.report_json:
            compact = args.get('report_json_compact', False)
            vmb_rep.json_report(args.report_json, pretty=not compact)
        if args.report_csv:
            vmb_rep.csv_report(args.report_csv)
        vmb_rep.text_report(full=True, exclude=args.exclude_list)
        return
    # if called standalone
    if not args.paths:
        print('Report files (paths) missed')
        Args.print_help(Command.REPORT)
    global TOLERANCE
    TOLERANCE = args.tolerance
    if args.compare:
        compare_reports(args)
    else:
        flaky = read_list_file(args.flaky_list) if args.flaky_list else []
        with open(args.paths[0], 'r', encoding="utf-8") as f:
            r = VMBReport.parse(f.read(), exclude=flaky)
        r.text_report(full=args.full, exclude=args.exclude_list)
        print_flaky(flaky)
        passed_cnt = r.total_cnt - r.excluded_cnt - r.fail_cnt
        sys.exit(0 if passed_cnt > 0 and r.fail_cnt == 0 else 1)


if __name__ == '__main__':
    report_main(Args())
