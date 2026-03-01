#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

from __future__ import annotations

from os import fdopen as os_fdopen, makedirs, O_CREAT, O_WRONLY, open as os_open, path
from pathlib import Path
from typing import List, Optional, Tuple

from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import Params, TestEnv, TestReport
from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata
from runner.test_file_based import TestFileBased

def create_tmp_interop_import_file(interop_path: str) -> None:
    if path.exists(interop_path):
        return
    makedirs(path.dirname(interop_path), exist_ok=True)
    interop_file = """\
    declare namespace st {
        export class Array<T> {}
        export class Map<K, V> {}
        export class Set<T> {}
    }
    export default st;
    """
    with open(interop_path, "w", encoding="utf-8") as f:
        f.write(interop_file)

class TestDeclgenETS2TS(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 build_dir: str, suite_name: str) -> None:
        super().__init__(test_env, test_path, flags, test_id)
        self.declgen_ets2ts_executor = path.join(
            build_dir, "bin", "declgen_ets2ts")
        self.tsc_executor = path.join(
            self.test_env.config.general.static_core_root, "third_party", "typescript", "bin", "tsc")
        self.metadata: TestMetadata = get_metadata(Path(test_path))
        self.declgen_ets2ts_timeout = 120
        self.tsc_timeout = 120
        self.decl_path = test_env.work_dir.intermediate
        makedirs(self.decl_path, exist_ok=True)
        name_without_ext, _ = path.splitext(self.test_id)
        self.test_dets = path.join(self.decl_path, f"{name_without_ext}.d.ets")
        self.test_ets = path.join(self.decl_path, "ets", f"{name_without_ext}.ets")
        makedirs(path.dirname(self.test_dets), exist_ok=True)
        makedirs(path.dirname(self.test_ets), exist_ok=True)
        self.suite_name = suite_name

    @property
    def is_compile_only_negative(self) -> bool:
        """ True if a test is marked as negative and compile-only at the same time """
        return self.metadata.tags.negative and self.metadata.tags.compile_only

    @property
    def is_valid_test(self) -> bool:
        """ True if a test is valid """
        return not self.metadata.tags.not_a_test

    @staticmethod
    def _validate_declgen_ets2ts(return_code: int, output_path: str) -> bool:
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    @staticmethod
    def _validate_tsc(return_code: int) -> bool:
        return return_code == 0

    def run_declgen_ets2ts(self, test_dets: str, test_ets: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        declgen_flags = [
            f"--output-dets={test_dets}",
            f"--output-ets={test_ets}",
            "--export-all",
            self.path]

        params = Params(
            executor=self.declgen_ets2ts_executor,
            flags=declgen_flags,
            env=self.test_env.cmd_env,
            timeout=self.declgen_ets2ts_timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.DECLGEN_ETS2TS_FAIL,
            fail_kind_timeout=FailKind.DECLGEN_ETS2TS_TIMEOUT,
            fail_kind_other=FailKind.DECLGEN_ETS2TS_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="declgen_ets2ts",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_declgen_ets2ts(rc, test_dets)
        )
        return passed, report, fail_kind

    def run_tsc(self, test_dets: str) -> Tuple[bool, TestReport, Optional[FailKind]]:

        tsc_flags = ["--lib", "es2020"]
        cwd = path.dirname(test_dets)

        if self.suite_name == "declgen-ets2ts-sdk":
            tsconfig_file = path.join(path.dirname(test_dets), path.basename(test_dets) + "_tsconfig.json")
            with os_fdopen(os_open(tsconfig_file, O_WRONLY | O_CREAT, 0o644), "w", encoding="utf-8") as handler:
                tsconfig = f"""\
                {{
                    "compilerOptions": {{
                    "baseUrl": "{self.decl_path}",
                    "paths": {{
                        "@ohos.base": [
                            "{self.decl_path}/api/@ohos.base.d.ets"
                        ]
                    }}
                }},
                "files" : [
                    "{test_dets}"
                ]
                }}"""
                handler.write(tsconfig)
            tsc_flags.append("--project")
            tsc_flags.append(tsconfig_file)
        else:
            tmp_interop_import_path = f"{path.join(path.dirname(test_dets), path.basename(test_dets))}_includes.d.ets"
            create_tmp_interop_import_file(tmp_interop_import_path)
            tsconfig_file = path.join(path.dirname(test_dets), path.basename(test_dets) + "_tsconfig.json")
            with os_fdopen(os_open(tsconfig_file, O_WRONLY|O_CREAT, 0o644), "w", encoding="utf-8") as handler:
                tsconfig = f"""\
                {{
                    "compilerOptions": {{
                    "baseUrl": "{self.decl_path}",
                    "paths": {{
                        "@ohos.lang.interop": [
                            "{tmp_interop_import_path}"
                        ]
                    }}
                }},
                "files" : [
                    "{test_dets}"
                ]
                }}"""
                handler.write(tsconfig)
            tsc_flags.append("--project")
            tsc_flags.append(tsconfig_file)

        params = Params(
            executor=self.tsc_executor,
            flags=tsc_flags,
            env=self.test_env.cmd_env,
            timeout=self.tsc_timeout,
            cwd=cwd,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.TSC_FAIL,
            fail_kind_timeout=FailKind.TSC_TIMEOUT,
            fail_kind_other=FailKind.TSC_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="tsc",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_tsc(rc)
        )
        return passed, report, fail_kind
