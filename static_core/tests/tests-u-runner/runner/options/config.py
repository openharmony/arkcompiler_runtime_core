import logging
from dataclasses import dataclass
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


@dataclass
class Config:
    def __init__(self, args):
        CliArgsWrapper.setup(args)
        YamlDocument.setup(args.config)

    def __str__(self) -> str:
        return _to_str(self, Config, 0)

    @cached_property
    @value(
        yaml_path="test-suites",
        cli_name=["test_suites", "test262", "parser", "hermes", "ets_func_tests", "ets_runtime", "ets_cts"],
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

    def generate_config(self):
        if self.general.generate_config is None:
            return
        data = self._to_dict()
        YamlDocument.save(self.general.generate_config, data)
