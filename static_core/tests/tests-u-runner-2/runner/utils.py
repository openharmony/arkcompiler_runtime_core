#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
#

import argparse
import importlib
import os
import random
import re
import shutil
import subprocess
import threading
import zipfile
from concurrent.futures import Future
from dataclasses import dataclass
from filecmp import cmp
from itertools import tee
from os import makedirs, path, remove
from pathlib import Path
from types import ModuleType
from typing import Callable, Optional, Type, Union, Any, List, Iterator, Tuple, Iterable, Dict
from urllib import request
from urllib.error import URLError

import yaml

from runner.common_exceptions import DownloadException, UnzipException
from runner.enum_types.base_enum import EnumT
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


def progress(block_num: int, block_size: int, total_size: int) -> None:
    _LOGGER.summary(f"Downloaded block: {block_num} ({block_size}). Total: {total_size}")


def download(name: str, *, git_url: str, revision: str, download_root: Path, target_path: Path,
             show_progress: bool = False) -> None:
    archive_file = path.join(download_root, f'{name}.zip')
    url_file = f'{git_url}/{revision}.zip'

    _LOGGER.summary(f"Downloading from {url_file} to {archive_file}")
    try:
        if show_progress:
            request.urlretrieve(url_file, archive_file, progress)
        else:
            request.urlretrieve(url_file, archive_file)
    except URLError as exc:
        raise DownloadException(f'Downloading {url_file} file failed.') from exc

    _LOGGER.summary(f"Extracting archive {archive_file} to {target_path}")
    if path.exists(target_path):
        shutil.rmtree(target_path)

    try:
        with zipfile.ZipFile(archive_file) as arch:
            arch.extractall(path.dirname(target_path))
    except (zipfile.BadZipfile, zipfile.LargeZipFile) as exc:
        raise UnzipException(f'Failed to unzip {archive_file} file') from exc

    remove(archive_file)


ProcessCopy = Callable[[Path, Path], None]


def download_and_generate(name: str, url: str, revision: str, download_root: Path, generated_root: Path, *,
                          stamp_name: Optional[str] = None, test_subdir: str = "test", show_progress: bool = False,
                          process_copy: Optional[ProcessCopy] = None, force_download: bool = False) -> Path:
    _LOGGER.summary("Prepare test files")
    stamp_name = f'{name}-{revision}' if not stamp_name else stamp_name
    dest_path = generated_root / stamp_name
    makedirs(dest_path, exist_ok=True)
    stamp_file = path.join(dest_path, f'{stamp_name}.stamp')

    if not force_download and path.exists(stamp_file):
        return dest_path

    temp_path = download_root / f'{name}-{revision}'

    if force_download or not path.exists(temp_path):
        download(
            name=name,
            git_url=url,
            revision=revision,
            download_root=download_root,
            target_path=temp_path,
            show_progress=show_progress
        )

    if dest_path.exists():
        shutil.rmtree(dest_path)

    _LOGGER.summary("Copy and transform test files")
    if process_copy is not None:
        process_copy(temp_path / test_subdir, dest_path)
    else:
        copy(temp_path / test_subdir, dest_path)

    _LOGGER.summary(f"Create stamp file {stamp_file}")
    with os.fdopen(os.open(stamp_file, os.O_RDWR | os.O_CREAT, 0o755),
                   'w+', encoding="utf-8") as _:  # Create empty file-marker and close it at once
        pass

    return dest_path


def copy(source_path: Path, dest_path: Path, remove_if_exist: bool = True) -> None:
    if not source_path.exists():
        error = f"Source path {source_path} does not exist! Cannot process this collection"
        raise FileNotFoundError(error)
    if source_path == dest_path:
        return
    if dest_path.exists() and remove_if_exist:
        shutil.rmtree(dest_path)
    shutil.copytree(source_path, dest_path, dirs_exist_ok=not remove_if_exist)


def read_file(file_path: Union[Path, str]) -> str:
    with os.fdopen(os.open(file_path, os.O_RDONLY, 0o755), "r", encoding='utf8') as f_handle:
        text = f_handle.read()
    return text


def write_2_file(file_path: Union[Path, str], content: str) -> None:
    """
    write content to file if file exists it will be truncated. if file does not exist it wil be created
    """
    makedirs(path.dirname(file_path), exist_ok=True)
    with open(file_path, mode='w+', encoding="utf-8") as f_handle:
        f_handle.write(content)


def purify(line: str) -> str:
    return line.strip(" \n").replace(" ", "")


def get_all_enum_values(enum_cls: Type[EnumT], *, delim: str = ", ", quotes: str = "'") -> str:
    result = []
    for enum_value in enum_cls:
        result.append(f"{quotes}{enum_value.value}{quotes}")
    return delim.join(result)


def wrap_with_function(code: str, jit_preheat_repeats: int) -> str:
    return f"""
    function repeat_for_jit() {{
        {code}
    }}

    for(let i = 0; i < {jit_preheat_repeats}; i++) {{
        repeat_for_jit(i)
    }}
    """


def iter_files(dirpath: Union[Path, str], allowed_ext: List[str]) -> Iterator[Tuple[str, str]]:
    dirpath_gen = ((name, path.join(str(dirpath), name)) for name in os.listdir(str(dirpath)))
    for name, path_value in dirpath_gen:
        if not path.isfile(path_value):
            continue
        _, ext = path.splitext(path_value)
        if ext in allowed_ext:
            yield name, path_value


def is_type_of(value: Any, type_: str) -> bool:
    return str(type(value)).find(type_) > 0


def get_platform_binary_name(name: str) -> str:
    if os.name.startswith("nt"):
        return name + ".exe"
    return name


def get_group_number(test_id: str, total_groups: int) -> int:
    """
    Calculates the number of a group by test_id
    :param test_id: string value test path or test_id
    :param total_groups: total number of groups
    :return: the number of a group in the range [1, total_groups],
    both boundaries are included.
    """
    random.seed(test_id)
    return random.randint(1, total_groups)


# from itertools import pairwise when switching to python version >= 3.10
def pair_wise(iterable: Iterable[Path]) -> Iterator[Tuple[Path, Path]]:
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


def compare_files(files: List[Path]) -> bool:
    for f1, f2 in pair_wise(files):
        if not cmp(f1, f2):
            return False
    return True


def is_build_instrumented(binary: str, line: str, threshold: int = 0) -> bool:
    check_instrumented = f"nm -an {binary}"

    _LOGGER.all(f"Run: {check_instrumented}")
    _, output = subprocess.getstatusoutput(check_instrumented)

    return len(re.findall(line, output)) >= threshold


def indent(times: int) -> str:
    return "    " * times


def has_macro(prop_value: str) -> bool:
    pattern = r"\$\{([^\}]+)\}"
    match = re.search(pattern, str(prop_value))
    return match is not None


def get_all_macros(prop_value: str) -> List[str]:
    pattern = r"\$\{([^\}]+)\}"
    result: List[str] = []
    for m in re.finditer(pattern, prop_value):
        result.append(m.group(1))
    return result


def replace_macro(prop_value: str, macro: str, replacing_value: str) -> str:
    pattern = r"\$\{" + macro + r"\}"
    if macro in prop_value:
        match = re.sub(pattern, replacing_value, prop_value, 0, re.IGNORECASE)
        return match
    return prop_value


def expand_file_name(arg: str) -> Optional[str]:
    if arg is None or arg.startswith("${"):
        return arg
    return path.abspath(path.expanduser(arg))


def is_directory(arg: str, create_if_not_exist: bool = False) -> str:
    expanded: Optional[str] = expand_file_name(arg)
    if expanded is None:
        raise argparse.ArgumentTypeError(f"The directory {arg} does not exist")
    if not path.isdir(expanded):
        if create_if_not_exist:
            makedirs(expanded)
        else:
            raise argparse.ArgumentTypeError(f"The directory {arg} does not exist")

    return str(expanded)


@dataclass
class UiUpdater:
    title: str
    timeout: int = 2

    @staticmethod
    def __in_progress(timer_function: Callable, future: Future) -> None:
        _LOGGER.default(". ")
        if future.running():
            timer_function(future)

    @classmethod
    def __timer(cls, future: Future) -> None:
        retimer = threading.Timer(2, cls.__in_progress, args=(cls.__timer, future))
        retimer.start()

    def start(self, future: Future) -> None:
        _LOGGER.default(f"{self.title}. ")
        self.__in_progress(self.__timer, future)


def make_dir_if_not_exist(arg: str) -> str:
    if not path.isdir(path.abspath(arg)):
        makedirs(arg)

    return str(path.abspath(arg))


def is_file(arg: str) -> str:
    expanded: Optional[str] = expand_file_name(arg)
    if expanded is None or not path.isfile(expanded):
        raise argparse.ArgumentTypeError(f"The file {arg} does not exist")

    return str(expanded)


def check_int(value: str, value_name: str, *, is_zero_allowed: bool = False) -> int:
    ivalue = int(value)
    if ivalue < 0 or (not is_zero_allowed and ivalue == 0):
        raise argparse.ArgumentTypeError(f"{value} is an invalid {value_name} value")

    return ivalue


def convert_minus(line: str) -> str:
    return line.replace("-", "_")


def convert_underscore(line: str) -> str:
    return line.replace("_", "-")


def remove_prefix(s: str, prefix: str) -> str:
    """Strip prefix from string."""
    return s[len(prefix):] if s.startswith(prefix) else s


def pretty_divider(length: int = 100) -> str:
    return '=' * length


def get_class_by_name(clazz: str) -> Type:
    last_dot = clazz.rfind(".")
    class_path = clazz[:last_dot]
    class_name = clazz[last_dot + 1:]
    class_module_runner: ModuleType = importlib.import_module(class_path)
    class_obj: Type = getattr(class_module_runner, class_name)
    return class_obj


def get_config_folder() -> Path:
    urunner_folder = Path(__file__).parent.parent
    return urunner_folder.joinpath("cfg")


def get_config_workflow_folder() -> Path:
    return get_config_folder().joinpath("workflows")


def get_config_test_suite_folder() -> Path:
    return get_config_folder().joinpath("test-suites")


def load_config(config_path: Optional[str]) -> Dict[str, Any]:
    __cfg_type = "type"
    config_from_file = {}
    if config_path is not None:
        with open(config_path, "r", encoding="utf-8") as stream:
            try:
                config_from_file = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                message = f"{exc}, {yaml.YAMLError}"
                raise yaml.YAMLError(message)
        if __cfg_type not in config_from_file:
            error = f"Cannot detect type of config '{config_path}'. Have you specified key '{__cfg_type}'?"
            raise yaml.YAMLError(error)
    return config_from_file


def to_bool(value: Any) -> bool:
    if isinstance(value, bool):
        return value
    if isinstance(value, str) and str(value).lower() in ("true", "false"):
        return str(value).lower() == "true"
    raise ValueError(f"'{value}' cannot be converted to 'bool'")


def extract_parameter_name(param_name: str) -> str:
    pattern = r"^\$\{parameters\.([^\.\}]+)\}$"
    match = re.match(pattern, param_name)
    if match:
        return match.group(1)
    return param_name


def correct_path(root: Union[str, Path], test_list: Union[str, Path]) -> Path:
    if isinstance(test_list, str):
        test_list = Path(test_list)
    if isinstance(root, str):
        root = Path(root)
    return test_list.absolute() if test_list.exists() else root.joinpath(test_list)


def get_test_id(file: Path, start_directory: Path) -> str:
    relpath = file.relative_to(start_directory)
    return str(relpath)


def prepend_list(pre_list: List[Any], post_list: List[Any]) -> List[Any]:
    result = pre_list[:]
    result.extend(post_list)
    return result


def check_obligatory_env(var_name: str) -> None:
    if os.getenv(var_name) is None:
        raise EnvironmentError(f"Obligatory environment variable '{var_name}' is not set")
