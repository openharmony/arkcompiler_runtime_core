#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

from __future__ import annotations

from pathlib import Path
from typing import List, Sequence

from runner.enum_types.params import TestEnv
from runner.plugins.ets.test_ets import TestETS
from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata


class TestEtsCts(TestETS):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestETS.__init__(self, test_env, test_path, flags, test_id)
        self.metadata: TestMetadata = get_metadata(Path(test_path))
        if self.metadata.package is not None:
            self.main_entry_point = f"{self.metadata.package}.{self.main_entry_point}"

    @property
    def is_negative_runtime(self) -> bool:
        return self.metadata.tags.negative and not self.metadata.tags.compile_only

    @property
    def is_negative_compile(self) -> bool:
        return self.metadata.tags.negative and self.metadata.tags.compile_only

    @property
    def is_compile_only(self) -> bool:
        return self.metadata.tags.compile_only

    @property
    def is_valid_test(self) -> bool:
        return not self.metadata.tags.not_a_test

    def ark_extra_options(self) -> List[str]:
        return self.metadata.ark_options

    @property
    def ark_timeout(self) -> int:
        return self.metadata.timeout if self.metadata.timeout else super().ark_timeout

    @property
    def dependent_files(self) -> Sequence[TestETS]:
        if not self.metadata.files:
            return []

        tests = []
        for file in self.metadata.files:
            path = Path(self.path).parent / Path(file)
            test_id = Path(self.test_id).parent / Path(file)
            tests.append(self.__class__(self.test_env, str(path), self.flags, str(test_id)))
        return tests

    @property
    def runtime_args(self) -> List[str]:
        if not self.dependent_files:
            return super().runtime_args
        return self.add_boot_panda_files(super().runtime_args)

    @property
    def verifier_args(self) -> List[str]:
        if not self.dependent_files:
            return super().verifier_args
        return self.add_boot_panda_files(super().verifier_args)

    def add_boot_panda_files(self, args: List[str]) -> List[str]:
        dep_files_args = []
        for arg in args:
            if arg.startswith("--boot-panda-files"):
                name, value = arg.split('=')
                dep_files_args.append(f'{name}={":".join([value] + [dt.test_abc for dt in self.dependent_files])}')
            else:
                dep_files_args.append(arg)
        return dep_files_args
