#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

import logging
from typing import Dict, Optional, List, Union, Any

import yaml

from runner import utils
from runner.logger import Log

_LOGGER = logging.getLogger("runner.options.yaml_document")


class YamlDocument:
    _document: Optional[Dict[str, Any]] = None

    @staticmethod
    def document() -> Optional[Dict[str, Any]]:
        return YamlDocument._document

    @staticmethod
    def load(config_path: Optional[str]) -> None:
        YamlDocument._document = {}
        if config_path is None:
            return

        with open(config_path, "r", encoding="utf-8") as stream:
            try:
                YamlDocument._document = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                Log.exception_and_raise(_LOGGER, str(exc), yaml.YAMLError)

    @staticmethod
    def save(config_path: str, data: Dict[str, Any]) -> None:
        data_to_save = yaml.dump(data, indent=4)
        utils.write_2_file(config_path, data_to_save)

    @staticmethod
    def get_value_by_path(yaml_path: str) -> Optional[Union[int, bool, str, List[str]]]:
        yaml_parts = yaml_path.split(".")
        current: Any = YamlDocument._document
        for part in yaml_parts:
            if current and isinstance(current, dict) and part in current.keys():
                current = current.get(part)
            else:
                return None
        if current is None or isinstance(current, (bool, int, list, str)):
            return current

        Log.exception_and_raise(_LOGGER, f"Unsupported value type '{type(current)}' for '{yaml_path}'")
