from functools import cached_property
from typing import Dict

from runner.options.decorator_value import value, _to_str, _to_bool


class ETSOptions:
    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "force-generate": self.force_generate,
        }

    @cached_property
    @value(yaml_path="ets.force-generate", cli_name="is_force_generate", cast_to_type=_to_bool)
    def force_generate(self) -> bool:
        return False

    def get_command_line(self) -> str:
        return '--force-generate' if self.force_generate else ''
