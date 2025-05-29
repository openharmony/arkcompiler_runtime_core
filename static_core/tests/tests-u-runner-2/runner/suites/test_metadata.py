#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, Any, List, Dict

import yaml

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.base_enum import BaseEnum
from runner.logger import Log

METADATA_PATTERN = re.compile(r"(?<=/\*---)(.*?)(?=---\*/)", flags=re.DOTALL)
DOTS_WHITESPACES_PATTERN = r"(\.\w+)*"
PACKAGE_PATTERN = re.compile(f"\\n\\s*package[\\t\\f\\v  ]+(?P<package_name>\\w+{DOTS_WHITESPACES_PATTERN})\\b")
SPEC_CHAPTER_PATTERN = re.compile(r"^\d{1,2}\.?\d{0,3}\.?\d{0,3}\.?\d{0,3}\.?\d{0,3}$")

__LOGGER = Log.get_logger(__file__)


class Tags:
    class EtsTag(BaseEnum):
        COMPILE_ONLY = "compile-only"
        NO_WARMUP = "no-warmup"
        NOT_A_TEST = "not-a-test"
        NEGATIVE = "negative"

    def __init__(self, tags: Optional[List[str]] = None) -> None:
        self.__values: Dict[Tags.EtsTag, bool] = {
            Tags.EtsTag.COMPILE_ONLY: Tags.__contains(Tags.EtsTag.COMPILE_ONLY.value, tags),
            Tags.EtsTag.NEGATIVE: Tags.__contains(Tags.EtsTag.NEGATIVE.value, tags),
            Tags.EtsTag.NOT_A_TEST: Tags.__contains(Tags.EtsTag.NOT_A_TEST.value, tags),
            Tags.EtsTag.NO_WARMUP: Tags.__contains(Tags.EtsTag.NO_WARMUP.value, tags),
        }

    def __repr__(self) -> str:
        values = [tag.name for (tag, value) in self.__values.items() if value]
        return str(values)

    @property
    def compile_only(self) -> bool:
        return self.__values.get(Tags.EtsTag.COMPILE_ONLY, False)

    @property
    def negative(self) -> bool:
        return self.__values.get(Tags.EtsTag.NEGATIVE, False)

    @property
    def not_a_test(self) -> bool:
        return self.__values.get(Tags.EtsTag.NOT_A_TEST, False)

    @property
    def no_warmup(self) -> bool:
        return self.__values.get(Tags.EtsTag.NO_WARMUP, False)

    @staticmethod
    def __contains(tag: str, tags: Optional[List[str]]) -> bool:
        return tag in tags if tags is not None else False


@dataclass
class TestMetadata:
    tags: Tags = field(default_factory=Tags)
    desc: Optional[str] = None
    files: Optional[List[str]] = None
    assertion: Optional[str] = None
    params: Optional[Any] = None
    name: Optional[str] = None
    package: Optional[str] = None
    ark_options: List[str] = field(default_factory=list)
    timeout: Optional[int] = None
    es2panda_options: List[str] = field(default_factory=list)
    spec: Optional[str] = None
    # Test262 specific metadata keys
    description: Optional[str] = None
    defines: Optional[str] = None
    includes: Optional[str] = None
    features: Optional[str] = None
    esid: Optional[str] = None
    es5id: Optional[str] = None
    es6id: Optional[str] = None
    author: Optional[str] = None
    info: Optional[str] = None
    locale: Optional[str] = None

    @classmethod
    def get_metadata(cls, path: Path) -> 'TestMetadata':
        data = Path.read_text(path)
        yaml_text = "\n".join(re.findall(METADATA_PATTERN, data))
        try:
            metadata = yaml.safe_load(yaml_text)
            if metadata is None or isinstance(metadata, str):
                return cls.create_empty_metadata(path)
            return cls.create_filled_metadata(metadata, path)
        except (FileNotFoundError, TypeError, AttributeError) as error:
            logger = globals().get('__LOGGER')
            if isinstance(logger, Log):
                raise InvalidConfiguration(f"Yaml parsing: '{path}' - error: '{error}'") from error
            return cls.create_empty_metadata(path)

    @classmethod
    def create_filled_metadata(cls, metadata: Dict[str, Any], path: Path) -> 'TestMetadata':
        metadata['tags'] = Tags(metadata.get('tags'))
        if 'assert' in metadata:
            metadata['assertion'] = metadata.get('assert')
            del metadata['assert']
        if 'description' in metadata:
            metadata['desc'] = metadata['description']
        if 'flags' in metadata:
            flags_list = metadata['flags']
            if flags_list and isinstance(flags_list, list):
                metadata.update({'es2panda_options': flags_list})
            del metadata['flags']
        if isinstance(type(metadata.get('ark_options')), str):
            metadata['ark_options'] = [metadata['ark_options']]
        metadata['package'] = metadata.get("package")
        if metadata['package'] is None:
            metadata['package'] = cls.get_package_statement(path)
        return TestMetadata(**metadata)

    @classmethod
    def create_empty_metadata(cls, path: Path) -> 'TestMetadata':
        metadata = cls()
        if metadata.package is None:
            metadata.package = cls.get_package_statement(path)
        return metadata

    @classmethod
    def get_package_statement(cls, path: Path) -> Optional[str]:
        data = Path.read_text(path)
        match = PACKAGE_PATTERN.search(data)
        if match is None:
            return None
        if (stmt := match.group("package_name")) is not None:
            return stmt
        return path.stem
