#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

import logging
from os import path, makedirs
from typing import NoReturn, Type, Optional

from runner.enum_types.verbose_format import VerboseKind

SUMMARY_LOG_LEVEL = 21
NONE_LOG_LEVEL = 22


class Log:
    _is_init = False

    @staticmethod
    def setup(verbose: VerboseKind, report_root: str) -> logging.Logger:
        logger = logging.getLogger("runner")

        log_path = report_root if report_root is not None else \
            path.join(path.sep, "tmp")
        makedirs(log_path, exist_ok=True)

        file_handler = logging.FileHandler(path.join(log_path, "runner.log"))
        console_handler = logging.StreamHandler()

        if verbose == VerboseKind.ALL:
            logger.setLevel(logging.DEBUG)
            file_handler.setLevel(logging.DEBUG)
            console_handler.setLevel(logging.DEBUG)
        elif verbose == VerboseKind.SHORT:
            logger.setLevel(logging.INFO)
            file_handler.setLevel(logging.INFO)
            console_handler.setLevel(logging.INFO)
        else:
            logger.setLevel(NONE_LOG_LEVEL)
            file_handler.setLevel(NONE_LOG_LEVEL)
            console_handler.setLevel(NONE_LOG_LEVEL)

        file_formatter = logging.Formatter('%(asctime)s - %(name)s - %(message)s')
        file_handler.setFormatter(file_formatter)
        console_formatter = logging.Formatter('%(message)s')
        console_handler.setFormatter(console_formatter)

        logger.addHandler(file_handler)
        logger.addHandler(console_handler)
        logger.addHandler(logging.NullHandler())

        return logger

    @staticmethod
    def all(logger: logging.Logger, message: str) -> None:
        """
        Logs on the level verbose=ALL
        """
        logger.debug(message)

    @staticmethod
    def short(logger: logging.Logger, message: str) -> None:
        """
        Logs on the level verbose=SHORT
        """
        logger.info(message)

    @staticmethod
    def summary(logger: logging.Logger, message: str) -> None:
        """
        Logs on the level verbose=SUMMARY (sum)
        """
        logger.log(SUMMARY_LOG_LEVEL, message)

    @staticmethod
    def default(logger: logging.Logger, message: str) -> None:
        """
        Logs on the level verbose=None
        """
        logger.log(NONE_LOG_LEVEL, message)

    @staticmethod
    def exception_and_raise(logger: logging.Logger, message: str,
                            exception_cls: Optional[Type[Exception]] = None) -> NoReturn:
        """
        Logs and throw the exception
        """
        logger.critical(message)
        if exception_cls is None:
            raise Exception(message)
        raise exception_cls(message)
