#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

import argparse
from email import message 
import os 
import os.path as ospath
from pathlib import Path
from re import T
from unittest import loader
from typing import List
from jinja2 import Environment, FileSystemLoader, select_autoescape, TemplateSyntaxError 
from utils.file_structure import walk_test_subdirs
from utils.metainformation import InvalidMetaException, find_all_metas 
from utils.fsutils import iter_files, write_file
from utils.constants import SKIP_PREFIX, VARIABLE_START_STRING, YAML_EXTENSIONS
from utils.constants import TEMPLATE_EXTENSION, OUT_EXTENSION, LIST_PREFIX
from utils.exceptions import *
from utils.test_parameters import load_params 

# ============================================
# Working with templates and their results

env = Environment(
    loader=FileSystemLoader("."),
    autoescape=select_autoescape(),
    variable_start_string=VARIABLE_START_STRING
)

def render_template(filepath: str, params=None) -> str:
    """
    Renders a single template and returns result as string 
    """
    if params is None:
        params = dict()
    try:
        template = env.get_template(filepath)
        return template.render(**params)
    except TemplateSyntaxError as e: 
        raise InvalidFileFormatException(message=f"Tempate Syntax Error: ${e.message}", filepath=filepath)
    except Exception as e:
        raise UnknownTemplateException(filepath=filepath, exception=e)


def split_into_tests(text: str, filepath: str) -> List[str]: 
    """
    Splits rendered template into multiple tests.
    'filepath' is needed for exceptions 
    """
    result = []

    try:
        start_indices = [metainfile[0] for metainfile in find_all_metas(text)]
    except InvalidMetaException as e:
        raise InvalidFileFormatException(
            filepath=filepath,
            message=f"Error raised while splitting a rendered template: {e.message}")

    for i in range(1, len(start_indices)):
        left = start_indices[i - 1]
        right = start_indices[i]
        result.append(text[left:right])
    result.append(text[start_indices[-1]:])

    return result


# ============================================
# Recursively walk the FS, save rendered templates 

def render_and_write_templates(dirpath, outpath):
    """
    Loads parameters and renders all templates in `dirpath`.
    Saves the results in `outpath`
    """
    os.makedirs(outpath, exist_ok=True)

    params = load_params(dirpath)
    for name, path in iter_files(dirpath, allowed_ext=[TEMPLATE_EXTENSION]):
        name_without_ext, _ = ospath.splitext(name)
        if name_without_ext.startswith(SKIP_PREFIX):
            continue
        tests = split_into_tests(render_template(path, params), path)
        assert len(tests) > 0, "Internal error: there should be tetst"
        if len(tests) == 1:
            # if there is only one test we do not add any suffixes
            write_file(path=ospath.join(outpath, name), text=tests[0])
        else:
            # if there are multiple tests we add any suffixes: name_0.ets, name_1.ets and etc
            for i, test in enumerate(tests):
                output_filepath = ospath.join(outpath, f"{name_without_ext}_{i}{OUT_EXTENSION}")
                write_file(path=output_filepath, text=test)
        
def process_tests(root: Path, outpath: Path):
    """
    Renders all templates and saves them.
    """
    for folder in walk_test_subdirs(root):
        dir_outpath = outpath / folder.path.relative_to(root)
        render_and_write_templates(str(folder.path), str(dir_outpath))


# ============================================
# Entry


parser = argparse.ArgumentParser(description='Generate CTS tests from a set of templates.')
parser.add_argument('tests_dir', help='Path to directory that contains tests templates and parameters')
parser.add_argument('output_dir',
                    help='Path to output directory.'
                         ' Output directory and all subdirectories are created automatically')


def main():
    args = parser.parse_args()
    root = Path(args.tests_dir)
    outpath = Path(args.output_dir)

    if not root.is_dir():
        print(f"ERROR: {str(root.absolute())} must be a directory")
        exit(1)
    
    try:
        process_tests(root, outpath)
    except InvalidFileFormatException as e:
        print("Error:\n", e.message)
        exit(1)
    except InvalidFileStructureException as e:
        print("Error:\n", e.message)
        exit(1)
    except UnknownTemplateException as e:
        print(f"{e.filepath}: exception while processing template:")
        print("\t", repr(e.exception))
        exit(1)
    print("Finished")
            
if __name__ == "__main__":
    main()
