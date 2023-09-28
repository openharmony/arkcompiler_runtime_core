from dataclasses import dataclass
from functools import cached_property
from typing import Dict, List

from runner.options.decorator_value import value, _to_bool, _to_int, _to_str


@dataclass
class ArkAotOptions:
    def __str__(self) -> str:
        return _to_str(self, ArkAotOptions, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "timeout": self.timeout,
            "aot-args": self.aot_args,
        }

    @cached_property
    @value(yaml_path="ark_aot.enable", cli_name="aot", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="ark_aot.timeout", cli_name="paoc_timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return 600

    @cached_property
    @value(yaml_path="ark_aot.aot-args", cli_name="aot_args")
    def aot_args(self) -> List[str]:
        return []
