from dataclasses import dataclass
from functools import cached_property
from typing import Dict

from runner.options.decorator_value import value, _to_bool, _to_str


@dataclass
class QuickOptions:
    def __str__(self) -> str:
        return _to_str(self, QuickOptions, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
        }

    @cached_property
    @value(yaml_path="quick.enable", cli_name="quick", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False
