from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_jit_preheats, _to_str


class JitOptions:
    __DEFAULT_REPEATS = 0
    __DEFAULT_REPEATS_IF_EMPTY = 30
    __DEFAULT_THRESHOLD_IF_EMPTY = 20

    def __str__(self) -> str:
        return _to_str(self, 2)

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
        cast_to_type=lambda x: _to_jit_preheats(
            cli_value=x, prop="num_repeats",
            default_if_empty=JitOptions.__DEFAULT_REPEATS_IF_EMPTY)
    )
    def num_repeats(self) -> int:
        return JitOptions.__DEFAULT_REPEATS

    @cached_property
    @value(
        yaml_path="ark.jit.compiler_threshold",
        cli_name="jit_preheat_repeats",
        cast_to_type=lambda x: _to_jit_preheats(
            cli_value=x, prop="compiler_threshold",
            default_if_empty=JitOptions.__DEFAULT_THRESHOLD_IF_EMPTY)
    )
    def compiler_threshold(self) -> Optional[int]:
        return None

    def get_command_line(self) -> str:
        options = '--jit' if self.enable else ''
        jit_options = [
            f'num_repeats={self.num_repeats}' if self.num_repeats != JitOptions.__DEFAULT_REPEATS else '',
            f'compiler_threshold={self.compiler_threshold}' if self.compiler_threshold is not None else '',
        ]
        jit_options = [option for option in jit_options if option]

        if jit_options:
            options += f' --jit-preheat-repeats="{",".join(jit_options)}"'
        return options
