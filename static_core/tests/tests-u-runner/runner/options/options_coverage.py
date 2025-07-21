#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_str, _to_dir


class CoverageOptions:
    def __str__(self) -> str:
        return _to_str(self, 2)

    def to_dict(self) -> Dict[str, object]:
        return {
            "use-llvm-cov": self.use_llvm_cov,
            "use-lcov": self.use_lcov,
            "profdata-files-dir": self.profdata_files_dir,
            "coverage-html-report-dir": self.coverage_html_report_dir,
            "coverage-per-binary": self.coverage_per_binary,
            "llvm-cov-exclude": self.llvm_cov_exclude,
            "lcov-exclude": self.lcov_exclude,
        }

    @cached_property
    @value(
        yaml_path="general.coverage.use-llvm-cov",
        cli_name="use_llvm_cov",
        cast_to_type=_to_bool
    )
    def use_llvm_cov(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="general.coverage.use-lcov",
        cli_name="use_lcov",
        cast_to_type=_to_bool
    )
    def use_lcov(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="general.coverage.profdata-files-dir",
        cli_name="profdata_files_dir",
        cast_to_type=_to_dir
    )
    def profdata_files_dir(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="general.coverage.coverage-html-report-dir",
        cli_name="coverage_html_report_dir",
        cast_to_type=_to_dir
    )
    def coverage_html_report_dir(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="general.coverage.coverage-per-binary",
        cli_name="coverage_per_binary",
        cast_to_type=_to_bool
    )
    def coverage_per_binary(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="general.coverage.llvm-cov-exclude",
        cli_name="llvm_cov_exclude",
    )
    def llvm_cov_exclude(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="general.coverage.lcov-exclude",
        cli_name="lcov_exclude",
    )
    def lcov_exclude(self) -> Optional[str]:
        return None

    def get_command_line(self) -> str:
        options = [
            '--use-llvm-cov' if self.use_llvm_cov else '',

            '--use-lcov' if self.use_lcov else '',

            '--coverage-per-binary' if self.coverage_per_binary else '',

            f'--profdata-files-dir="{self.profdata_files_dir}"'
            if self.profdata_files_dir is not None else '',

            f'--coverage-html-report-dir="{self.coverage_html_report_dir}"'
            if self.coverage_html_report_dir is not None else '',

            f'--llvm-cov-exclude="{self.llvm_cov_exclude}"'
            if self.llvm_cov_exclude is not None else '',

            f'--lcov-exclude="{self.lcov_exclude}"'
            if self.lcov_exclude is not None else '',
        ]
        return ' '.join(options)
