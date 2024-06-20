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
#

import logging
import subprocess
from copy import deepcopy
from os import path, remove
from typing import List, Callable, Tuple, Optional

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, Params, TestReport
from runner.logger import Log
from runner.options.options_jit import JitOptions
from runner.test_base import Test

_LOGGER = logging.getLogger("runner.test_file_based")

ResultValidator = Callable[[str, str, int], bool]


class TestFileBased(Test):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        Test.__init__(self, test_env, test_path, flags, test_id)
        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind: Optional[FailKind] = None
        self.main_entry_point = "_GLOBAL::func_main_0"

    def ark_extra_options(self) -> List[str]:
        return []

    @property
    def ark_timeout(self) -> int:
        return int(self.test_env.config.ark.timeout)

    @property
    def runtime_args(self) -> List[str]:
        return self.test_env.runtime_args

    @property
    def verifier_args(self) -> List[str]:
        return self.test_env.verifier_args

    # pylint: disable=too-many-locals
    def run_one_step(self, name: str, params: Params, result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        if self.test_env.config.general.coverage.use_llvm_cov:
            params = deepcopy(params)
            profraw_file, profdata_file = self.test_env.coverage.get_uniq_profraw_profdata_file_paths()
            params.env['LLVM_PROFILE_FILE'] = profraw_file

        cmd = self.test_env.cmd_prefix + [params.executor]
        cmd.extend(params.flags)

        self.log_cmd(cmd)
        Log.all(_LOGGER, f"Run {name}: {' '.join(cmd)}")

        passed = False
        output = ""

        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=params.env,
                encoding='utf-8',
                errors='ignore',
        ) as process:
            try:
                output, error = process.communicate(timeout=params.timeout)
                return_code = process.returncode
                passed = result_validator(output, error, return_code)
                fail_kind = params.fail_kind_fail if not passed else None
            except subprocess.TimeoutExpired:
                self.log_cmd(f"Failed by timeout after {params.timeout} sec")
                fail_kind = params.fail_kind_timeout
                error = fail_kind.name
                return_code = process.returncode
                process.kill()
            except Exception as ex:  # pylint: disable=broad-except
                self.log_cmd(f"Failed with {ex}")
                fail_kind = params.fail_kind_other
                error = fail_kind.name
                return_code = -1

        if self.test_env.config.general.coverage.use_llvm_cov:
            self.test_env.coverage.merge_and_delete_prowraw_files(profraw_file, profdata_file)

        report = TestReport(
            output=output,
            error=error,
            return_code=return_code
        )

        return passed, report, fail_kind

    def run_es2panda(self, flags: List[str], test_abc: str, result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = flags[:]
        es2panda_flags.append("--thread=0")
        if len(test_abc) > 0:
            es2panda_flags.append(f"--output={test_abc}")

        es2panda_flags.append(self.path)

        params = Params(
            executor=self.test_env.es2panda,
            flags=es2panda_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.es2panda.timeout,
            fail_kind_fail=FailKind.ES2PANDA_FAIL,
            fail_kind_timeout=FailKind.ES2PANDA_TIMEOUT,
            fail_kind_other=FailKind.ES2PANDA_OTHER,
        )

        return self.run_one_step("es2panda", params, result_validator)

    def run_runtime(self, test_an: str, test_abc: str, result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        ark_flags = []
        ark_flags.extend(self.ark_extra_options())
        ark_flags.extend(self.runtime_args)
        if self.test_env.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            ark_flags.extend(["--aot-files", test_an])

        if self.test_env.conf_kind == ConfigurationKind.JIT:
            ark_flags.extend([
                '--compiler-enable-jit=true',
                '--compiler-check-final=true'])
            jit_options: JitOptions = self.test_env.config.ark.jit
            if jit_options.compiler_threshold is not None:
                ark_flags.append(
                    f'--compiler-hotness-threshold={jit_options.compiler_threshold}'
                )
        else:
            ark_flags.extend(['--compiler-enable-jit=false'])

        if self.test_env.config.ark.interpreter_type is not None:
            ark_flags.extend([f'--interpreter-type={self.test_env.config.ark.interpreter_type}'])

        ark_flags.extend([test_abc, self.main_entry_point])

        params = Params(
            timeout=self.ark_timeout,
            executor=self.test_env.runtime,
            flags=ark_flags,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.RUNTIME_FAIL,
            fail_kind_timeout=FailKind.RUNTIME_TIMEOUT,
            fail_kind_other=FailKind.RUNTIME_OTHER,
        )

        return self.run_one_step("ark", params, result_validator)

    def run_aot(self, test_an: str, test_abcs: List[str], result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        aot_flags = []
        aot_flags.extend(self.test_env.aot_args)
        aot_flags = [flag.strip("'\"") for flag in aot_flags]
        for test_abc in test_abcs:
            aot_flags.extend(['--paoc-panda-files', test_abc])
        aot_flags.extend(['--paoc-output', test_an])

        if path.isfile(test_an):
            remove(test_an)

        assert self.test_env.ark_aot is not None
        params = Params(
            timeout=self.test_env.config.ark_aot.timeout,
            executor=self.test_env.ark_aot,
            flags=aot_flags,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.AOT_FAIL,
            fail_kind_timeout=FailKind.AOT_TIMEOUT,
            fail_kind_other=FailKind.AOT_OTHER,
        )

        return self.run_one_step("ark_aot", params, result_validator)

    def run_ark_quick(self, flags: List[str], test_abc: str, result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind], str]:
        quick_flags = flags[:]
        quick_flags.extend(self.test_env.quick_args)

        src_abc = test_abc
        root, ext = path.splitext(src_abc)
        dst_abc = f'{root}.quick{ext}'
        quick_flags.extend([src_abc, dst_abc])

        params = Params(
            timeout=self.ark_timeout,
            executor=self.test_env.ark_quick,
            flags=quick_flags,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.QUICK_FAIL,
            fail_kind_timeout=FailKind.QUICK_TIMEOUT,
            fail_kind_other=FailKind.QUICK_OTHER,
        )

        return *(self.run_one_step("ark_quick", params, result_validator)), dst_abc
