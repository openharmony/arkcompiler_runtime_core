import argparse
import logging
import re
from functools import cached_property
from typing import Set, Dict

from runner.options.cli_args_wrapper import CliArgsWrapper
from runner.options.decorator_value import value, _to_test_suites, _to_str
from runner.options.options_ark import ArkOptions
from runner.options.options_ark_aot import ArkAotOptions
from runner.options.options_es2panda import Es2PandaOptions
from runner.options.options_ets import ETSOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_quick import QuickOptions
from runner.options.options_test_lists import TestListsOptions
from runner.options.options_time_report import TimeReportOptions
from runner.options.options_verifier import VerifierOptions
from runner.options.yaml_document import YamlDocument

_LOGGER = logging.getLogger("runner.options.config")


class Config:
    def __init__(self, args: argparse.Namespace):
        CliArgsWrapper.setup(args)
        YamlDocument.load(args.config)

    def __str__(self) -> str:
        return _to_str(self, 0)

    @cached_property
    @value(
        yaml_path="test-suites",
        cli_name=["test_suites", "test262", "parser", "hermes",
                  "ets_func_tests", "ets_runtime", "ets_cts", "ets_gc_stress"],
        cast_to_type=_to_test_suites,
        required=True
    )
    def test_suites(self) -> Set[str]:
        return set([])

    general = GeneralOptions()
    es2panda = Es2PandaOptions()
    verifier = VerifierOptions()
    quick = QuickOptions()
    ark_aot = ArkAotOptions()
    ark = ArkOptions()
    time_report = TimeReportOptions()
    test_lists = TestListsOptions()
    ets = ETSOptions()

    def _to_dict(self) -> Dict[str, object]:
        return {
            "test-suites": list(self.test_suites),
            "general": self.general.to_dict(),
            "es2panda": self.es2panda.to_dict(),
            "verifier": self.verifier.to_dict(),
            "quick": self.quick.to_dict(),
            "ark_aot": self.ark_aot.to_dict(),
            "ark": self.ark.to_dict(),
            "time-report": self.time_report.to_dict(),
            "test-lists": self.test_lists.to_dict(),
            "ets": self.ets.to_dict()
        }

    def generate_config(self) -> None:
        if self.general.generate_config is None:
            return
        data = self._to_dict()
        YamlDocument.save(self.general.generate_config, data)

    def get_command_line(self) -> str:
        _test_suites = ['--' + suite.replace('_', '-') for suite in self.test_suites]
        options = ' '.join([
            ' '.join(_test_suites),
            self.general.get_command_line(),
            self.es2panda.get_command_line(),
            self.verifier.get_command_line(),
            self.quick.get_command_line(),
            self.ark_aot.get_command_line(),
            self.ark.get_command_line(),
            self.time_report.get_command_line(),
            self.test_lists.get_command_line(),
            self.ets.get_command_line()
        ])
        options_str = re.sub(r'\s+', ' ', options, re.IGNORECASE | re.DOTALL)

        return options_str
