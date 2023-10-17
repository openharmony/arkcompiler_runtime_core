from dataclasses import dataclass
from functools import cached_property
from typing import Dict, List

from runner.options.decorator_value import value, _to_bool, _to_time_edges, _to_str


@dataclass
class TimeReportOptions:
    def __str__(self) -> str:
        return _to_str(self, TimeReportOptions, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "time-edges": self.time_edges,
        }

    @cached_property
    @value(yaml_path="time-report.enable", cli_name="time_report", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="time-report.time-edges", cli_name="time_edges", cast_to_type=_to_time_edges)
    def time_edges(self) -> List[int]:
        return [1, 5, 10]
