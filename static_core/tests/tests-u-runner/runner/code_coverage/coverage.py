#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

import subprocess
import sys
import uuid
import os
import logging

from pathlib import Path
from os import getenv, path, remove
from multiprocessing import current_process
from typing import Optional, List, Any, TextIO, Union

from runner.logger import Log
from runner.plugins.work_dir import WorkDir

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

    def _popen(self, **kwargs: Any) -> Any:
        if sys.version_info.major == 3 and sys.version_info.minor >= 6:
            return subprocess.Popen(encoding=sys.stdout.encoding, **kwargs)
        return subprocess.Popen(**kwargs)


class LlvmCovCommands:
    def llvm_prof_merge_command(self, args: List[str]) -> List[str]:
        return [LLVM_PROFDATA_BINARY, 'merge'] + args

    def llvm_cov_export_command(self, args: List[str]) -> List[str]:
        return [LLVM_COV_PATH_BINARY, 'export'] + args


class LlvmCov:
    def __init__(self, build_dir: str, work_dir: WorkDir) -> None:
        self.build_dir = build_dir
        self.coverage_dir = work_dir.coverage_dir
        self.llvm_cov_commands = LlvmCovCommands()
        self.linux_commands = LinuxCommands()

    def do_find(self, search_directory: Path, extension: str) -> List[Path]:
        return list(Path(search_directory).rglob(extension))

    def get_uniq_profraw_profdata_file_paths(self) -> List[str]:
        pid = current_process().pid
        hash_code = uuid.uuid4()
        file_path = f"{self.coverage_dir.profdata_dir}/{pid}-{hash_code}"
        profraw_file = os.extsep.join([file_path, 'profraw'])
        profdata_file = os.extsep.join([file_path, 'profdata'])
        return [profraw_file, profdata_file]

    def llvm_profdata_merge(self, args: List[str]) -> None:
        prof_merge_command = self.llvm_cov_commands.llvm_prof_merge_command(args)
        self.linux_commands.run_command(prof_merge_command)

    def merge_and_delete_prowraw_files(self, profraw_file: str, profdata_file: str) -> None:
        file = Path(profraw_file)
        if file.is_file():
            self.llvm_profdata_merge(['--sparse', profraw_file, '-o', profdata_file])
            remove(profraw_file)
        else:
            Log.all(_LOGGER, f"File with name {profraw_file} not exist")

    def make_profdata_list_file(self) -> None:
        results = self.do_find(self.coverage_dir.profdata_dir, '*.profdata')
        with os.fdopen(os.open(self.coverage_dir.profdata_files_list_file, os.O_APPEND | os.O_CREAT, 0o755),
                "a", encoding="utf-8") as the_file:
            for i in results:
                the_file.write(str(i) + '\n')

    def merge_all_profdata_files(self) -> None:
        input_files = f"--input-files={self.coverage_dir.profdata_files_list_file}"
        self.llvm_profdata_merge(['--sparse', input_files, '-o', str(self.coverage_dir.profdata_merged_file)])

    def llvm_cov_export_to_info_file(self) -> None:
        instr_profile = f"-instr-profile={self.coverage_dir.profdata_merged_file}"

        bin_dir = f"{self.build_dir}/bin"

        bins = ['ark', 'ark_aot', 'ark_asm', 'ark_disasm', 'c2abc',
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

        command = self.llvm_cov_commands.llvm_cov_export_command(args)

        with os.fdopen(os.open(self.coverage_dir.info_file, os.O_WRONLY | os.O_CREAT, 0o755), \
                       "w", encoding="utf-8") as file_dot_info:
            self.linux_commands.run_command(command, stdout=file_dot_info)

    def genhtml(self) -> None:
        output_directory = f"--output-directory={self.coverage_dir.html_report_dir}"
        self.linux_commands.do_genhtml([output_directory, str(self.coverage_dir.info_file)])
