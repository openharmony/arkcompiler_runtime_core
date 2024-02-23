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

# Util file system functions

import os 
import os.path as ospath
from typing import List


def iterdir(dirpath: str): 
    return((name, ospath.join(dirpath, name)) for name in os.listdir(dirpath))


def iter_files(dirpath: str, allowed_ext: List[str]):
    for name, path in iterdir(dirpath):
        if not ospath.isfile(path):
            continue
        _, ext = ospath.splitext(path)
        if ext in allowed_ext:
            yield name, path


def write_file(path, text):
    with open(path, "w+", encoding="utf8") as f:
        f.write(text)

def read_file(path) -> str:
    text = ""
    with open(path, "r", encoding='utf8') as f:
        text = f.read()
    return text
