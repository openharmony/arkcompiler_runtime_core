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

import re
import shutil
from pathlib import Path

from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.options_collections import CollectionsOptions
from runner.suites.test_metadata import TestMetadata

_LOGGER = Log.get_logger(__file__)

START_ERRORS = re.compile(rb"/\*\s*@@[@?].*?\*/")


class SrcdumperGenerator(IGenerator):
    @staticmethod
    def _expects_error(file_path: Path) -> bool:
        content = file_path.read_bytes()
        return START_ERRORS.search(content) is not None

    def generate(self) -> list[str]:
        raw_files: list[Path] = []
        skipped = 0
        test_suite_root = self._config.test_suite.test_root
        coll = self.__get_collection()
        mask = f"**/*.{self._config.test_suite.extension(coll)}"
        for file_path in list(self._source.rglob(mask)):
            if not self._expects_error(file_path):
                raw_files.append(file_path)
                raw_files.extend(self._search_dependencies(file_path))
            else:
                skipped += 1

        for file_path in raw_files:
            test_id = file_path.relative_to(test_suite_root / coll.name).as_posix()
            target = self._target / test_id
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(file_path, target)

        _LOGGER.short(f"Collection {coll.name}: {skipped} files are excluded because they expect errors")

        return [raw_file.as_posix() for raw_file in raw_files]

    def _search_dependencies(self, file_path: Path) -> list[Path]:
        result: list[Path] = []
        metadata = TestMetadata.get_metadata(file_path)
        if metadata.files:
            for file in metadata.files:
                dep_file = file_path.parent / file
                result.append(dep_file)
                for deep_dep in self._search_dependencies(dep_file):
                    if deep_dep not in result:
                        result.append(deep_dep)
        return result

    def __get_collection(self) -> CollectionsOptions:
        for collection in self._config.test_suite.collections:
            if self._source.as_posix().endswith(collection.name):
                return collection
        raise ValueError(f"Collection for path {self._source} not found")
