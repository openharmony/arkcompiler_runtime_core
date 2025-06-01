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

"""Manage output files."""

from abc import ABC, abstractmethod
from contextlib import contextmanager
from io import StringIO
from os import makedirs, path
from pathlib import Path
from typing import Generic, ParamSpec, TypeVar

from typing_extensions import override

P = ParamSpec("P")
T = TypeVar("T", bound="OutputBase")


class OutputBase(ABC, Generic[P]):
    """Abstract base class for all types of generated output.

    Created, managed and saved to file via an `OutputManager`.
    """

    @abstractmethod  # pyre-ignore
    def __init__(
        self,
        *args: P.args,
        **kwargs: P.kwargs,
    ):
        """Initialize the output instance."""

    @classmethod
    def create(
        cls: type[T],  # pyre-ignore
        tm: "OutputManager",
        filename: str,
        *args: P.args,
        **kwargs: P.kwargs,
    ) -> T:
        """Create or retrieve an output instance via the manager."""
        return tm.get_or_create(cls, filename, *args, **kwargs)

    @abstractmethod
    def save_as(self, file_path: Path):
        """Save the output to the specified file path."""


class IndentManager:
    def __init__(self):
        self.count = 0

    @contextmanager
    def offset(self, n=4):
        try:
            self.count += n
            yield
        finally:
            self.count -= n

    @property
    def current(self):
        return self.count * " "


class STSOutputBuffer(OutputBase[[]]):
    """Represents a general target file."""

    @override
    def __init__(self):
        super().__init__()
        self.indent_manager = IndentManager()
        self.code = StringIO()
        self.import_dict: dict[str, str] = {}

    @override
    def save_as(self, file_path: Path):
        if not path.exists(file_path.parent):
            makedirs(file_path.parent, exist_ok=True)
        with open(file_path, "w", encoding="utf-8") as dst:
            for import_name, module_name in self.import_dict.items():
                dst.write(f'import * as {import_name} from "./{module_name}";\n')
            dst.write(self.code.getvalue())

    @contextmanager
    def code_block(self, start: str, end: str, n=4):
        self.writeln(start)
        with self.indent_manager.offset(n):
            yield
        self.writeln(end)

    def write(self, codes: str):
        for code in codes.splitlines():
            self.code.write(self.indent_manager.current + code + "\n")

    def writeln(self, *codes: str):
        for code in codes:
            self.code.write(self.indent_manager.current + code + "\n")

    def imports(self, import_dict: dict[str, str]):
        self.import_dict.update(import_dict)


class COutputBuffer(OutputBase[[bool]]):
    """Represents a C or C++ target file."""

    @override
    def __init__(self, is_header: bool):
        super().__init__(is_header)
        self.is_header = is_header
        self.headers: dict[str, None] = {}
        self.indent_manager = IndentManager()
        self.code = StringIO()

    @contextmanager
    def code_block(self, start: str, end: str, n=4):
        self.writeln(start)
        with self.indent_manager.offset(n):
            yield
        self.writeln(end)

    def write(self, codes: str):
        for code in codes.splitlines():
            self.code.write(self.indent_manager.current + code + "\n")

    def writeln(self, *codes: str):
        for code in codes:
            self.code.write(self.indent_manager.current + code + "\n")

    @override
    def save_as(self, file_path: Path):
        if not path.exists(file_path.parent):
            makedirs(file_path.parent, exist_ok=True)
        with open(file_path, "w", encoding="utf-8") as dst:
            if self.is_header:
                dst.write(f"#pragma once\n")
            for header in self.headers:
                dst.write(f'#include "{header}"\n')
            dst.write(self.code.getvalue())

    def include(self, *headers: str, back=False):
        for header in headers:
            self.headers.setdefault(header, None)


class OutputManager:
    """Manages the creation and saving of output files."""

    def __init__(self):
        """Initialize with an empty cache."""
        self.targets: dict[str, OutputBase] = {}

    def output_to(self, dst_dir: Path):
        """Save all managed outputs to a target directory."""
        for filename, target in self.targets.items():
            target.save_as(dst_dir / filename)

    def get_or_create(
        self,
        cls: type[T],
        filename: str,
        *args,
        **kwargs,
    ) -> T:
        """Get or create an output instance by filename."""
        if target := self.targets.get(filename):
            return target

        target = cls(*args, **kwargs)
        self.targets[filename] = target
        return target
