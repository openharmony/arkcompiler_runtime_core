from functools import cached_property
from typing import Dict, List

from runner.options.decorator_value import value, _to_str, _to_int


class GroupsOptions:
    __DEFAULT_GROUPS = 1
    __DEFAULT_GROUP_NUMBER = 1

    def __str__(self) -> str:
        return _to_str(self, 2)

    def to_dict(self) -> Dict[str, object]:
        return {
            "quantity": self.quantity,
            "number": self.number,
            "chapters": self.chapters,
            "chapters-file": self.chapters_file
        }

    @cached_property
    @value(yaml_path="test-lists.groups.quantity", cli_name="groups", cast_to_type=_to_int)
    def quantity(self) -> int:
        return GroupsOptions.__DEFAULT_GROUPS

    @cached_property
    @value(yaml_path="test-lists.groups.number", cli_name="group_number", cast_to_type=_to_int)
    def number(self) -> int:
        return GroupsOptions.__DEFAULT_GROUP_NUMBER

    def get_command_line(self) -> str:
        options = [
            f'--group={self.quantity}' if self.quantity != GroupsOptions.__DEFAULT_GROUPS else '',
            f'--group-number={self.number}' if self.number != GroupsOptions.__DEFAULT_GROUP_NUMBER else ''
        ]
        return ' '.join(options)

    @cached_property
    @value(yaml_path="test-lists.groups.chapters", cli_name="chapters")
    def chapters(self) -> List[str]:
        return []

    @cached_property
    @value(yaml_path="test-lists.groups.chapters-file", cli_name="chapters_file")
    def chapters_file(self) -> str:
        return "chapters.yaml"
