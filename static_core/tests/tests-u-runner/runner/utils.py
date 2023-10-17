import logging
import os
import random
import shutil
from enum import Enum
from os import makedirs, path, remove
from typing import TypeVar, Callable, Optional, Type, Union, Any, List
from pathlib import Path
from urllib import request
from urllib.error import URLError
import zipfile

from runner.logger import Log

_LOGGER = logging.getLogger("runner.utils")


def progress(block_num, block_size, total_size):
    Log.summary(_LOGGER, f"Downloaded block: {block_num} ({block_size}). Total: {total_size}")


def download(name, git_url, revision, target_path, show_progress=False):
    archive_file = path.join(path.sep, 'tmp', f'{name}.zip')
    url_file = f'{git_url}/{revision}.zip'

    Log.summary(_LOGGER, f"Downloading from {url_file} to {archive_file}")
    try:
        if show_progress:
            request.urlretrieve(url_file, archive_file, progress)
        else:
            request.urlretrieve(url_file, archive_file)
    except URLError:
        Log.exception_and_raise(_LOGGER, f'Downloading {url_file} file failed.')

    Log.summary(_LOGGER, f"Extracting archive {archive_file} to {target_path}")
    if path.exists(target_path):
        shutil.rmtree(target_path)

    try:
        with zipfile.ZipFile(archive_file) as arch:
            arch.extractall(path.dirname(target_path))
    except (zipfile.BadZipfile, zipfile.LargeZipFile):
        Log.exception_and_raise(_LOGGER, f'Failed to unzip {archive_file} file')

    remove(archive_file)


ProcessCopyLambda = Callable[[str, str], None]


def generate(name: str, url: str, revision: str, generated_root: Path, *,
             stamp_name: Optional[str] = None, test_subdir: str = "test", show_progress: bool = False,
             process_copy: Optional[ProcessCopyLambda] = None, force_download: bool = False):
    Log.summary(_LOGGER, "Prepare test files")
    stamp_name = f'{name}-{revision}' if not stamp_name else stamp_name
    dest_path = path.join(generated_root, stamp_name)
    makedirs(dest_path, exist_ok=True)
    stamp_file = path.join(dest_path, f'{stamp_name}.stamp')

    if not force_download and path.exists(stamp_file):
        return dest_path

    temp_path = path.join(path.sep, 'tmp', name, f'{name}-{revision}')

    if force_download or not path.exists(temp_path):
        download(
            name,
            url,
            revision,
            temp_path,
            show_progress
        )

    if path.exists(dest_path):
        shutil.rmtree(dest_path)

    Log.summary(_LOGGER, "Copy and transform test files")
    if process_copy is not None:
        process_copy(
            path.join(temp_path, test_subdir),
            dest_path
        )
    else:
        copy(
            path.join(temp_path, test_subdir),
            dest_path
        )

    Log.summary(_LOGGER, f"Create stamp file {stamp_file}")
    with open(stamp_file, 'w+', encoding="utf-8") as _:  # Create empty file-marker and close it at once
        pass

    return dest_path


def copy(source_path, dest_path, remove_if_exist: bool = True):
    try:
        if path.exists(dest_path) and remove_if_exist:
            shutil.rmtree(dest_path)
        shutil.copytree(source_path, dest_path, dirs_exist_ok=not remove_if_exist)
    except OSError as ex:
        Log.exception_and_raise(_LOGGER, str(ex))


def read_file(file_path) -> str:
    with open(file_path, "r", encoding='utf8') as f_handle:
        text = f_handle.read()
    return text


def write_2_file(file_path, content):
    """
    write content to file if file exists it will be truncated. if file does not exist it wil be created
    """
    makedirs(path.dirname(file_path), exist_ok=True)
    with open(file_path, mode='w+', encoding="utf-8") as f_handle:
        f_handle.write(content)


def purify(line):
    return line.strip(" \n").replace(" ", "")


EnumClass = TypeVar("EnumClass", bound=Enum)


def enum_from_str(item_name: str, enum_cls: Type[EnumClass]) -> Optional[EnumClass]:
    for enum_value in enum_cls:
        if enum_value.value.lower() == item_name.lower() or str(enum_value).lower() == item_name.lower():
            return enum_cls(enum_value)
    return None


def wrap_with_function(code: str, jit_preheat_repeats: int) -> str:
    return f"""
    function repeat_for_jit() {{
        {code}
    }}

    for(let i = 0; i < {jit_preheat_repeats}; i++) {{
        repeat_for_jit(i)
    }}
    """


def iterdir(dirpath: str):
    return ((name, path.join(dirpath, name)) for name in os.listdir(dirpath))


def iter_files(dirpath: Union[str, Path], allowed_ext: List[str]):
    for name, path_value in iterdir(str(dirpath)):
        if not path.isfile(path_value):
            continue
        _, ext = path.splitext(path_value)
        if ext in allowed_ext:
            yield name, path_value


def is_type_of(value: Any, str_type: str) -> bool:
    return str(type(value)).find(str_type) > 0


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
