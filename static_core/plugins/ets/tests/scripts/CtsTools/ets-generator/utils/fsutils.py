import os
import os.path as ospath
from typing import List
from utils.log import *


def iterdir(dirpath: str):
    return ((name, ospath.join(dirpath, name)) for name in os.listdir(dirpath))


def iter_files(dirpath: str, allowed_ext: List[str]):
    for name, path in iterdir(dirpath):
        if not ospath.isfile(path):
            continue
        _, ext = ospath.splitext(path)
        if ext in allowed_ext:
            yield name, path


def create_folder(path):
    info(f"Creating directory: {path}")
    os.makedirs(path)


def write_file(path, text):
    dir = path.parent
    if not (os.path.exists(dir)):
        create_folder(dir)

    info(f"Creating file: {path}")

    with open(path, "x", encoding="utf8") as f:
        f.write(text)


def read_file(path):
    with open(path, encoding="utf8") as f:
        return f.read()