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

import re
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Optional, Any, List

import yaml

METADATA_PATTERN = re.compile(r"(?<=\/\*---)(.*?)(?=---\*\/)", flags=re.DOTALL)
PACKAGE_PATTERN = re.compile(r"\n\s*package[\t\f\v  ]+(?P<package_name>\w+(\.\w+)*)\b")


class Tags:
    class EtsTag(Enum):
        COMPILE_ONLY = "compile-only"
        NO_WARMUP = "no-warmup"
        NOT_A_TEST = "not-a-test"
        NEGATIVE = "negative"

    def __init__(self, tags: Optional[List[str]] = None) -> None:
        self.__compile_only = Tags.__contains(Tags.EtsTag.COMPILE_ONLY.value, tags)
        self.__negative = Tags.__contains(Tags.EtsTag.NEGATIVE.value, tags)
        self.__not_a_test = Tags.__contains(Tags.EtsTag.NOT_A_TEST.value, tags)
        self.__no_warmup = Tags.__contains(Tags.EtsTag.NO_WARMUP.value, tags)

    @staticmethod
    def __contains(tag: str, tags: Optional[List[str]]) -> bool:
        return tag in tags if tags is not None else False

    @property
    def compile_only(self) -> bool:
        return self.__compile_only

    @property
    def negative(self) -> bool:
        return self.__negative

    @property
    def not_a_test(self) -> bool:
        return self.__not_a_test

    @property
    def no_warmup(self) -> bool:
        return self.__no_warmup


@dataclass
class TestMetadata:
    tags: Tags
    desc: Optional[str] = None
    files: Optional[List[str]] = None
    assertion: Optional[str] = None
    params: Optional[Any] = None
    name: Optional[str] = None
    package: Optional[str] = None
    ark_options: List[str] = field(default_factory=list)
    timeout: Optional[int] = None


def get_metadata(path: Path) -> TestMetadata:
    data = Path.read_text(path)
    yaml_text = "\n".join(re.findall(METADATA_PATTERN, data))
    metadata = yaml.safe_load(yaml_text)
    if metadata is None:
        metadata = {}
    metadata['tags'] = Tags(metadata.get('tags'))
    metadata['assertion'] = metadata.get('assert')
    if 'assert' in metadata:
        del metadata['assert']
    if isinstance(type(metadata.get('ark_options')), str):
        metadata['ark_options'] = [metadata['ark_options']]
    metadata['package'] = metadata.get("package")
    if metadata['package'] is None:
        metadata['package'] = get_package_statement(path)
    return TestMetadata(**metadata)


def get_package_statement(path: Path) -> Optional[str]:
    data = Path.read_text(path)
    match = PACKAGE_PATTERN.search(data)
    if match is None:
        return None
    return stmt if (stmt := match.group("package_name")) is not None else None
