from dataclasses import dataclass
from functools import cached_property
from typing import Dict

from runner.options.decorator_value import value, _to_str, _to_int


@dataclass
class GroupsOptions:
    def __str__(self) -> str:
        return _to_str(self, GroupsOptions, 2)

    def to_dict(self) -> Dict[str, object]:
        return {
            "quantity": self.quantity,
            "number": self.number
        }

    @cached_property
    @value(yaml_path="test-lists.groups.quantity", cli_name="groups", cast_to_type=_to_int)
    def quantity(self) -> int:
        return 1

    @cached_property
    @value(yaml_path="test-lists.groups.number", cli_name="group_number", cast_to_type=_to_int)
    def number(self) -> int:
        return 1
