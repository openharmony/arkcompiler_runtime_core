#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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
import os
import re
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import cast

from jinja2 import Environment, FileSystemLoader

from runner.common_exceptions import (
    CompileEtsTestPartException,
    CompileEtsTestPartTimeoutException,
    InvalidConfiguration,
)
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.options.options_step import Step, StepKind
from runner.runner_file_based import RunnerUtils
from runner.suites.test_metadata import TestMetadata
from runner.utils import ani_name_rule, make_gtest_name

_LOGGER = Log.get_logger(__file__)


@dataclass
class J2TemplateParams:
    template_path: Path
    params: list[dict[str, dict[str, str]]]


class GTestEtsCompiler(IGenerator):
    ARKTSCONFIG = "arktsconfig"

    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self.config = config
        self.stdlib_path = config.general.static_core_root / 'plugins' / 'ets' / 'stdlib'
        self.steps = config.workflow.steps
        self.extension = config.workflow.get_parameter("extension")
        self.processes = self.config.general.processes
        self.test_prefix = cast(str, config.test_suite.get_parameter("gtest-zip-prefix"))

    @property
    def root(self) -> Path:
        work_dir = os.getenv("WORK_DIR")
        if work_dir is not None:
            return Path(work_dir) / self.config.test_suite.suite_name
        return self._target.parent

    @property
    def gen(self) -> Path:
        return self.root / "gen"

    @property
    def intermediate(self) -> Path:
        return self.root / "intermediate"

    @staticmethod
    def make_dict_from_param_str(param_str: str) -> dict[str, str]:
        result: dict[str, str] = {}
        params = param_str.split(',')
        prev_key = None
        for item in params:
            if not item.strip():
                continue

            key, sep, val = item.partition('=')
            if not sep:
                if prev_key is not None:
                    result[prev_key] += ',' + item
                else:
                    raise InvalidConfiguration("Invalid val in template parameters set")
            else:
                result[key.strip()] = val.strip()
                prev_key = key.strip()
        return result

    @staticmethod
    def collect_params_j2_files(param_files: set[Path]) -> list[J2TemplateParams]:
        result = []
        for param_file in param_files:
            result.extend(GTestEtsCompiler._get_params_from_cmake(param_file))
        return result

    @staticmethod
    def _get_params_from_cmake(path_to_file: Path) -> list[J2TemplateParams]:
        cmake_current_source_dir = '${CMAKE_CURRENT_SOURCE_DIR}'
        content = Path(path_to_file).read_text(encoding="utf-8")

        pattern = r'J2_ETS_SOURCE\s+(\S+\.ets\.j2)\s*PARAMS\s+((?:"[^"]*"\s*)+)'

        results: list[J2TemplateParams] = []

        for match in re.finditer(pattern, content, re.DOTALL):
            ets_file_raw = match.group(1)
            params_block = match.group(2)

            template_path = Path(ets_file_raw.replace(cmake_current_source_dir, str(path_to_file.parent)))

            params = GTestEtsCompiler._parse_params_block(params_block)

            results.append(J2TemplateParams(template_path=template_path, params=params))

        return results

    @staticmethod
    def _parse_params_block(params_block: str) -> list[dict]:
        result = []

        for line in params_block.strip().split('\n'):
            line = line.strip().strip('"')
            if not line:
                continue

            key, val = line.split(':', 1)
            result.append({key.strip(): GTestEtsCompiler.make_dict_from_param_str(val.strip())
                           })
        return result

    def set_test_id(self, step_args: list[str], test_path: Path) -> list[str]:
        flags: list[str] = []
        test_metadata = TestMetadata.get_metadata(Path(test_path))

        if test_metadata.file_name and test_metadata.file_name not in test_path.name:
            output_path = (self.intermediate / test_metadata.file_name).with_suffix(".abc")
        else:
            output_path = (self.intermediate / test_path.name).with_suffix(".abc")
        self.intermediate.mkdir(parents=True, exist_ok=True)

        custom_arktsconfig = test_metadata.arktsconfig

        for arg in step_args:
            if "--arktsconfig=" in arg and custom_arktsconfig:
                flag_name, _ = arg.split("=", 1)
                flags.append(f"{flag_name}={custom_arktsconfig}")

            elif "--output=" in arg:
                flag_name, _ = arg.split("=", 1)
                flags.append(f"{flag_name}={output_path}")

            elif "${test-id}" in arg:
                flags.append(str(test_path))
            else:
                flags.append(arg)

        if custom_arktsconfig and not any("--arktsconfig=" in f for f in flags):
            flags.insert(0, f"--arktsconfig={custom_arktsconfig}")

        if custom_arktsconfig:
            flags.insert(0, f"--stdlib={self.stdlib_path}")

        return flags

    def generate(self) -> list[str]:
        step = self._get_step_compiler_params()

        ets_test_sources = self._get_ets_sources()

        ets_j2_sources = self._get_ets_j2_sources()
        params_files: set[Path] = set()
        for ets_j2_file in ets_j2_sources:
            if (ets_j2_file.parent / "CMakeLists.txt").exists():
                params_files.add(ets_j2_file.parent / "CMakeLists.txt")

        j2_test_params = GTestEtsCompiler.collect_params_j2_files(params_files)
        self._generate_ets_files(j2_test_params)

        configs_j2_tests = self._collect_arktsconfigs(j2_test_params)
        self._copy_arktsconfigs(configs_j2_tests)

        tmpl_tests_to_compile = list(self.gen.rglob(f"**/*.{self.extension}"))

        max_workers = self.processes

        with ThreadPoolExecutor(max_workers=max_workers) as ex:
            futures = {ex.submit(self.compile_file, step, test): test
                       for test in ets_test_sources + tmpl_tests_to_compile}

            for fut in as_completed(futures):
                fut.result()

        return [str(file_path) for file_path in self.intermediate.rglob("**/*")]

    def compile_file(self, step: Step, test_id: Path) -> None:
        step_args = self.set_test_id(step.args, test_id)

        cmd = [str(step.executable_path)]
        if not step.skip_qemu:
            qemu_prefix = RunnerUtils.set_cmd_prefix(self.config)
            cmd = [*qemu_prefix, *cmd]

        cmd.extend(step_args)

        env = {**os.environ}

        try:
            cp = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="ignore",
                env=env,
                timeout=step.timeout,
                check=False
            )
        except subprocess.TimeoutExpired as e:
            raise CompileEtsTestPartTimeoutException(
                f"{step.name}: timeout after {step.timeout}s\n"
                f"cmd params: {' '.join(cmd)}"
            ) from e

        if cp.returncode != 0:
            raise CompileEtsTestPartException(
                f"{step.name}: compilation failed for {test_id.name} with return code {cp.returncode}\n"
                f"cmd params: {' '.join(cmd)}\n"
                f"STDERR:\n{cp.stderr}\n"
                f"STDOUT:\n{cp.stdout}"
            )

    def _get_step_compiler_params(self) -> Step:
        for step in self.steps:
            if step.step_kind == StepKind.COMPILER:
                return step

        raise InvalidConfiguration(f"Compiler step is not set in workflow {self.config.workflow.name} but required"
                                   f"for the generator class of the test suite {self.config.test_suite.suite_name}")

    def _get_ets_sources(self) -> list[Path]:
        return list(self._source.resolve().rglob(f"*.{self.extension}"))

    def _get_ets_j2_sources(self) -> list[Path]:
        root = self._source.resolve()
        return [root / ets_files for ets_files in root.rglob(f"*.{self.extension}.j2") if ets_files.is_file()]

    def _generate_ets_files(self, templates: list[J2TemplateParams]) -> None:
        for template in templates:
            for param_set in template.params:
                for test_prefix, tmpl_params in param_set.items():
                    self._generate_ets_file(template.template_path, test_prefix, tmpl_params)

    def _collect_arktsconfigs(self, templates: list[J2TemplateParams]) -> set[Path]:
        template_folders = [template.template_path.parent for template in templates]
        configs: set[Path] = set()

        for tmpl_folder in template_folders:
            for path in tmpl_folder.rglob("arktsconfig.json"):
                configs.add(path)

        return configs

    def _copy_arktsconfigs(self, paths_to_config: set[Path]) -> None:
        for src in paths_to_config:
            test_folder = src.parent.relative_to(self._source)
            src_dest = self.gen / test_folder
            src_dest.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, src_dest / src.name)

    def _generate_ets_file(self, file_tmpl: Path, test_prefix: str, j2_params: dict) -> None:

        output_path = self._build_output_path(file_tmpl, test_prefix)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        template_dir = file_tmpl.parent
        template_name = file_tmpl.name

        try:
            env = Environment(
                loader=FileSystemLoader(template_dir),
                trim_blocks=True,
                lstrip_blocks=True
            )
            template = env.get_template(template_name)

            generated_code = template.render(**j2_params)
            output_path.write_text(generated_code)
        except Exception as e:
            raise CompileEtsTestPartException(
                f"Failed to generate ETS file {file_tmpl} with params {j2_params}: {e!s}") from e

    def _build_output_path(self, template_path: Path, tmpl_prefix: str) -> Path:
        tmpl_metadata = TestMetadata.get_metadata(template_path)
        if tmpl_metadata.file_name:
            base_name = tmpl_metadata.file_name
        else:
            base_name = (
            template_path.name
            .replace(''.join(template_path.suffixes), '')
            .replace('_test', '')
        )

        naming_rule = ani_name_rule if self.test_prefix == "ani_test" else None
        output_filename = make_gtest_name(Path(base_name), self.test_prefix, rule=naming_rule)

        output_dir = self.gen / template_path.parent.relative_to(self._source)

        return output_dir / f"{output_filename}_{tmpl_prefix}.ets"
