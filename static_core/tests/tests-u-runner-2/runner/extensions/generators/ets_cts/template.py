#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
from dataclasses import dataclass
from pathlib import Path

import yaml
from jinja2 import Environment, TemplateSyntaxError, select_autoescape

from runner.sts_utils.constants import (
    META_COPYRIGHT,
    META_END_COMMENT,
    META_END_STRING,
    META_START_COMMENT,
    META_START_STRING,
)
from runner.sts_utils.exceptions import InvalidFileFormatException, InvalidMetaException, UnknownTemplateException
from runner.utils import read_file

ROOT_PATH = Path(os.path.realpath(os.path.dirname(__file__)))
BENCH_PATH = ROOT_PATH / "test_template.tpl"
COPYRIGHT_PATH = ROOT_PATH / "copyright.txt"


@dataclass
class Meta:
    config: dict[str, str]
    code: str


class Template:
    def __init__(self, test_path: Path, params: dict[str, list]) -> None:
        self.test_path = str(test_path)
        self.text = read_file(test_path)
        self.__params = params
        self.__jinja_env = Environment(autoescape=select_autoescape())

        if self.is_copyright:
            self.__copyright = read_file(COPYRIGHT_PATH)
        else:
            self.__copyright, out_content = self.__get_in_out_content(self.text, META_START_COMMENT, META_END_COMMENT)
            self.text = out_content.strip()

    @property
    def is_copyright(self) -> bool:
        return META_COPYRIGHT not in self.text

    @staticmethod
    def __replace_symbols(text: str) -> str:
        codes = [('&#39;', "'"), ('&#34;', '"'), ('&gt;', '>'), ('&lt;', '<'), ('&amp;', '&')]
        for old, new in codes:
            text = text.replace(old, new)
        return text

    @staticmethod
    def __get_in_out_content(text: str, meta_start: str, meta_end: str) -> tuple[str, str]:
        start, end = text.find(meta_start), text.find(meta_end)
        if start > end or start == -1 or end == -1:
            raise InvalidMetaException("Invalid meta or meta does not exist")
        inside_content = text[start + len(meta_start):end]
        outside_content = text[:start] + text[end + len(meta_end):]
        return inside_content, outside_content

    def render_template(self) -> list[str]:
        try:
            template = self.__jinja_env.from_string(self.text)
            rendered = template.render(**self.__params).strip()
        except TemplateSyntaxError as exc:
            raise InvalidFileFormatException(message=f"Template Syntax Error: {exc.message}",
                                             filepath=self.test_path) from exc
        except Exception as exc:
            raise UnknownTemplateException(filepath=self.test_path, exception=exc) from exc

        meta_start = META_START_STRING if self.is_copyright else META_START_COMMENT
        return [meta_start + e for e in rendered.split(meta_start) if e]

    def generate_test(self, test: str, fullname: str) -> str:
        meta = self.__get_meta(test)
        meta.config["name"] = fullname
        yaml_content = yaml.dump(meta.config)

        bench_template = read_file(BENCH_PATH)
        bench_code = bench_template.format(
            copyright=self.__copyright.strip(),
            description=yaml_content.strip(),
            code=meta.code.strip())
        return bench_code

    def __get_meta(self, text: str) -> Meta:
        test_text = self.__replace_symbols(text)
        inside_content, outside_content = self.__get_in_out_content(test_text, META_START_STRING, META_END_STRING)
        return Meta(config=self.__parse_yaml(inside_content), code=outside_content)

    def __parse_yaml(self, text: str) -> dict[str, str]:
        try:
            result: dict[str, str] = yaml.safe_load(text)
        except Exception as exc:
            raise InvalidFileFormatException(message=f"Could not load YAML in test params: {exc!s}",
                                             filepath=self.test_path) from exc
        return result
