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
            "llvm-cov-profdata-out-path": self.llvm_profdata_out_path,
            "llvm-cov-html-out-path": self.llvm_cov_html_out_path,
            "llvm-cov-report-by-components": self.llvm_cov_report_by_components,
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
        yaml_path="general.coverage.llvm-cov-profdata-out-path",
        cli_name="llvm_profdata_out_path",
        cast_to_type=_to_dir
    )
    def llvm_profdata_out_path(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="general.coverage.llvm-cov-html-out-path",
        cli_name="llvm_cov_html_out_path",
        cast_to_type=_to_dir
    )
    def llvm_cov_html_out_path(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="general.coverage.llvm-cov-report-by-components",
        cli_name="llvm_cov_report_by_components",
        cast_to_type=_to_bool
    )
    def llvm_cov_report_by_components(self) -> bool:
        return False

    def get_command_line(self) -> str:
        options = [
            '--use-llvm-cov' if self.use_llvm_cov else '',
            f'--llvm-profdata-out-path="{self.llvm_profdata_out_path}"'
            if self.llvm_profdata_out_path is not None else '',
            f'--llvm-cov-html-out-path="{self.llvm_cov_html_out_path}"'
            if self.llvm_cov_html_out_path is not None else '',
            '--llvm-cov-report-by-components' if self.llvm_cov_report_by_components else '',
        ]
        return ' '.join(options)
