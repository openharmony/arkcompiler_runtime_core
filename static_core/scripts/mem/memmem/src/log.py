# -- coding: utf-8 --
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

import pathlib
import sys
import typing

LogLevel = typing.Literal["info", "warn", "err"]

_LEVEL_VALUES: dict[LogLevel, int] = {
    "info": 0,
    "warn": 1,
    "err": 2,
}

_PREFIXES: dict[LogLevel, str] = {
    "info": "info",
    "warn": "warning",
    "err": "error",
}

_logger: "MemmemLogger | None" = None


class MemmemLogger:
    def __init__(
        self,
        level: LogLevel = "err",
        log_file: str | pathlib.Path | None = None,
    ) -> None:
        self._level = parse_log_level(level)
        log_file_path = pathlib.Path(
            log_file) if log_file is not None else None
        self._file_stream: typing.TextIO | None = None
        self._stdout = sys.stdout
        self._stderr = sys.stderr
        if log_file_path is not None:
            self._file_stream = log_file_path.open("w", encoding="utf-8")
            self._stdout = self._file_stream
            self._stderr = self._file_stream

    def info(self, message: str) -> None:
        self._log("info", message)

    def warn(self, message: str) -> None:
        self._log("warn", message)

    def err(self, message: str) -> None:
        self._log("err", message)

    def reset(self) -> None:
        if self._file_stream is not None:
            self._file_stream.close()
            self._file_stream = None
        self._stdout = sys.stdout
        self._stderr = sys.stderr
        self._level = "err"

    def _log(self, level: LogLevel, message: str) -> None:
        if _LEVEL_VALUES[level] < _LEVEL_VALUES[self._level]:
            return
        line = f"{_PREFIXES[level]}: {message}\n"
        stream = self._stderr if level == "err" else self._stdout
        stream.write(line)
        stream.flush()


def configure_logger(
    level: LogLevel = "err",
    log_file: str | pathlib.Path | None = None,
) -> MemmemLogger:
    global _logger
    parsed_level = parse_log_level(level)
    if _logger is not None:
        _logger.reset()
    _logger = MemmemLogger(parsed_level, log_file)
    return _logger


def get_logger() -> MemmemLogger:
    global _logger
    if _logger is None:
        _logger = MemmemLogger("err")
    return _logger


def reset_logger() -> None:
    global _logger
    if _logger is not None:
        _logger.reset()
    _logger = None


def parse_log_level(value: str) -> LogLevel:
    if value == "info" or value == "warn" or value == "err":
        return typing.cast(LogLevel, value)
    raise ValueError(f"invalid memmem log level: {value}")
