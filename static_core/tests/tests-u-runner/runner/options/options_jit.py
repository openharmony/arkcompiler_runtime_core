from dataclasses import dataclass
from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_jit_preheats, _to_str


@dataclass
class JitOptions:
    def __str__(self) -> str:
        return _to_str(self, JitOptions, 2)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "num_repeats": self.num_repeats,
            "compiler_threshold": self.compiler_threshold,
        }

    @cached_property
    @value(yaml_path="ark.jit.enable", cli_name="jit", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="ark.jit.num_repeats",
        cli_name="jit_preheat_repeats",
        cast_to_type=lambda x: _to_jit_preheats(x, prop="num_repeats", default_if_empty=30)
    )
    def num_repeats(self) -> int:
        return 0

    @cached_property
    @value(
        yaml_path="ark.jit.compiler_threshold",
        cli_name="jit_preheat_repeats",
        cast_to_type=lambda x: _to_jit_preheats(x, prop="compiler_threshold", default_if_empty=20)
    )
    def compiler_threshold(self) -> Optional[int]:
        return None
