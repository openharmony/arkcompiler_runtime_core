import logging
from os import path, makedirs
from typing import TypeVar, NoReturn, Type, Optional

from runner.enum_types.verbose_format import VerboseKind

SUMMARY_LOG_LEVEL = 21
NONE_LOG_LEVEL = 22


class Log:
    _is_init = False

    @staticmethod
    def setup(verbose: VerboseKind, report_root: str):
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

        Log._is_init = True

        return logger

    @staticmethod
    def all(logger, message: str):
        """
        Logs on the level verbose=ALL
        """
        if Log._is_init:
            logger.debug(message)
        else:
            print(message)

    @staticmethod
    def short(logger, message: str):
        """
        Logs on the level verbose=SHORT
        """
        if Log._is_init:
            logger.info(message)
        else:
            print(message)

    @staticmethod
    def summary(logger, message: str):
        """
        Logs on the level verbose=SUMMARY (sum)
        """
        if Log._is_init:
            logger.log(SUMMARY_LOG_LEVEL, message)
        else:
            print(message)

    @staticmethod
    def default(logger, message: str):
        """
        Logs on the level verbose=None
        """
        if Log._is_init:
            logger.log(NONE_LOG_LEVEL, message)
        else:
            print(message)

    T = TypeVar("T", bound=Exception)

    @staticmethod
    def exception_and_raise(logger, message: str, exception_cls: Optional[Type[T]] = None) -> NoReturn:
        """
        Logs and throw the exception
        """
        if Log._is_init:
            logger.critical(message)
        if exception_cls is None:
            raise Exception(message)
        raise exception_cls(message)
