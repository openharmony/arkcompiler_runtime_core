import argparse
import logging
from typing import Optional, List, Union

from runner.logger import Log

_LOGGER = logging.getLogger("runner.options.cli_args_wrapper")


class CliArgsWrapper:
    _args = None

    @staticmethod
    def setup(args: argparse.Namespace) -> None:
        CliArgsWrapper._args = args

    @staticmethod
    def get_by_name(name: str) -> Optional[Union[str, List[str]]]:
        if name in dir(CliArgsWrapper._args):
            value = getattr(CliArgsWrapper._args, name)
            if value is None:
                return None
            if isinstance(value, list):
                return value
            return str(value)

        Log.exception_and_raise(_LOGGER, f"Cannot recognize CLI option {name} ")
