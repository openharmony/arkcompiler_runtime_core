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

"""Manages diagnostics messages such as semantic errors."""

from abc import ABC, abstractmethod
from collections.abc import Callable, Iterable
from contextlib import contextmanager
from dataclasses import dataclass, field
from enum import IntEnum
from sys import stderr
from typing import (
    TYPE_CHECKING,
    ClassVar,
    Optional,
    TextIO,
    TypeVar,
)

if TYPE_CHECKING:
    from taihe.utils.sources import SourceLocation


T = TypeVar("T")


class AnsiStyle:
    RED = "\033[31m"
    GREEN = "\033[32m"
    BLUE = "\033[33m"
    YELLOW = "\033[34m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"

    RESET = "\033[39m"
    BRIGHT = "\033[1m"
    RESET_ALL = "\033[0m"


def _passthrough(x):
    return x


def _discard(x):
    return ""


FilterT = Callable[[str], str]


###################
# The Basic Types #
###################


class Level(IntEnum):
    NOTE = 0
    WARN = 1
    ERROR = 2
    FATAL = 3


@dataclass
class DiagBase(ABC):
    """The base class for diagnostic messages."""

    LEVEL: ClassVar[Level]
    LEVEL_DESC: ClassVar[str]
    STYLE: ClassVar[str]

    loc: Optional["SourceLocation"] = field(kw_only=True)
    """The source location where the diagnostic refers to."""

    def notes(self) -> Iterable["DiagNote"]:
        """Returns the associated notes."""
        return ()

    def _format(self, f: FilterT) -> str:
        return (
            f"{f(AnsiStyle.BRIGHT)}{self.loc or '???'}: "  # "example.taihe:7:20: "
            f"{f(self.STYLE)}{self.LEVEL_DESC}{f(AnsiStyle.RESET)}: "  # "error: "
            f"{self.format_msg}{f(AnsiStyle.RESET_ALL)}"  # "redefinition of ..."
        )

    def __str__(self) -> str:
        return self._format(_discard)

    @property
    @abstractmethod
    def format_msg(self) -> str: 
        ...


######################################
# Base classes with different levels #
######################################


@dataclass
class DiagNote(DiagBase):
    LEVEL = Level.NOTE
    LEVEL_DESC = "note"
    STYLE = AnsiStyle.CYAN


@dataclass
class DiagWarn(DiagBase):
    LEVEL = Level.WARN
    LEVEL_DESC = "warning"
    STYLE = AnsiStyle.MAGENTA


@dataclass
class DiagError(DiagBase, Exception):
    LEVEL = Level.ERROR
    LEVEL_DESC = "error"
    STYLE = AnsiStyle.RED


@dataclass
class DiagFatalError(DiagError):
    LEVEL = Level.FATAL
    LEVEL_DESC = "fatal"


########################


class AbstractDiagnosticsManager(ABC):
    @abstractmethod
    def emit(self, diag: DiagBase) -> None: 
        ...

    @contextmanager
    def capture_error(self):
        """Captures "error" and "fatal" diagnostics using context manager.

        Example:
        ```
        # Emit the error and prevent its propogation
        with diag_mgr.capture_error():
            foo();
            raise DiagError(...)
            bar();

        # Equivalent to:
        try:
            foo();
            raise DiagError(...)
            bar();
        except DiagError as e:
            diag_mgr.emit(e)
        ```
        """
        try:
            yield None
        except DiagError as e:
            self.emit(e)

    def for_each(self, xs: Iterable[T], cb: Callable[[T], bool | None]) -> bool:
        """Calls `cb` for each element. Records and recovers from `DiagError`s.

        Returns `True` if no errors are encountered.
        """
        no_error = True
        for x in xs:
            try:
                if cb(x):
                    return True
            except DiagError as e:
                self.emit(e)
                no_error = False
        return no_error


class DiagnosticsManager(AbstractDiagnosticsManager):
    """Manages diagnostic messages."""

    def __init__(self, out: TextIO = stderr):
        self._out = out
        if self._out.isatty():
            self._color_filter_fn = _passthrough
        else:
            self._color_filter_fn = _discard
        self.reset_max_level()

    def _write(self, s: str):
        self._out.write(s)

    def _flush(self):
        self._out.flush()

    # TODO: could be slow.
    def _render_source_location(self, loc: "SourceLocation"):
        MAX_LINE_NO_SPACE = 5
        if not loc.has_pos:
            return

        line_contents = loc.file.read()

        if loc.start_row < 1 or loc.stop_row > len(line_contents):
            return

        for line, line_content in enumerate(line_contents, 1):
            if line < loc.start_row or line > loc.stop_row:
                continue

            line_content = line_content.rstrip("\n")

            # The first line: content.
            self._write(f"{line:{MAX_LINE_NO_SPACE}} | " f"{line_content}\n")

            # The second line: marker.
            markers = "".join(
                (
                    " "
                    if (line == loc.start_row and col < loc.start_col)
                    or (line == loc.stop_row and col > loc.stop_col)
                    else "^"
                )
                for col in range(1, len(line_content) + 1)
            )

            f = self._color_filter_fn
            self._write(
                f"{'':{MAX_LINE_NO_SPACE}} | "
                f"{f(AnsiStyle.GREEN + AnsiStyle.BRIGHT)}{markers}{f(AnsiStyle.RESET_ALL)}\n"
            )

    def _render(self, d: DiagBase):
        self._write(f"{d._format(self._color_filter_fn)}\n")
        if d.loc:
            self._render_source_location(d.loc)

    def emit(self, diag: DiagBase) -> None:
        """Emits a new diagnostic message."""
        self._max_level_record = max(self._max_level_record, diag.LEVEL)
        self._render(diag)
        for n in diag.notes():
            self._render(n)
        stderr.flush()

    def reset_max_level(self):
        self._max_level_record = -1

    def current_max_level(self):
        return self._max_level_record

    def has_errors(self):
        return self.current_max_level() >= Level.ERROR
