#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import json
import re
from pathlib import Path
from typing import Any

import yaml
from pydantic import BaseModel, ConfigDict, Field, ValidationError, field_validator
from yaml.scanner import ScannerError

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.base_enum import BaseEnum
from runner.logger import Log
from runner.options.options_step import StepKind
from runner.utils import ExpectedField

METADATA_PATTERN = re.compile(r"(?<=/\*---)(.*?)(?=---\*/)", flags=re.DOTALL)
DOTS_WHITESPACES_PATTERN = r"(\.\w+)*"
PACKAGE_PATTERN = re.compile(f"\\n\\s*package[\\t\\f\\v  ]+(?P<package_name>\\w+{DOTS_WHITESPACES_PATTERN})\\b")
SPEC_CHAPTER_PATTERN = re.compile(r"^\d{1,2}\.?\d{0,3}\.?\d{0,3}\.?\d{0,3}\.?\d{0,3}$")
RawData = str | list[str] | dict | None

__LOGGER = Log.get_logger(__file__)


class Tags:
    class EtsTag(BaseEnum):
        COMPILE_ONLY = "compile-only"
        NO_WARMUP = "no-warmup"
        NOT_A_TEST = "not-a-test"
        NEGATIVE = "negative"

    def __init__(self, tags: str | list[str] | None = None) -> None:
        self.__values: dict[Tags.EtsTag, bool] = {
            Tags.EtsTag.COMPILE_ONLY: Tags.__contains(Tags.EtsTag.COMPILE_ONLY.value, tags),
            Tags.EtsTag.NEGATIVE: Tags.__contains(Tags.EtsTag.NEGATIVE.value, tags),
            Tags.EtsTag.NOT_A_TEST: Tags.__contains(Tags.EtsTag.NOT_A_TEST.value, tags),
            Tags.EtsTag.NO_WARMUP: Tags.__contains(Tags.EtsTag.NO_WARMUP.value, tags),
        }
        self.invalid_tags = Tags.get_invalid_tags(tags)

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
    def get_invalid_tags(tags: str | list[str] | None) -> list[str]:
        if isinstance(tags, str):
            tags = [tag.strip() for tag in tags.split(",")]

        if tags:
            return [tag for tag in tags if tag not in Tags.EtsTag.values()]
        return []

    @staticmethod
    def __contains(tag: str, tags: str | list[str] | None) -> bool:
        return tag in tags if tags is not None else False


class TestMetadata(BaseModel):   # type: ignore[explicit-any]
    model_config = ConfigDict(
        populate_by_name=True,
        arbitrary_types_allowed=True,
    )

    tags: Tags = Field(default_factory=Tags)
    desc: str | None = Field(default=None, alias='description')
    test_suffix: str | None = None
    file_name: str | None = None
    files: list[str] | None = None
    assertion: str | None = Field(default=None, alias="assert")
    params: Any | None = None   # type: ignore[explicit-any]
    name: str | None = None
    entry_point: str | None = None
    test_cli: list[str] | None = None
    package: str | None = None
    ark_options: list[str] = Field(default_factory=list)
    timeout: int | None = None
    es2panda_options: list[str] = Field(default_factory=list, alias="flags")
    spec: str | None = None
    arktsconfig: Path | None = None
    expected_out: ExpectedField = Field(default_factory=dict)
    expected_error: ExpectedField = Field(default_factory=dict)
    # Test262 specific metadata keys
    description: str | None = None
    defines: str | None = None
    includes: str | None = None
    features: str | None = None
    esid: str | None = None
    es5id: str | None = None
    es6id: str | None = None
    author: str | None = None
    info: str | None = None
    locale: str | None = None
    # ark_tests/parser
    issues: str | None = None

    @classmethod
    def get_metadata(cls, path: Path) -> 'TestMetadata':
        data = Path.read_text(path)
        yaml_text = "\n".join(re.findall(METADATA_PATTERN, data))
        if not yaml_text:
            return cls.create_empty_metadata(path)
        try:
            metadata = yaml.safe_load(yaml_text)
            if metadata is None or isinstance(metadata, str):
                return cls.create_empty_metadata(path)
            return cls.create_filled_metadata(metadata, path)
        except (FileNotFoundError, TypeError, AttributeError, ScannerError) as error:
            logger = globals().get('__LOGGER')
            if isinstance(logger, Log):
                raise InvalidConfiguration(f"Yaml parsing: '{path}' - error: '{error}'") from error
            return cls.create_empty_metadata(path)

    @classmethod
    def create_filled_metadata(cls, metadata: dict[str, Any],       # type: ignore[explicit-any]
                               path: Path) -> 'TestMetadata':
        try:
            parsed_metadata = cls.model_validate(metadata)
        except ValidationError as err:
            raise InvalidConfiguration(f"Incorrect test metadata in test {path}") from err

        package = parsed_metadata.package
        arktsconfig = parsed_metadata.arktsconfig

        if arktsconfig is not None:
            arktsconfig_path = (path.parent / arktsconfig).resolve()
            parsed_metadata = parsed_metadata.model_copy(
                update={"arktsconfig": arktsconfig_path if arktsconfig_path.exists() else None})

            package = cls.get_package_statement_from_arktsconfig(path, arktsconfig_path)
        if package is None:
            package = cls.get_package_statement_from_source(path)

        parsed_metadata = parsed_metadata.model_copy(update={"package": package})
        return parsed_metadata

    @classmethod
    def create_empty_metadata(cls, path: Path, find_package: bool = True) -> 'TestMetadata':
        package = cls.get_package_statement_from_source(path) if find_package else None
        return cls(package=package)

    @classmethod
    def get_package_statement_from_source(cls, path: Path) -> str | None:
        data = Path.read_text(path)
        match = PACKAGE_PATTERN.search(data)
        if match is None:
            return None
        if (stmt := match.group("package_name")) is not None:
            return stmt
        return path.stem

    @classmethod
    def get_package_statement_from_arktsconfig(cls, path: Path, arktsconfig_path: Path) -> str | None:
        with open(arktsconfig_path, encoding="utf-8") as json_handler:
            data = json.load(json_handler)
        package = data.get("compilerOptions", {}).get('package', None)
        return f"{package}.{path.stem}" if package is not None else None

    @field_validator("tags", mode="before")
    @classmethod
    def _normalize_tags(cls, value: RawData) -> Tags:
        if isinstance(value, (list, str)) or value is None:
            return Tags(value)
        raise ValueError("tags must be a list of strings")

    @field_validator("ark_options", mode="before")
    @classmethod
    def _normalize_ark_options(cls, value: RawData) -> list[str]:
        if value is None:
            return []
        if isinstance(value, list):
            return value
        if isinstance(value, str):
            return [value]
        raise ValueError("ark_options must be a string or a list of strings")

    @field_validator("desc", mode="before")
    @classmethod
    def _normalize_desc(cls, value: RawData) -> str | None:
        if value is None or isinstance(value, str):
            return value
        if isinstance(value, dict):
            return str(value)
        raise ValueError("Unsupported description format")

    @field_validator("expected_out", "expected_error", mode="before")
    @classmethod
    def _normalize_expected_field(cls, value: RawData) -> ExpectedField:
        """
        we expect expected_out/ expected_err to be in format {step_name | step_kind: list[str]},
        if step_name/ step_kind is not set explicitly - consider expected output to be related to compiler step
        """
        if value is None:
            return {}
        if isinstance(value, list):
            return {StepKind.COMPILER.value: value}
        if isinstance(value, str):
            return {StepKind.COMPILER.value: [exp_str for exp_str in value.splitlines() if exp_str.strip()]}
        if isinstance(value, dict):
            normalized: dict[str, list[str]] = {}
            for key, val in value.items():
                normalized[key] = [val] if isinstance(val, str) else val
            return normalized

        raise ValueError("Incorrect format of the expected_out or expected_error field in test mark-up")

    @field_validator("test_cli", "files", "es2panda_options", mode="before")
    @classmethod
    def _check_fields_lists(cls, value: RawData) -> list[str] | None:
        if value is None:
            return None
        if isinstance(value, list):
            return value
        raise ValueError("Expected list of strings")

    def get_package_name(self) -> str:
        return self.package if self.package is not None else ""
