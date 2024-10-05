#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

import logging
import re
import subprocess
from unittest import TestCase
from abc import abstractmethod
from datetime import datetime
from functools import cached_property
from glob import glob
from os import path, environ
from pathlib import Path
from typing import List, Dict, Optional, Type, Tuple

from runner.utils import get_platform_binary_name
from runner.enum_types.configuration_kind import ConfigurationKind, SanitizerKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv
from runner.enum_types.qemu import QemuKind
from runner.logger import Log
from runner.options.config import Config
from runner.reports.report_format import ReportFormat
from runner.reports.detailed_report import DetailedReport
from runner.reports.spec_report import SpecReport
from runner.runner_base import Runner
from runner.test_base import Test
from runner.test_file_based import TestFileBased
from runner.code_coverage.coverage import LlvmCov
from runner.reports.html_view import HtmlView
from runner.reports.xml_view import XmlView
from runner.reports.standard_view import StandardView
from runner.reports.summary import Summary
from runner.plugins.work_dir import WorkDir


INDEX_TITLE = "${Title}"
INDEX_OPTIONS = "${Options}"
INDEX_TOTAL = "${Total}"
INDEX_PASSED = "${Passed}"
INDEX_FAILED = "${Failed}"
INDEX_IGNORED = "${Ignored}"
INDEX_EXCLUDED_LISTS = "${ExcludedThroughLists}"
INDEX_EXCLUDED_OTHER = "${ExcludedByOtherReasons}"
INDEX_TEST_NAME = "${TestName}"
INDEX_TEST_ID = "${TestId}"
INDEX_FAILED_TESTS_LIST = "${FailedTestsList}"


_LOGGER = logging.getLogger("runner.runner_file_based")


class PandaBinaries:
    DISABLE_CHECK_RUNTIME = ['parser']

    def __init__(self, runner_name: str, build_dir: str, config: Config, conf_kind: ConfigurationKind) -> None:
        self.build_dir = build_dir
        self.config = config
        self.conf_kind = conf_kind
        self.runner_name = runner_name

    @property
    def es2panda(self) -> str:
        if self.config.es2panda.custom_path is not None:
            es2panda = self.__get_binary_path(self.config.es2panda.custom_path)
        else:
            es2panda = self.__get_binary_path("es2panda")
        return es2panda

    @property
    def runtime(self) -> str:
        if self.runner_name in self.DISABLE_CHECK_RUNTIME:
            return ""
        return self.__get_binary_path('ark')

    @property
    def verifier(self) -> str:
        if not self.config.verifier.enable:
            return ""
        return self.__get_binary_path("verifier")

    @property
    def ark_aot(self) -> str:
        return self.__get_binary_path('ark_aot')

    @property
    def ark_quick(self) -> str:
        return ""

    @staticmethod
    def check_path(*path_parts: str) -> str:
        path_ = path.join(*path_parts)
        if not path.isfile(path_):
            Log.exception_and_raise(_LOGGER, f"Cannot find binary file: {path_}", FileNotFoundError)
        return str(path_)

    def __get_binary_path(self, binary: str) -> str:
        return self.check_path(self.build_dir, 'bin', get_platform_binary_name(binary))


class RunnerFileBased(Runner):
    def __init__(self, config: Config, name: str, panda_binaries: Type[PandaBinaries] = PandaBinaries) -> None:
        Runner.__init__(self, config, name)
        self.cmd_env = environ.copy()

        self.coverage = LlvmCov(self.build_dir, self.work_dir)

        self.__set_test_list_options()
        self.binaries = panda_binaries(name, self.build_dir, self.config, self.conf_kind)

        self.verifier_args: List[str] = []
        self.cmd_prefix = self._set_cmd_prefix(config)
        self.quick_args: List[str] = []

        self.runtime_args = self.__fill_up_runtime_args()
        self.ark_aot, self.aot_args = self.__fill_up_aot_args()

        self.test_env = TestEnv(
            config=config,
            conf_kind=self.conf_kind,
            cmd_prefix=self.cmd_prefix,
            cmd_env=self.cmd_env,
            coverage=self.coverage,
            es2panda=self.binaries.es2panda,
            es2panda_args=[],
            runtime=self.binaries.runtime,
            runtime_args=self.runtime_args,
            ark_aot=self.ark_aot,
            aot_args=self.aot_args,
            ark_quick=self.binaries.ark_quick,
            quick_args=self.quick_args,
            timestamp=int(datetime.timestamp(datetime.now())),
            report_formats={self.config.report.report_format},
            work_dir=self.work_dir,
            verifier=self.binaries.verifier,
            verifier_args=self.verifier_args
        )

    @property
    @abstractmethod
    def default_work_dir_root(self) -> Path:
        pass

    @staticmethod
    def _set_cmd_prefix(config: Config) -> List[str]:
        if config.general.qemu == QemuKind.ARM64:
            return ["qemu-aarch64", "-L", "/usr/aarch64-linux-gnu/"]

        if config.general.qemu == QemuKind.ARM32:
            return ["qemu-arm", "-L", "/usr/arm-linux-gnueabihf"]

        return []

    @staticmethod
    def _get_error_message(error: str) -> str:
        lines = [line.strip() for line in error.split('\n') if len(line.strip()) > 0]
        if len(lines) > 0:
            error_message = lines[0]
            if error_message.startswith("["):
                last_bracket = error_message.find("]")
                error_message = error_message[last_bracket + 1:].strip()
            return error_message
        return ""

    @cached_property
    def work_dir(self) -> WorkDir:
        return WorkDir(self.config.general, self.default_work_dir_root)

    def generate_quick_stdlib(self, stdlib_abc: str) -> str:
        Log.all(_LOGGER, "Generate quick stdlib")
        cmd = self.cmd_prefix + [self.binaries.ark_quick]
        cmd.extend(self.quick_args)
        src_abc = stdlib_abc
        root, ext = path.splitext(src_abc)
        dst_abc = f'{root}.quick{ext}'
        cmd.extend([src_abc, dst_abc])

        Log.all(_LOGGER, f"quick stdlib: {' '.join(cmd)}")

        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=self.cmd_env) as process:
            try:
                process.communicate(timeout=600)
            except subprocess.TimeoutExpired:
                process.kill()
                Log.exception_and_raise(_LOGGER, f"Cannot quick {src_abc}: timeout")

        if process.returncode != 0:
            Log.exception_and_raise(_LOGGER, f"Cannot quick {src_abc}: {process.returncode}")

        return dst_abc

    def create_coverage_html(self) -> None:
        Log.all(_LOGGER, "Create html report for coverage")
        self.coverage.make_profdata_list_file()
        self.coverage.merge_all_profdata_files()
        self.coverage.llvm_cov_export_to_info_file()
        self.coverage.genhtml()

    def summarize(self) -> int:
        Log.all(_LOGGER, "Processing run statistics")

        fail_lists: Dict[FailKind, List[Test]] = {kind: [] for kind in FailKind}
        ignored_still_failed: List[Test] = []
        ignored_but_passed: List[Test] = []
        excluded_but_passed: List[Test] = []
        excluded_still_failed: List[Test] = []

        self.failed = 0
        self.ignored = 0
        self.passed = 0
        self.excluded_after = 0
        self.excluded = 0 if self.update_excluded else self.excluded

        results = [result for result in self.results if result.passed is not None]

        for test_result in results:
            if not isinstance(test_result, TestFileBased):
                TestCase().assertTrue(isinstance(test_result, TestFileBased))
                return self.failed
            if test_result.excluded:
                self.excluded_after += 1
                continue

            if not test_result.passed:
                self._process_failed(test_result, ignored_still_failed, excluded_still_failed, fail_lists)
            else:
                self.passed += 1
                if test_result.ignored:
                    ignored_but_passed.append(test_result)
                if test_result.path in self.excluded_tests:
                    excluded_but_passed.append(test_result)

        summary = Summary(
            self.name, len(results) + self.excluded, self.passed, self.failed,
            self.ignored, len(ignored_but_passed), self.excluded, self.excluded_after
        )

        if results:
            xml_view = XmlView(self.work_dir.report, summary)
            execution_time = round((datetime.now() - self.start_time).total_seconds(), 3)
            xml_view.create_xml_report(results, execution_time)
            xml_view.create_ignore_list(set(results))
            self.__generate_detailed_report(results)
            self.__generate_spec_report(results)

        if ReportFormat.HTML in self.test_env.report_formats:
            html_view = HtmlView(self.work_dir.report, self.config, summary)
            html_view.create_html_index(fail_lists, self.test_env.timestamp)

        summary.passed = summary.passed - summary.ignored_but_passed

        standard_view = StandardView(self.work_dir.report, self.update_excluded, self.excluded_lists, summary)
        standard_view.summarize_failures(fail_lists)
        standard_view.create_failures_report(fail_lists)
        standard_view.create_update_excluded(excluded_but_passed, excluded_still_failed)
        standard_view.summarize_passed_ignored(ignored_but_passed)
        standard_view.display_summary(fail_lists)
        standard_view.create_time_report(results, self.config.time_report)

        return self.failed

    def collect_excluded_test_lists(self, extra_list: Optional[List[str]] = None,
                                    test_name: Optional[str] = None) -> None:
        self.excluded_lists.extend(self.collect_test_lists("excluded", extra_list, test_name))

    def collect_ignored_test_lists(self, extra_list: Optional[List[str]] = None,
                                   test_name: Optional[str] = None) -> None:
        self.ignored_lists.extend(self.collect_test_lists("ignored", extra_list, test_name))

    def collect_test_lists(
            self,
            kind: str, extra_lists: Optional[List[str]] = None,
            test_name: Optional[str] = None
    ) -> List[str]:
        test_lists = extra_lists[:] if extra_lists else []
        test_name = test_name if test_name else self.name

        short_template_name = f"{test_name}*-{kind}*.txt"
        conf_kind = self.conf_kind.value \
            if self.conf_kind != ConfigurationKind.OTHER_INT \
            else self.config.ark.interpreter_type
        full_template_name = f"{test_name}.*-{kind}" + \
                             f"(-{self.operating_system.value})?" \
                             f"(-{self.architecture.value})?" \
                             f"(-{conf_kind.upper()})?"
        if self.sanitizer != SanitizerKind.NONE:
            full_template_name += f"(-{self.sanitizer.value})?"
        full_template_name += f"(-OL{self.config.es2panda.opt_level})?"
        if self.config.es2panda.debug_info:
            full_template_name += "(-DI)?"
        if self.conf_kind == ConfigurationKind.JIT and self.config.ark.jit.num_repeats > 1:
            full_template_name += "(-(repeats|REPEATS))?"
        full_template_name += f"(-{self.build_type.value})?"
        full_template_name += ".txt"
        full_pattern = re.compile(full_template_name)

        def is_matched(file: str) -> bool:
            file = file.split(path.sep)[-1]
            match = full_pattern.match(file)
            return match is not None

        glob_expression = path.join(self.list_root, f"**/{short_template_name}")
        test_lists.extend(filter(
            is_matched,
            glob(glob_expression, recursive=True)
        ))

        Log.all(_LOGGER, f"Loading {kind} test lists: {test_lists}")

        return test_lists

    def _process_failed(self, test_result: TestFileBased, ignored_still_failed: List[Test],
                        excluded_still_failed: List[Test], fail_lists: Dict[FailKind, List[Test]]) -> None:
        if test_result.ignored:
            self.ignored += 1
            ignored_still_failed.append(test_result)
        elif test_result.path in self.excluded_tests:
            excluded_still_failed.append(test_result)
            self.excluded += 1
        else:
            self.failed += 1
            if test_result.fail_kind:
                fail_lists[test_result.fail_kind].append(test_result)
            else:
                TestCase().assertTrue(test_result.fail_kind)

    def __generate_detailed_report(self, results: List[Test]) -> None:
        if self.config.report.detailed_report:
            detailed_report = DetailedReport(
                results,
                self.name,
                self.work_dir.report,
                self.config.report.detailed_report_file)
            detailed_report.populate_report()

    def __generate_spec_report(self, results: List[Test]) -> None:
        if self.config.report.spec_report:
            spec_report = SpecReport(
                results,
                self.name,
                self.work_dir.report,
                self.config.report.spec_report_file,
                self.config.report.spec_report_yaml,
                self.config.report.spec_file)
            spec_report.populate_report()

    def __set_test_list_options(self) -> None:
        for san in ["ASAN_OPTIONS", "TSAN_OPTIONS", "MSAN_OPTIONS", "LSAN_OPTIONS"]:
            # we don't want to interpret asan failures as SyntaxErrors
            self.cmd_env[san] = ":exitcode=255"

        self.sanitizer = self.config.test_lists.sanitizer
        self.architecture = self.config.test_lists.architecture
        self.operating_system = self.config.test_lists.operating_system
        self.build_type = self.config.test_lists.build_type

    def __fill_up_runtime_args(self) -> List[str]:
        runtime_args: List[str] = self.config.ark.ark_args[:]
        runtime_args.extend([
            f'--gc-type={self.config.general.gc_type}',
            f'--heap-verifier={self.config.general.heap_verifier}',
            f'--full-gc-bombing-frequency={self.config.general.full_gc_bombing_frequency}',
        ])

        if self.config.general.run_gc_in_place:
            runtime_args += ['--run-gc-in-place']

        return runtime_args

    def __fill_up_aot_args(self) -> Tuple[Optional[str], List[str]]:
        ark_aot = None
        aot_args = []
        if self.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            ark_aot = self.binaries.ark_aot

            aot_args = [
                f'--gc-type={self.config.general.gc_type}',
                f'--heap-verifier={self.config.general.heap_verifier}',
                f'--full-gc-bombing-frequency={self.config.general.full_gc_bombing_frequency}',
            ]

            if self.config.general.run_gc_in_place:
                aot_args += ['--run-gc-in-place']

            aot_args += self.config.ark_aot.aot_args

        return ark_aot, aot_args
