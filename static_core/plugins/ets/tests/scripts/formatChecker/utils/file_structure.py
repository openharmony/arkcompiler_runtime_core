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

# This file defines the CTS file structure 
# The entrypoint is the function 'walk_test_subdirs'

from dataclasses import dataclass
from pathlib import Path 
from typing import Iterable, Tuple, List, Union
import os
import re 
import os.path as ospath

from utils.exceptions import InvalidFileStructureException


@dataclass()
class TestDirectory:
    id: int
    path: Path
    name: str

    parent: Union['TestDirectory', None]
    subdirs: 'List[TestDirectory]'
    
    
    def __init__(self, path: Path, id: int = 0, name: str = "",
                 parent: Union['TestDirectory', None] = None,
                 subdirs: 'List[TestDirectory]' = None) -> None:
        self.path = path

        if id == 0 or name == "":
            self.id, self.name = parse_section_dir_name(path)
        else:
            self.id = id
            self.name = name 
        
        self.parent = parent
        if subdirs is None:
            self.subdirs = []
        else:
            self.subdirs = list(subdirs)
        
    
    def full_index(self) -> List[int]:
        cur = self
        result = []
        while (cur is not None):
            result.append(cur.id)
            cur = cur.parent
        return list(reversed(result))
    
    def iter_files(self, allowed_ext: List[str] = None) -> Iterable[Path]:
        for filename in os.listdir(str(self.path)):
            filepath: Path = self.path / filename
            if allowed_ext is not None and len(allowed_ext) > 0 and filepath.suffix not in allowed_ext:
                continue
            yield filepath
    
    def add_subdir(self, td: 'TestDirectory'):
        td.parent = self
        self.subdirs.append(td)

    def find_subdir_by_name(self, name: str) -> Union['TestDirectory', None]:
        # TODO: decrease complexity 
        for sd in self.subdirs:
            if sd.name == name:
                return sd
        return None

    def find_subdir_by_id(self, id: int) -> Union['TestDirectory', None]:
        # TODO: decrease complexity
        for sd in self.subdirs:
            if sd.id == id:
                return sd
        return None  

    def is_empty(self):
        return len(os.listdir(str(self.path))) == 0
     

def walk_test_subdirs(path: Path, parent=None) -> Iterable[TestDirectory]:
    """
    Walks the file system from the CTS root, yielding TestDirectories, in correct order:
    For example, if only directories 1, 1/1, 1/1/1, 1/1/2, 1/2 exist, they will be yielded in that order.
    """
    subdirs = [] 
    for name in os.listdir(str(path)):
        if (path / name).is_dir():
            subdirs.append(TestDirectory(parent=parent, path=(path / name)))       
    subdirs = sorted(subdirs, key=lambda dir: dir.id)

    for subdir in subdirs:
        yield subdir
        # walk recursively
        for subsubdir in walk_test_subdirs(subdir.path, subdir):
            yield subsubdir


def build_directory_tree(td: TestDirectory):
    subdirs = []
    for name in os.listdir(str(td.path)):
        if (td.path / name).is_dir():
            subdirs.append(TestDirectory(parent=td, path=(td.path / name)))
    subdirs = sorted(subdirs, key=lambda dir: dir.id)

    for sd in subdirs:
        td.add_subdir(sd)
        build_directory_tree(sd)



def print_tree(td: TestDirectory):
    for sd in td.subdirs:

        left_space = " " * 2 * len(sd.full_index())
        section_index = str(sd.id)
        section_name = sd.name.replace("_", " ").title()
        right_space = 90 - len(left_space) - len(section_index) - len(section_name)

        print(left_space, section_index, section_name, "." *  right_space, "\n")
        print_tree(sd)


def normalize_section_name(name: str) -> str:
    parts = name.split(".")
    if len(parts) != 2:
        raise InvalidFileStructureException(filepath=name,
                                            message="Invalid section name format")
    first, second = parts
    if not first.isdigit():
        raise InvalidFileStructureException(filepath=name,
                                            message="Invalid section name format")
    second = second.replace(" ",  "_").replace("-", "_").lower()
    return first + "." + "".join([c for c in second if c.isalpha() or c == "_"])


def denormalize_section_name(name: str) -> str:
    return name.replace("_", " ").title() 


def parse_section_dir_name(path: Path) -> Tuple[int, str]:
    parts = path.name.split(".")
    if len(parts) == 1:
        if not path.name.isdigit():
            raise InvalidFileStructureException(filepath=str(path), message="Invalid directory name format")
        return int(path.name), ""
    if len(parts) != 2:
        raise InvalidFileStructureException(filepath=str(path), message="Invalid directory name format")
    first, second = parts
    if not first.isdigit():
        raise InvalidFileStructureException(filepath=str(path), message="Invalid directory name format")
    if not re.match("^[a-z_]*$", second):
        raise InvalidFileStructureException(filepath=str(path), message="Section name must be lowercase and contain spaces")
    return int(first), second
