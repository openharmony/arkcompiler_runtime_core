#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
from typing import List, Tuple
from subprocess import TimeoutExpired
from vmb.platform import PlatformBase
from vmb.unit import BenchUnit
from vmb.result import BUStatus
from vmb.tool import VmbToolExecError
from vmb.hook import HookRegistry
from vmb.cli import Args
from vmb.result import ExtInfo
from vmb.helpers import Timer

log = logging.getLogger('vmb')


class VmbRunner:
    def __init__(self, args: Args) -> None:
        self.platform = PlatformBase.create(args)
        log.info('Using platform: %s', self.platform.name)
        hook_names = self.platform.required_hooks
        # add some hooks depending on args
        if args.get('enable_gc_logs', False):
            hook_names.append('gclog')
        if args.get('cpumask', False):
            hook_names.append('cpumask')
        # add hooks specifically requested by platform
        hook_names += [h for h in args.hooks.split(',') if h.strip()]
        self.hooks = HookRegistry().register_all_by_name(hook_names, args)
        self.abort_on_fail = args.abort_on_fail
        self.dry_run = args.dry_run
        self.exclude_list = args.exclude_list
        self.fail_logs = args.fail_logs
        self.tests_per_batch = args.tests_per_batch

    def process_error(self, bu: BenchUnit, e: Exception) -> None:
        msg = str(e)
        if isinstance(e, VmbToolExecError):
            msg = e.out
            if BUStatus.COMPILATION_FAILED == bu.status:
                bu.result.compile_status = 1
                log.error('%s: compilation failed', bu.name)
            else:
                bu.status = BUStatus.EXECUTION_FAILED
        if isinstance(e, RuntimeError):
            bu.status = BUStatus.ERROR
        elif isinstance(e, TimeoutExpired):
            bu.status = BUStatus.TIMEOUT
        if self.fail_logs:
            bu.save_fail_log(self.fail_logs, msg)
        log.error(e)

    def run_one_unit(self, bu: BenchUnit) -> None:
        timer_unit = Timer()
        if bu.name in self.exclude_list:
            log.info('Excluding bench unit: %s', bu.name)
            bu.status = BUStatus.SKIPPED
            return
        log.info('Starting bench unit: %s', bu.name)
        try:
            self.hooks.run_before_unit(bu)
            timer_unit.start()
            self.platform.run_unit(bu)  # do actual work
            self.hooks.run_after_unit(bu)
            if BUStatus.PASS == bu.status:
                log.passed('%s: %f', bu.name, bu.result.get_avg_time())
            elif BUStatus.COMPILATION_FAILED == bu.status:
                bu.result.compile_status = 1
                log.error('%s: compilation failed', bu.name)
            elif len(bu.result.execution_forks) == 0 and not self.dry_run:
                raise VmbToolExecError('No benchmark iterations!')
            elif not self.dry_run:
                log.error('%s: failed', bu.name)
        except (VmbToolExecError, TimeoutExpired, RuntimeError) as e:
            self.process_error(bu, e)
            if self.abort_on_fail:
                log.fatal('Aborting on first fail...')
                return
        except KeyboardInterrupt as e:
            raise KeyboardInterrupt() from e
        finally:
            if not self.dry_run:
                self.platform.cleanup(bu)
            timer_unit.finish()
            log.trace('Bench total time: %s %f', bu.name,
                      timer_unit.elapsed().total_seconds())

    def run_suite_batch(self, bench_units: List[BenchUnit]) -> List[BenchUnit]:
        for i in range(0, len(bench_units), self.tests_per_batch):
            log.info('Batch run %s tests, starting from %d', self.tests_per_batch, i + 1)
            try:
                self.platform.run_batch(bench_units[i:i + self.tests_per_batch])
            except (VmbToolExecError, TimeoutExpired, RuntimeError):
                log.error('Batch run failed in pre-phase!')
                for bu in bench_units[i:i + self.tests_per_batch]:
                    bu.status = BUStatus.ERROR
                continue
            except KeyboardInterrupt:
                log.warning('Aborting batch run...')
                break
            for bu in bench_units[i:i + self.tests_per_batch]:
                try:
                    self.run_one_unit(bu)
                except KeyboardInterrupt:
                    log.warning('Aborting run...')
                    break
        return bench_units

    def run_suite_serial(self, bench_units: List[BenchUnit]) -> List[BenchUnit]:
        for bu in bench_units:
            try:
                self.run_one_unit(bu)
            except KeyboardInterrupt:
                log.warning('Aborting run...')
                break
        return bench_units

    def run(self, bench_units: List[BenchUnit]) -> Tuple[List[BenchUnit], ExtInfo, Timer]:
        log.info("Starting RUN phase...")
        timer_suite = Timer()
        self.hooks.run_before_suite(self.platform)
        # run suite in serial or batch mode
        # if platform expose 'run_batch' method run it in batch mode
        # otherwise - one by one
        run_batch = getattr(self.platform, 'run_batch', None)
        bus = self.run_suite_batch(bench_units) \
            if run_batch and callable(run_batch) \
            else self.run_suite_serial(bench_units)
        self.hooks.run_after_suite(self.platform)
        timer_suite.finish()
        elapsed = timer_suite.elapsed()
        if self.dry_run:
            log.passed('Dry run finished in %s', elapsed)
        else:
            log.passed('Run took %s', elapsed)
        return bus, self.platform.ext_info, timer_suite


if __name__ == '__main__':
    arg = Args()
    runner = VmbRunner(arg)
    runner.run(PlatformBase.search_units(arg.paths))
