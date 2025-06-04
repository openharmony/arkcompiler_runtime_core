#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import os
import subprocess
import sys
import uuid
import threading
from multiprocessing import current_process
from os import getenv, path, remove
from pathlib import Path
from typing import Optional, List, Any, TextIO, Union, Dict
from runner.code_coverage.coverage_dir import CoverageDir

from runner.logger import Log

_LOGGER = logging.getLogger("runner.code_coverage.coverage")

LLVM_PROFDATA_VERSION: Optional[str] = getenv('LLVM_PROFDATA_VERSION')
LLVM_COV_VERSION: Optional[str] = getenv('LLVM_COV_VERSION')

IGNORE_REGEX: Optional[str] = getenv('IGNORE_REGEX')

LLVM_PROFDATA_BINARY = path.join('llvm-profdata-14')
LLVM_COV_PATH_BINARY = path.join('llvm-cov-14')

GENHTML_BINARY = 'genhtml'

if LLVM_PROFDATA_VERSION is not None:
    LLVM_PROFDATA_BINARY = f"{LLVM_PROFDATA_BINARY}-{LLVM_PROFDATA_VERSION}"

if LLVM_COV_VERSION is not None:
    LLVM_COV_PATH_BINARY = f"{LLVM_COV_PATH_BINARY}-{LLVM_COV_VERSION}"


class LinuxCommands:
    @staticmethod
    def _popen(**kwargs: Any) -> Any:
        if sys.version_info.major == 3 and sys.version_info.minor >= 6:
            return subprocess.Popen(encoding=sys.stdout.encoding, **kwargs)
        return subprocess.Popen(**kwargs)

    def run_command(self, command: List[str], stdout: Union[TextIO, int] = subprocess.PIPE) -> Union[TextIO, int]:
        with self._popen(
                args=command,
                stdout=stdout,
                stderr=subprocess.PIPE,
        ) as proc:
            try:
                stdout = proc.communicate(timeout=5400)
            except subprocess.TimeoutExpired:
                Log.all(_LOGGER, f"Timeout when try execute {command}")
        return stdout

    def do_genhtml(self, args: List[str]) -> Union[TextIO, int]:
        command = ['genhtml'] + args
        return self.run_command(command)


class LlvmCovCommands:
    @staticmethod
    def get_llvm_prof_merge_command(args: List[str]) -> List[str]:
        return [LLVM_PROFDATA_BINARY, 'merge'] + args

    @staticmethod
    def get_llvm_cov_export_command(args: List[str]) -> List[str]:
        return [LLVM_COV_PATH_BINARY, 'export'] + args


class LlvmCov:
    _instance: Optional['LlvmCov'] = None
    _lock: threading.Lock = threading.Lock()
    _initialized: bool = False

    def __new__(cls, build_dir_path: Optional[Path] = None, coverage_dir: Optional[CoverageDir] = None) -> 'LlvmCov':
        if cls._instance is None:
            with cls._lock:
                if build_dir_path is None:
                    raise ValueError("build_dir_path is not initialized")
                if not build_dir_path.parts or str(build_dir_path) == ".":
                    raise ValueError("build_dir_path must not be '.' or empty")
                if not build_dir_path.exists():
                    raise FileNotFoundError(f"Dirrectory build_dir_path={build_dir_path} not found")

                if coverage_dir is None:
                    raise ValueError("coverage_dir is not initialized")
                if not coverage_dir.root.exists():
                    raise FileNotFoundError(f"Dirrectory coverage_dir.root={coverage_dir.root} not found")

                if cls._instance is None:
                    instance = super().__new__(cls)
                    instance._initialize(build_dir_path, coverage_dir)
                    cls._instance = instance
        return cls._instance

    def get_uniq_profraw_profdata_file_paths(self, component_name: Optional[str] = None) -> List[str]:
        pid = current_process().pid
        hash_code = uuid.uuid4()
        file_name = f"{pid}-{hash_code}"
        file_path = f"{self.coverage_dir.profdata_dir}/{file_name}"
        if component_name is not None:
            self.components[component_name] = Path()
            component_profdata_dir = Path(f"{self.coverage_dir.profdata_dir}/{component_name}")
            component_profdata_dir.mkdir(parents=True, exist_ok=True)
            file_path = f"{component_profdata_dir}/{component_name}-{file_name}"
        profraw_file = os.extsep.join([file_path, 'profraw'])
        profdata_file = os.extsep.join([file_path, 'profdata'])
        return [profraw_file, profdata_file]

    def llvm_profdata_merge(self, args: List[str]) -> None:
        prof_merge_command = self.llvm_cov_commands.get_llvm_prof_merge_command(args)
        self.linux_commands.run_command(prof_merge_command)

    def merge_and_delete_prowraw_files(self, profraw_file: str, profdata_file: str) -> None:
        file = Path(profraw_file)
        if file.is_file():
            self.llvm_profdata_merge(['--sparse', profraw_file, '-o', profdata_file])
            remove(profraw_file)
        else:
            Log.all(_LOGGER, f"File with name {profraw_file} not exist")

    def make_profdata_list_file(
            self,
            profdata_root_dir_path: Optional[Path] = None,
            profdata_files_list_file_path: Optional[Path] = None
        ) -> None:
        _profdata_root_dir_path = profdata_root_dir_path or self.coverage_dir.profdata_dir
        _profdata_files_list_file_path = profdata_files_list_file_path or self.coverage_dir.profdata_files_list_file

        profdata_files = list(_profdata_root_dir_path.rglob('*.profdata'))

        with os.fdopen(os.open(_profdata_files_list_file_path, os.O_RDWR | os.O_CREAT, 0o755),
                       'w', encoding="utf-8") as file:
            for entity in profdata_files:
                file.write(f"{entity}\n")

    def make_profdata_list_files_by_components(self) -> None:
        for key in self.components:
            profdata_files_list_file_path = self.get_path_from_coverage_work_dir(f"{key}_profdatalist.txt")
            component_profdata_dir = self.coverage_dir.profdata_dir / key

            self.components[key] = profdata_files_list_file_path

            self.make_profdata_list_file(component_profdata_dir, profdata_files_list_file_path)

    def merge_all_profdata_files(
            self,
            profdata_files_list_file_path: Optional[Path] = None,
            merged_profdata_file_path: Optional[Path] = None
        ) -> None:
        _merged_profdata_file_path = merged_profdata_file_path or self.coverage_dir.profdata_merged_file
        _profdata_files_list_file_path = profdata_files_list_file_path or self.coverage_dir.profdata_files_list_file

        input_files_option = f"--input-files={_profdata_files_list_file_path}"

        self.llvm_profdata_merge(['--sparse', input_files_option, '-o', str(_merged_profdata_file_path)])

    def merge_all_profdata_files_by_components(self) -> None:
        for component_name, profdata_files_list_file_path in self.components.items():
            merged_profdata_file_path = self.get_path_from_coverage_work_dir(f"{component_name}_merged.profdata")

            self.components[component_name] = merged_profdata_file_path

            self.merge_all_profdata_files(profdata_files_list_file_path, merged_profdata_file_path)

    def llvm_cov_export_to_info_file(
            self,
            merged_profdata_file_path: Optional[Path] = None,
            dot_info_file_path: Optional[Path] = None,
            component_name: Optional[str] = None
        ) -> None:
        _merged_profdata_file_path = merged_profdata_file_path or self.coverage_dir.profdata_merged_file
        _dot_info_file_path = dot_info_file_path or self.coverage_dir.info_file

        instr_profile_option = f"-instr-profile={_merged_profdata_file_path}"
        llvm_cov_export_command_args = ['-format=lcov', '-Xdemangler', 'c++filt']

        llvm_cov_export_command_args.append(instr_profile_option)

        if component_name is None:
            for bin_path in self.bin_dir.rglob('*'):
                if os.path.splitext(bin_path)[1] == '':
                    llvm_cov_export_command_args.append(f"--object={bin_path}")
        else:
            llvm_cov_export_command_args.append(f"--object={self.bin_dir}/{component_name}")

        for so_path in self.build_dir.rglob('*.so'):
            llvm_cov_export_command_args.append(f"--object={so_path}")

        if IGNORE_REGEX is not None:
            ignore_filename_regex = f"--ignore-filename-regex=\"{IGNORE_REGEX}\""
            llvm_cov_export_command_args.append(ignore_filename_regex)

        command = self.llvm_cov_commands.get_llvm_cov_export_command(llvm_cov_export_command_args)

        with open(_dot_info_file_path, 'w', encoding='utf-8') as file:
            self.linux_commands.run_command(command, stdout=file)

    def llvm_cov_export_to_info_file_by_components(self) -> None:
        for component_name, merged_profdata_file_path in self.components.items():
            dot_info_file_path = self.get_path_from_coverage_work_dir(f"{component_name}_lcov_format.info")
            self.components[component_name] = dot_info_file_path
            self.llvm_cov_export_to_info_file(merged_profdata_file_path, dot_info_file_path, component_name)

    def genhtml(self) -> None:
        if len(self.components):
            for component_name, dot_info_file in self.components.items():
                output_directory_option = f"--output-directory={self.coverage_dir.html_report_dir}/{component_name}"
                self.linux_commands.do_genhtml([output_directory_option, str(dot_info_file)])
        else:
            output_directory_option = f"--output-directory={self.coverage_dir.html_report_dir}"
            self.linux_commands.do_genhtml([output_directory_option, str(self.coverage_dir.info_file)])

    def get_path_from_coverage_work_dir(self, path_name: str) -> Path:
        return Path(f"{self.coverage_dir.coverage_work_dir}/{path_name}")

    def _initialize(self, build_dir_path: Path, coverage_dir: CoverageDir) -> None:
        if self._initialized:
            return

        self._initialized = True
        self.components: Dict[str, Path] = {}

        self.build_dir = build_dir_path
        self.bin_dir = self.build_dir / 'bin'
        self.coverage_dir = coverage_dir

        self.llvm_cov_commands = LlvmCovCommands()
        self.linux_commands = LinuxCommands()
