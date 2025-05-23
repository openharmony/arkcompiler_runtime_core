#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from argparse import ArgumentParser
from functools import lru_cache
from typing import List, Optional, Tuple, Dict

import yaml
from pydantic import BaseModel
from pydantic_settings import BaseSettings

from .models.options import OptionModel


class SyntaxModel(BaseModel):
    tokenizer: Dict[str, List[Tuple[str, str]]]


class Server(BaseModel):
    uvicorn_config: dict
    cors: bool = False
    cors_allow_origins: List[str] = []
    cors_allow_methods: List[str] = []
    cors_allow_headers: List[str] = []
    cors_allow_credentials: bool = False


class Binary(BaseModel):
    build: str
    timeout: int = 120
    # Path to icu dat file f.e 'icudt72l.dat'
    icu_data: str = ""


class PlaygroundSettings(BaseSettings):
    server: Server
    binary: Binary
    options: List[OptionModel]
    syntax: SyntaxModel


@lru_cache
def get_settings(args: Optional[Tuple] = None) -> PlaygroundSettings:
    parser = ArgumentParser()
    parser.add_argument("--config", "-c", type=str)
    args = parser.parse_args(args)
    with open(args.config, "r", encoding="utf-8") as f:
        conf = yaml.safe_load(f)
    return PlaygroundSettings(**conf)


@lru_cache
def get_options(args: Optional[Tuple] = None) -> List[OptionModel]:
    return get_settings(args).options


@lru_cache
def get_syntax(args: Optional[Tuple] = None) -> SyntaxModel:
    return get_settings(args).syntax
