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