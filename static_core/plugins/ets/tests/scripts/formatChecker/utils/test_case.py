
from pathlib import Path
from re import template
from typing import Tuple 

from utils.constants import NEGATIVE_PREFIX, NEGATIVE_EXECUTION_PREFIX, SKIP_PREFIX


def is_negative(path: Path) -> bool:
    return path.name.startswith(NEGATIVE_PREFIX) or path.name.startswith(NEGATIVE_EXECUTION_PREFIX)


def should_be_skipped(path: Path) -> bool:
    return path.name.startswith(SKIP_PREFIX)


def strip_template(path: Path) -> Tuple[str, int]:
    stem = path.stem
    i = path.stem.rfind("_")
    if i == -1:
        return stem, 0
    template_name = stem[:i]
    test_index = stem[i + 1:]
    if not test_index.isdigit():
        return stem, 0
    else:
        return template_name, int(test_index)