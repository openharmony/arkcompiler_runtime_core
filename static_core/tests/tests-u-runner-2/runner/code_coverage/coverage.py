#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import os
import subprocess
import sys
import uuid
from multiprocessing import current_process
from os import getenv, path, remove
from pathlib import Path
from typing import Any, TextIO

from runner.logger import Log
from runner.suites.work_dir import WorkDir

_LOGGER = Log.get_logger(__file__)

LLVM_PROFDATA_VERSION: str | None = getenv('LLVM_PROFDATA_VERSION')
LLVM_COV_VERSION: str | None = getenv('LLVM_COV_VERSION')

IGNORE_REGEX: str | None = getenv('IGNORE_REGEX')

LLVM_PROFDATA_BINARY = path.join('llvm-profdata-14')
LLVM_COV_PATH_BINARY = path.join('llvm-cov-14')

GENHTML_BINARY = 'genhtml'

if LLVM_PROFDATA_VERSION is not None:
    LLVM_PROFDATA_BINARY = f"{LLVM_PROFDATA_BINARY}-{LLVM_PROFDATA_VERSION}"

if LLVM_COV_VERSION is not None:
    LLVM_COV_PATH_BINARY = f"{LLVM_COV_PATH_BINARY}-{LLVM_COV_VERSION}"


class LinuxCommands:
    @staticmethod
    def _popen(**kwargs: Any) -> Any:   # type: ignore[explicit-any]
        if sys.version_info.major == 3 and sys.version_info.minor >= 6:
            return subprocess.Popen(encoding=sys.stdout.encoding, **kwargs)
        return subprocess.Popen(**kwargs)

    def run_command(self, command: list[str], stdout: TextIO | int = subprocess.PIPE) -> TextIO | int:
        with self._popen(
                args=command,
                stdout=stdout,
                stderr=subprocess.PIPE,
        ) as proc:
            try:
                stdout = proc.communicate(timeout=5400)
            except subprocess.TimeoutExpired:
                _LOGGER.all(f"Timeout when try execute {command}")
        return stdout

    def do_genhtml(self, args: list[str]) -> TextIO | int:
        command = ['genhtml', *args]
        return self.run_command(command)


class LlvmCovCommands:
    @staticmethod
    def llvm_prof_merge_command(args: list[str]) -> list[str]:
        return [LLVM_PROFDATA_BINARY, 'merge', *args]

    @staticmethod
    def llvm_cov_export_command(args: list[str]) -> list[str]:
        return [LLVM_COV_PATH_BINARY, 'export', *args]


class LlvmCov:
    def __init__(self, build_dir: Path, work_dir: WorkDir) -> None:
        self.build_dir: Path = build_dir
        self.coverage_dir = work_dir.coverage_dir
        self.llvm_cov_commands = LlvmCovCommands()
        self.linux_commands = LinuxCommands()

    @staticmethod
    def do_find(search_directory: Path, extension: str) -> list[Path]:
        return list(Path(search_directory).rglob(extension))

    def get_uniq_profraw_profdata_file_paths(self) -> list[str]:
        pid = current_process().pid
        hash_code = uuid.uuid4()
        file_path = f"{self.coverage_dir.profdata_dir}/{pid}-{hash_code}"
        profraw_file = os.extsep.join([file_path, 'profraw'])
        profdata_file = os.extsep.join([file_path, 'profdata'])
        return [profraw_file, profdata_file]

    def llvm_profdata_merge(self, args: list[str]) -> None:
        prof_merge_command = self.llvm_cov_commands.llvm_prof_merge_command(args)
        self.linux_commands.run_command(prof_merge_command)

    def merge_and_delete_prowraw_files(self, profraw_file: str, profdata_file: str) -> None:
        file = Path(profraw_file)
        if file.is_file():
            self.llvm_profdata_merge(['--sparse', profraw_file, '-o', profdata_file])
            remove(profraw_file)
        else:
            _LOGGER.all(f"File with name {profraw_file} not exist")

    def make_profdata_list_file(self) -> None:
        results = self.do_find(self.coverage_dir.profdata_dir, '*.profdata')
        with os.fdopen(os.open(self.coverage_dir.profdata_files_list_file, os.O_RDWR | os.O_APPEND | os.O_CREAT, 0o755),
                       "a", encoding="utf-8") as the_file:
            for i in results:
                the_file.write(str(i) + '\n')

    def merge_all_profdata_files(self) -> None:
        input_files = f"--input-files={self.coverage_dir.profdata_files_list_file}"
        self.llvm_profdata_merge(['--sparse', input_files, '-o', str(self.coverage_dir.profdata_merged_file)])

    def llvm_cov_export_to_info_file(self) -> None:
        instr_profile = f"-instr-profile={self.coverage_dir.profdata_merged_file}"

        bin_dir = f"{self.build_dir}/bin"

        bins = [
            'ark', 'ark_aot', 'ark_asm', 'ark_disasm', 'c2abc',
            'c2p', 'es2panda', 'irtoc_ecmascript_fastpath_exec',
            'irtoc_fastpath_exec', 'irtoc_interpreter_exec',
            'panda', 'pandasm', 'paoc', 'verifier'
        ]

        args = ['-format=lcov', '-Xdemangler', 'c++filt', instr_profile]

        for bin_n in bins:
            args.append(f"--object={bin_dir}/{bin_n}")

        for so_path in Path(self.build_dir).rglob('*.so'):
            args.append(f"--object={so_path}")

        if IGNORE_REGEX is not None:
            ignore_filename_regex = f"--ignore-filename-regex=\"{IGNORE_REGEX}\""
            args.append(ignore_filename_regex)

        command_info = self.llvm_cov_commands.llvm_cov_export_command(args)
        args = list(map(lambda x: x.replace('-format=lcov', '-format=text'), args))
        args.append("-summary-only")
        command_json = self.llvm_cov_commands.llvm_cov_export_command(args)

        with os.fdopen(os.open(self.coverage_dir.info_file, os.O_WRONLY | os.O_CREAT, 0o755),
                       "w", encoding="utf-8") as file_dot_info:
            self.linux_commands.run_command(command_info, stdout=file_dot_info)

        with os.fdopen(os.open(self.coverage_dir.json_file, os.O_WRONLY | os.O_CREAT, 0o755),
                       "w", encoding="utf-8") as file_dot_json:
            self.linux_commands.run_command(command_json, stdout=file_dot_json)

    def genhtml(self) -> None:
        output_directory = f"--output-directory={self.coverage_dir.html_report_dir}"
        self.linux_commands.do_genhtml([output_directory, str(self.coverage_dir.info_file)])
