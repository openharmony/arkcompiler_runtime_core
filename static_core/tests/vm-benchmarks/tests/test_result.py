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

# flake8: noqa
# pylint: skip-file

import json
import io
import contextlib
from unittest import TestCase
from statistics import mean

from vmb.result import RunReport, TestResult, RunResult
from vmb.report import VMBReport


REPORT = '''
{
    "ext_info": {},
    "format_version": "2",
    "machine": {
        "devid": "xxx",
        "hardware": "arm64-v8a",
        "name": "Empty",
        "os": "xxx"
    },
    "run": {
        "end_time": "2023-04-14T19:04:12.00000+0300",
        "framework_version": "xxx",
        "job_url": "xxx",
        "mr_change_id": "xxx",
        "panda_commit_hash": "???",
        "panda_commit_msg": "???",
        "start_time": "2023-04-14T18:48:54.00000+0300"
    },
    "tests": [
        {
            "build": [
                {
                    "compiler": "c2abc",
                    "size": 18864,
                    "time": 800000000.0
                },
                {
                    "compiler": "ark_aot",
                    "size": 78520,
                    "time": 221090000.0
                }
            ],
            "code_size": 97384,
            "compile_status": 0,
            "compile_time": 1.02109,
            "component": "API",
            "execution_forks": [
                {
                    "avg_time": 94783.751799,
                    "iterations": [
                        93075.32413,
                        92386.986759,
                        90960.576873,
                        97360.491352,
                        97724.468958,
                        97692.124251
                    ],
                    "unit": "ns/op",
                    "warmup": []
                }
            ],
            "execution_status": 0,
            "gc_stats": null,
            "iterations_count": 0,
            "mean_time": 0.0,
            "mean_time_error_99": 0.0,
            "mem_bytes": 89336,
            "memory": null,
            "name": "Test_One_Blah",
            "perf": false,
            "perf_log_file": null,
            "pn_hash": "N/A",
            "power": {},
            "rank": 0,
            "stdev_time": 0.0,
            "tags": []
        },
        {
            "build": [
                {
                    "compiler": "c2abc",
                    "size": 18864,
                    "time": 800000000.0
                },
                {
                    "compiler": "ark_aot",
                    "size": 78520,
                    "time": 221090000.0
                }
            ],
            "code_size": 97384,
            "compile_status": 0,
            "compile_time": 1.02109,
            "component": "API",
            "execution_forks": [
                {
                    "avg_time": 94783.751799,
                    "iterations": [
                        93075.32413,
                        92386.986759,
                        90960.576873,
                        97360.491352,
                        97724.468958,
                        97692.124251
                    ],
                    "unit": "ns/op",
                    "warmup": []
                }
            ],
            "execution_status": 0,
            "gc_stats": null,
            "iterations_count": 0,
            "mean_time": 0.0,
            "mean_time_error_99": 0.0,
            "mem_bytes": 89336,
            "memory": null,
            "name": "Test_Two",
            "perf": false,
            "perf_log_file": null,
            "pn_hash": "N/A",
            "power": {},
            "rank": 0,
            "stdev_time": 0.0,
            "tags": []
        }
    ]
}
'''

REPORT2 = '''
{
    "ext_info": {},
    "format_version": "2",
    "machine": {
        "devid": "xxx",
        "hardware": "arm64-v8a",
        "name": "Empty",
        "os": "xxx"
    },
    "run": {
        "end_time": "2023-04-14T19:04:12.00000+0300",
        "framework_version": "xxx",
        "job_url": "xxx",
        "mr_change_id": "xxx",
        "panda_commit_hash": "???",
        "panda_commit_msg": "???",
        "start_time": "2023-04-14T18:48:54.00000+0300"
    },
    "tests": [
        {
            "build": [
            ],
            "code_size": 97384,
            "compile_status": 0,
            "compile_time": 1.02109,
            "component": "API",
            "execution_forks": [
                {
                    "avg_time": 94783.751799,
                    "iterations": [
                        93075.32413,
                        92386.986759,
                        90960.576873,
                        97360.491352,
                        97724.468958,
                        97692.124251
                    ],
                    "unit": "ns/op",
                    "warmup": []
                }
            ],
            "execution_status": 0,
            "gc_stats": null,
            "iterations_count": 0,
            "mean_time": 0.0,
            "mean_time_error_99": 0.0,
            "mem_bytes": 89336,
            "memory": null,
            "name": "Test_One_Blah",
            "perf": false,
            "perf_log_file": null,
            "pn_hash": "N/A",
            "power": {},
            "rank": 0,
            "stdev_time": 0.0,
            "tags": []
        },
        {
            "build": [
            ],
            "code_size": 97384,
            "compile_status": 0,
            "compile_time": 1.02109,
            "component": "API",
            "execution_forks": [
                {
                    "avg_time": 94783.751799,
                    "iterations": [
                        93075.32413,
                        92386.986759,
                        90960.576873,
                        97360.491352,
                        97724.468958,
                        97692.124251
                    ],
                    "unit": "ns/op",
                    "warmup": []
                }
            ],
            "execution_status": 1,
            "gc_stats": null,
            "iterations_count": 0,
            "mean_time": 0.0,
            "mean_time_error_99": 0.0,
            "mem_bytes": 89336,
            "memory": null,
            "name": "Test_Two",
            "perf": false,
            "perf_log_file": null,
            "pn_hash": "N/A",
            "power": {},
            "rank": 0,
            "stdev_time": 0.0,
            "tags": []
        }
    ]
}
'''


def test_load_report():
    obj = json.loads(REPORT)
    rep = RunReport.from_obj(**obj)
    test = TestCase()
    test.assertTrue(2 == len(rep.tests))
    test.assertTrue(6 == len(rep.tests[0].execution_forks[0].iterations))


def test_vmb_report():
    f = io.StringIO()
    lines = []
    test = TestCase()
    with contextlib.redirect_stdout(f):
        vmb_rep = VMBReport.parse(REPORT)
        vmb_rep.text_report(full=True)
        lines = [line.strip() for line in
                 f.getvalue().split("\n") if line]
    test.assertTrue('Test_One_Blah | 9.48e+04 | 9.74e+04 | 8.93e+04 | Passed  |' \
        in lines)
    test.assertTrue('2 tests; 0 failed; 0 excluded; ' \
           'Time(GM): 94783.8 Size(GM): 97384.0 ' \
           'RSS(GM): 89336.0' in lines)


def test_vmb_compare():
    f = io.StringIO()
    with contextlib.redirect_stdout(f):
        r1 = VMBReport.parse(REPORT)
        r2 = VMBReport.parse(REPORT2)
        cmp = VMBReport.compare_vmb(r1, r2)
        cmp.print(full=True)
        lines = [line.strip() for line in
                 f.getvalue().split("\n") if line]
    TestCase().assertTrue('Test_Two     ; Time: 9.49e+04->9.49e+04(same); ' \
           'Size: n/a; RSS: 8.93e+04->8.93e+04(same); new fail' \
        in lines)


def create_report(**iterations) -> VMBReport:
    tr = []
    for name, i in iterations.items():
        rr = RunResult(avg_time=mean(i), iterations=i, warmup=[])
        tr.append(TestResult(name, execution_status=0, execution_forks=[rr]))
    vmb_rep = VMBReport(report=RunReport(tests=tr))
    vmb_rep.recalc()
    return vmb_rep


def test_vmb_report_sanity():
    vmb_rep = create_report(test1=[1.0, 3.0])
    test = TestCase()
    test.assertTrue(vmb_rep.total_cnt == 1)
    test.assertTrue(vmb_rep.fail_cnt == 0)
    test.assertTrue(vmb_rep.rss_gm == 0)
    test.assertTrue(vmb_rep.time_gm == 2.0)
    test.assertTrue(vmb_rep.summary == '1 tests; 0 failed; 0 excluded; ' \
                              'Time(GM): 2.0 Size(GM): 0.0 RSS(GM): 0.0')
    vmb_rep.text_report(full=True)


def test_vmb_compare_flaky():
    f = io.StringIO()
    lines = []
    r1 = create_report(testA=[4.0], testB=[5.0], bad=[6.0])
    r2 = create_report(testA=[4.0], testB=[5.0], bad=[7.0])
    with contextlib.redirect_stdout(f):
        cmp = VMBReport.compare_vmb(r1, r2, flaky=['bad'])
        cmp.print(full=True)
        lines = [line.strip() for line in
                 f.getvalue().split("\n") if line]
    TestCase().assertTrue('Time: 4.93e+00->5.19e+00(worse +5.3%); Size: n/a; RSS: n/a' \
        in lines)
