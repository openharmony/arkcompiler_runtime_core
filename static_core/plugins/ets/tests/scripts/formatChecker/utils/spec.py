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

from pathlib import Path

import yaml 
from utils.file_structure import TestDirectory, normalize_section_name
from utils.exceptions import InvalidFileFormatException
from utils.constants import SPEC_SECTION_TITLE_FIELD_NAME, SPEC_SUBSECTIONS_FIELD_NAME
from utils.fsutils import read_file

Spec = dict

def build_spec_tree(specpath: str, root: TestDirectory):
    spec = __parse_spec(specpath)
    walk_spec(spec, root, specpath=specpath)


def walk_spec(spec: list, parent: TestDirectory, specpath: str): 
    if not isinstance(spec, list):
        raise InvalidFileFormatException(message="Spec sections list must be a YAML list", filepath=specpath)

    for s in spec:
        if not isinstance(spec, dict):
            raise InvalidFileFormatException(message="Spec section must be a YAML dict", filepath=specpath)
        
        s = dict(s)
        if not SPEC_SECTION_TITLE_FIELD_NAME in s:
            raise InvalidFileFormatException(
                message=f"Spec section must contain the '{SPEC_SECTION_TITLE_FIELD_NAME}' field",
                filepath=specpath)

        # Normalize name 
        name = normalize_section_name(str(s[SPEC_SECTION_TITLE_FIELD_NAME]))
       
        td = TestDirectory(path=(parent.path / Path(name)), parent=parent) 
        parent.add_subdir(td)   

        # Has subsections?
        if SPEC_SUBSECTIONS_FIELD_NAME in s: 
            walk_spec(s[SPEC_SUBSECTIONS_FIELD_NAME], parent=td, specpath=specpath)


def __parse_spec(path: str) -> Spec:
    text = read_file(path)

    try:
        spec = yaml.safe_load(text)
    except Exception as e:
        raise InvalidFileFormatException(message=f"Could not load YAML: {str(e)}", filepath=path)
    
    return spec
