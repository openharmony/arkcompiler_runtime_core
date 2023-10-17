from dataclasses import dataclass
from functools import cached_property
from typing import Dict, List, Optional

from runner.options.decorator_value import value, _to_int, _to_str
from runner.options.options_jit import JitOptions


@dataclass
class ArkOptions:
    def __str__(self) -> str:
        return _to_str(self, ArkOptions, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "timeout": self.timeout,
            "interpreter-type":self.interpreter_type,
            "jit": self.jit.to_dict(),
            "ark-args": self.ark_args
        }

    @cached_property
    @value(yaml_path="ark.timeout", cli_name="timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return 10

    @cached_property
    @value(yaml_path="ark.interpreter-type", cli_name="interpreter_type")
    def interpreter_type(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="ark.ark-args", cli_name="ark_args")
    def ark_args(self) -> List[str]:
        return []

    jit = JitOptions()
