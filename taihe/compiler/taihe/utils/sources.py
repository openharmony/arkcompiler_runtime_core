# coding=utf-8
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

"""Manages source files."""

from abc import ABC, abstractmethod
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class SourceBase(ABC):
    """Base class reprensenting all kinds of source code."""

    @property
    @abstractmethod
    def source_identifier(self) -> str: 
        ...

    @property
    @abstractmethod
    def pkg_name(self) -> str: 
        ...

    @abstractmethod
    def read(self) -> list[str]: 
        ...


@dataclass(frozen=True)
class SourceFile(SourceBase):
    """Represents a file-based source code."""

    path: Path

    @property
    def source_identifier(self) -> str:
        return str(self.path)

    @property
    def pkg_name(self) -> str:
        return self.path.stem

    def read(self) -> list[str]:
        with open(self.path) as f:
            return f.readlines()


@dataclass(frozen=True)
class SourceBuffer(SourceBase):
    """Represents a string-based source code."""

    name: str
    buf: str

    @property
    def source_identifier(self) -> str:
        return f"<source-buffer-{self.pkg_name}>"

    @property
    def pkg_name(self) -> str:
        return self.name

    def read(self) -> list[str]:
        return self.buf.splitlines()


class SourceManager:
    """Manages all input files throughout the compilation."""

    src_list: list[SourceBase]

    def __init__(self):
        self.src_list = []

    def add_source(self, sb: SourceBase):
        self.src_list.append(sb)

    @property
    def sources(self) -> Iterable[SourceBase]:
        return self.src_list


@dataclass
class SourceLocation:
    """Represents a location (either a position or a region) within a file."""

    file: SourceBase
    """Required: The source file associated with the location."""

    has_pos: bool

    start_row: int
    """Optional: The start line number (1-based)."""

    start_col: int
    """Optional: The start column number (1-based)."""

    stop_row: int
    """Optional: The stop line number (1-based)."""

    stop_col: int
    """Optional: The stop column number (1-based)."""

    def __init__(self, file: SourceBase, *pos: int):
        self.file = file

        if len(pos) == 4:
            self.start_row, self.start_col, self.stop_row, self.stop_col = pos
            self.has_pos = True
        elif len(pos) == 0:
            self.start_row, self.start_col, self.stop_row, self.stop_col = 0, 0, 0, 0
            self.has_pos = False
        else:
            raise ValueError()

    def __str__(self) -> str:
        r = self.file.source_identifier
        if self.has_pos:
            r = f"{r}:{self.start_row}:{self.start_col}"

        return r

    @classmethod
    def with_path(cls, path: Path) -> "SourceLocation":
        """Returns a file-only source location, without any position information."""
        return cls(SourceFile(path))
