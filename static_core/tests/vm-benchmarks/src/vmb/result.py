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

import logging
import statistics
import inspect
from collections import namedtuple
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional

from vmb.helpers import Jsonable, StringEnum

Stat = namedtuple("Stat",
                  "runs ir_mem local_mem time")
AotPasses = Dict[str, Stat]
log = logging.getLogger('vmb')


class BUStatus(StringEnum):
    """Enum for distinguish status of Unit."""

    NOT_RUN = 'Not Run'
    SKIPPED = 'Skipped'
    COMPILATION_FAILED = 'CompErr'
    EXECUTION_FAILED = 'Failed'
    ERROR = 'Error'
    TIMEOUT = 'Timeout'
    PASS = 'Passed'


@dataclass
class BuildResult(Jsonable):
    """Metrics for compilation step."""

    compiler: str
    size: int = 0
    time: float = 0.0
    rss: int = 0
    error: Optional[str] = None  # not used yet


@dataclass
class GCStats(Jsonable):
    gc_avg_bytes_reclaimed: int = 0
    gc_avg_interval_time: float = 0.0
    gc_avg_time: float = 0.0
    gc_max_time: int = 0
    gc_median_time: int = 0
    gc_memory_total_heap: int = 0
    gc_min_time: int = 0
    gc_name: str = ''
    gc_pause_count: int = 0
    gc_pauses: List[Any] = field(default_factory=list)
    gc_pct50_time: int = 0
    gc_pct95_time: int = 0
    gc_pct99_time: int = 0
    gc_std_time: float = 0.0
    gc_total_bytes_reclaimed: int = 0
    gc_total_time: float = 0.0
    gc_total_time_sum: float = 0.0
    gc_vm_time: float = 0.0
    time_unit: str = 'ns'


@dataclass
class AOTStats(Jsonable):
    number_of_methods: int = 0
    passes: AotPasses = field(default_factory=dict)

    @classmethod
    def from_obj(cls, **kwargs):
        kwargs = {
            k: v for k, v in kwargs.items()
            if k in inspect.signature(cls).parameters
        }
        for k, v in kwargs.items():
            if 'passes' == k:
                kwargs[k] = {n: Stat(*i) for n, i in v.items()}
        return cls(**kwargs)


@dataclass
class AOTStatsLib(Jsonable):
    aot_stats: AOTStats = field(default_factory=AOTStats)
    size: int = 0
    time: float = 0.0

    @classmethod
    def from_obj(cls, **kwargs):
        kwargs = {
            k: v for k, v in kwargs.items()
            if k in inspect.signature(cls).parameters
        }
        for k, v in kwargs.items():
            if 'aot_stats' == k:
                kwargs[k] = AOTStats.from_obj(**v)
        return cls(**kwargs)


AotStatEntry = Dict[str, AOTStatsLib]
ExtInfo = Dict[str, AotStatEntry]


@dataclass
class RunResult(Jsonable):
    """This is data structure to hold result of parsing output of sort.

    INFO - Startup execution started: 1703945979537
    INFO - Tuning: 16777216 ops, 66.51878356933594 ns/op => 15033347 reps
    INFO - Iter 1:15033347 ops, 95.58749625083489 ns/op
    INFO - Benchmark result: DemoBench_Demo 95.58749625083489
    """

    avg_time: float = 0.0
    iterations: List[float] = field(default_factory=list)
    warmup: List[float] = field(default_factory=list)
    unit: str = 'ns/op'


@dataclass
class TestResult(Jsonable):
    # meta info:
    name: str
    component: str = ''
    tags: List[str] = field(default_factory=list)
    bugs: List[str] = field(default_factory=list)
    # build stats:
    compile_status: int = 0
    build: List[BuildResult] = field(default_factory=list)
    # exec stats:
    execution_status: int = -13
    execution_forks: List[RunResult] = field(default_factory=list)
    mem_bytes: int = 0
    gc_stats: Optional[GCStats] = None
    # this field seem unused: aot_stats
    # no-exportable:
    _status: BUStatus = BUStatus.NOT_RUN
    _ext_time: float = 0.0
    __test__ = None

    @classmethod
    def from_obj(cls, **kwargs):
        kwargs = {  # skipping 'unknown' props
            k: v for k, v in kwargs.items()
            if k in inspect.signature(cls).parameters
        }
        for k, v in kwargs.items():
            if 'build' == k:
                kwargs[k] = [BuildResult(**i) for i in v]
            if 'execution_forks' == k:
                kwargs[k] = [RunResult(**i) for i in v]
            if 'gc_stats' == k:
                kwargs[k] = GCStats(**v) if v else None
        return cls(**kwargs)

    def __str__(self) -> str:
        # Note: using avg, as it reported by test itself
        # not `mean_time` which re-calculate by iteration results
        if self._status == BUStatus.PASS:
            time = self.get_avg_time()
            return f'{time:.2e} | ' \
                   f'{self.code_size:.2e} | ' \
                   f'{self.mem_bytes:.2e} | {self._status.value:<7}'
        return ' | '.join(['.'*8]*3 + [f'{self._status.value:<7}'])

    def get_avg_time(self) -> float:
        """Get mean of avg_time by executions."""
        if not self.execution_forks:
            return 0.0
        return statistics.mean(f.avg_time for f in self.execution_forks)

    @property
    def code_size(self) -> int:
        return sum(b.size for b in self.build)

    @property
    def compile_time(self) -> float:
        return sum(b.time for b in self.build)

    @property
    def iterations_count(self) -> int:
        return sum(len(f.iterations) for f in self.execution_forks)

    @property
    def mean_time(self) -> float:
        if not self.execution_forks:
            return 0.0
        if not self.execution_forks[0].iterations:
            return 0.0
        return statistics.mean(self.execution_forks[0].iterations)

    @property
    def stdev_time(self) -> float:
        if not self.execution_forks:
            return 0
        if not self.execution_forks[0].iterations:
            return 0
        if len(self.execution_forks[0].iterations) < 2:
            return 0
        return statistics.stdev(self.execution_forks[0].iterations)


@dataclass
class RunMeta(Jsonable):
    start_time: str = ''
    end_time: str = ''
    framework_version: str = ''
    job_url: str = ''
    mr_change_id: str = ''
    panda_commit_hash: str = ''
    panda_commit_msg: str = ''


@dataclass
class MachineMeta(Jsonable):
    name: str = ''
    devid: str = ''
    hardware: str = ''
    os: str = ''


@dataclass
class RunReport(Jsonable):
    ext_info: ExtInfo = field(default_factory=dict)
    format_version: str = '2'
    machine: MachineMeta = field(default_factory=MachineMeta)
    run: RunMeta = field(default_factory=RunMeta)
    tests: List[TestResult] = field(default_factory=list)

    @classmethod
    def from_obj(cls, **kwargs):
        kwargs = {
            k: v for k, v in kwargs.items()
            if k in inspect.signature(cls).parameters
        }
        for k, v in kwargs.items():
            if 'machine' == k:
                kwargs[k] = MachineMeta(**v)
            if 'run' == k:
                kwargs[k] = RunMeta(**v)
            if 'tests' == k:
                kwargs[k] = [TestResult.from_obj(**i) for i in v]
            if 'ext_info' == k:
                ext = {}
                for lib, info in v.items():
                    ext[lib] = {n: AOTStatsLib.from_obj(**s)
                                for n, s in info.items()}
                kwargs[k] = ext
        return cls(**kwargs)
