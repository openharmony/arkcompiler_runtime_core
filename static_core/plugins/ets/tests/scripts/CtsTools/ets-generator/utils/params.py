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

import yaml
from utils.log import *
from utils.exceptions import InvalidFileFormatException


YAML_EXTENSIONS = ".yaml"
PARAM_SUFIX = ".params"


def generate_params(input, bench_name):
    input_dir = input.parent
    param_name = f"{bench_name}{PARAM_SUFIX}{YAML_EXTENSIONS}"
    param_path = input_dir.joinpath(param_name)
    
    result = dict()
    
    if(param_path.exists()):
        info(f"Generating yaml file for test: {input.name}")
        result = parse_yaml_list(param_path)
        info(f"Finish generating yaml file for test: {input.name}")
        
    return result


def parse_yaml_list(path):
    with open(path, "r", encoding='utf8') as f:
        text = f.read()
     
    try:
        params = yaml.load(text, Loader=yaml.FullLoader)
    except Exception as e:
        raise InvalidFileFormatException(message=f"Could not load YAML: {str(e)}", filepath=path)
    
    return params
