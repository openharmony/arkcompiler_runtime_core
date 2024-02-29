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

import re
import logging
from typing import Optional, List, Union, Iterable
from pathlib import Path
from vmb.shell import ShellResult
from vmb.helpers import remove_prefix, norm_list
from vmb.result import TestResult, RunResult, BUStatus

BENCH_PREFIX = 'bench_'
UNIT_PREFIX = 'bu_'
LIB_PREFIX = 'lib'
log = logging.getLogger('vmb')


class BenchUnit:
    """BU represents single VM Benchmark test.

    Simply put, Unit consists of:
    1) directory containing program to run + libs + resources
    2) results of building and running this bench
    """

    # TODO: parse any unit (ns/op) and save
    __result_patt = re.compile(
        r'.*: ([\w-]+) (-?\d+(\.\d+)?([eE][+\-]\d+)?)')
    __warmup_patt = re.compile(
        r'.*Warmup \d+:.* (?P<value>\d+(\.\d+)?) ns/op')
    __iter_patt = re.compile(
        r'.*Iter \d+:.* (?P<value>\d+(\.\d+)?) ns/op')
    __real_tm_patt = re.compile(
        r"(?:Elapsed.*\(h:mm:ss or m:ss\)|Real time)[^:]*:\s*"
        r"(?:(\d*):)?(\d*)(?:.(\d*))?")

    def __init__(self,
                 path: Union[str, Path],
                 src: Optional[Union[str, Path]] = None,
                 libs: Optional[Iterable[Union[str, Path]]] = None,
                 tags: Optional[Iterable[str]] = None,
                 bugs: Optional[Iterable[str]] = None) -> None:
        self.path: Path = Path(path)
        self.__src: Optional[Path] = self.path / src if src else None
        self.__libs: List[Path] = [
            self.path / lib for lib in libs] if libs else []
        self.name: str = remove_prefix(self.path.name, UNIT_PREFIX)
        self.device_path: Optional[Path] = None
        self.result: TestResult = TestResult(
            self.name,
            tags=norm_list(tags),
            bugs=norm_list(bugs))
        self.run_out: str = ''

    @property
    def status(self) -> BUStatus:
        return self.result._status  # pylint: disable=protected-access

    @status.setter
    def status(self, stat: BUStatus) -> None:
        self.result._status = stat  # pylint: disable=protected-access

    def parse_ext_time(self, rss_out: str) -> float:
        m = re.search(self.__real_tm_patt, rss_out)
        if m is None:
            return 0.0
        tmp = m.groups()
        if tmp[0] is None:
            return round(float(str(tmp[1]) + "." + tmp[2]), 5)
        return round(int(tmp[0]) * 60 + float(str(tmp[1]) + "." + tmp[2]), 5)

    def parse_run_output(self, res: ShellResult) -> None:
        if not res.out:
            return
        self.run_out = res.out
        mtch = re.search(self.__result_patt, res.out)
        if mtch:
            if mtch.groups()[0] != self.name:
                log.warning('Name mismatch: %s vs %s',
                            mtch.groups()[0], self.name)
            avg_time = float(mtch.groups()[1])
            warms = [
                float(m.group("value"))
                for m in re.finditer(self.__warmup_patt, res.out)]
            iters = [
                float(m.group("value"))
                for m in re.finditer(self.__iter_patt, res.out)]
            self.result.execution_forks.append(
                RunResult(avg_time, iters, warms))
        self.result.mem_bytes = res.rss
        self.result.execution_status = \
            int(res.ret) if res.ret is not None else -1
        self.status = BUStatus.PASS \
            if 0 == self.result.execution_status \
            else BUStatus.EXECUTION_FAILED
        if not res.err:
            return
        # pylint: disable-next=protected-access
        self.result._ext_time = self.parse_ext_time(res.err)

    def src(self, *ext) -> Path:
        if self.__src:
            return self.__src  # regardless of ext
        files = [f for f in self.path.glob(f'{BENCH_PREFIX}{self.name}*')
                 if (not ext or f.suffix in ext)]
        if files:
            return files[0]
        # fallback: return unexistent
        return self.path / f'{BENCH_PREFIX}{self.name}'

    def libs(self, *ext) -> Iterable[Path]:
        if self.__libs:
            return [lib for lib in self.__libs
                    if (not ext or lib.suffix in ext)]
        # search all libs on filesystem:
        return {lib for lib in self.path.glob(f'{LIB_PREFIX}*')
                if (not ext or lib.suffix in ext)}

    def src_device(self, *ext) -> Path:
        if self.device_path is None:
            raise RuntimeError(f'Device path not set for {self.name}')
        return self.device_path / self.src(*ext).name

    def libs_device(self, *ext) -> Iterable[Path]:
        if self.device_path is None:
            raise RuntimeError(f'Device path not set for {self.name}')
        return [self.device_path / lib.name for lib in self.libs(*ext)]
