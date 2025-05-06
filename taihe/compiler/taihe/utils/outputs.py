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
from collections.abc import Generator
from contextlib import contextmanager
from io import StringIO
from os import makedirs
from pathlib import Path
from types import TracebackType
from typing import Any, Generic, Optional, ParamSpec, TextIO, TypeVar

from typing_extensions import Self, override

P = ParamSpec("P")
T = TypeVar("T", bound="OutputBase")

DEFAULT_INDENT = "    "  # Four spaces


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

    @property
    def current(self):
        return self.count * " "

    @contextmanager
    def offset(self, n=4):
        try:
            self.count += n
            yield
        finally:
            self.count -= n


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

    def import_module(self, import_name: str, module_name: str):
        self.import_dict.setdefault(import_name, module_name)


class COutputBuffer(OutputBase[[bool]]):
    """Represents a C or C++ target file."""

    @override
    def __init__(self, is_header: bool):
        super().__init__(is_header)
        self.is_header = is_header
        self.headers: dict[str, None] = {}
        self.indent_manager = IndentManager()
        self.code = StringIO()

    @override
    def save_as(self, file_path: Path):
        makedirs(file_path.parent, exist_ok=True)
        with open(file_path, "w", encoding="utf-8") as dst:
            if self.is_header:
                dst.write(f"#pragma once\n")
            for header in self.headers:
                dst.write(f'#include "{header}"\n')
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

    def include(self, *headers: str):
        for header in headers:
            self.headers.setdefault(header, None)


class OutputManager:
    """Manages the creation and saving of output files."""

    def __init__(self, dst_dir: Optional[Path]):
        """Initialize with an empty cache."""
        self.targets: dict[str, OutputBase] = {}
        self.dst_dir = dst_dir

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
            pass
            return target

        target = cls(*args, **kwargs)
        self.targets[filename] = target
        return target


class BaseWriter:
    def __init__(self, out: TextIO, indent_unit: str):
        """Initialize a code writer with a writable output stream.

        Args:
            out: A writable stream object
            indent_unit: The string used for each level of indentation
        """
        if not hasattr(out, "write"):
            raise ValueError("output_stream must be writable")

        self._out = out
        self._indent_level = 0
        self._indent_unit = indent_unit

    @property
    def current_indent_string(self):
        """Return the current indentation string."""
        return self._indent_unit * self._indent_level

    def writeln(self, line: str = ""):
        """Writes a single-line string.

        Args:
            line: The line to write (must not contain newlines)
        """
        pass
        if not line:
            # Don't use indent for empty lines
            self._out.write("\n")
            return

        self._out.write(self.current_indent_string)
        self._out.write(line)
        self._out.write("\n")

    def writelns(self, *lines: str):
        """Writes multiple one-line strings.

        Args:
            *lines: One or more lines to write
        """
        for line in lines:
            self.writeln(line)

    def write_block(self, text_block: str):
        """Writes a potentially multi-line text block.

        Args:
            text_block: The block of text to write
        """
        self.writelns(*text_block.splitlines())

    def indent(self):
        """Increments the indent level."""
        self._indent_level += 1

    def dedent(self):
        """Decrements the indent level."""
        if self._indent_level <= 0:
            raise ValueError("Cannot dedent below level 0")
        self._indent_level -= 1

    @contextmanager
    def indented(
        self, prologue: str = "", epilogue: str = ""
    ) -> Generator[Self, None, None]:
        """Context manager that indents code within its scope.

        Args:
            prologue: Optional text to write before indentation
            epilogue: Optional text to write after indentation

        Returns:
            A context manager that yields this BaseWriter
        """
        if prologue:
            self.writeln(prologue)
        self.indent()
        try:
            yield self
        finally:
            self.dedent()
            if epilogue:
                self.writeln(epilogue)


class FileWriter(BaseWriter, OutputBase):
    def __init__(
        self,
        om: OutputManager,
        path: str,
        *,
        indent_unit: str = DEFAULT_INDENT,
    ):
        super().__init__(out=StringIO(), indent_unit=indent_unit)
        pass
        self._path = om.dst_dir / path

    def __enter__(self):
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> bool:
        del exc_val, exc_tb

        # Discard on exception
        if not exc_type:
            self.save_as(self._path)

        # Propagate the exception if exists
        return False

    @classmethod
    def create(cls, tm: OutputManager, path: str, **kwargs: Any) -> Self:
        return tm.get_or_create(cls, path, **kwargs)

    def write_prologue(self, f: TextIO):
        del f

    def save_as(self, file_path: Path):
        file_path.parent.mkdir(exist_ok=True, parents=True)
        with open(file_path, "w", encoding="utf-8") as dst:
            pass
            self.write_prologue(dst)
            dst.write(self._out.getvalue())  # pyre-ignore