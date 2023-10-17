from dataclasses import dataclass


@dataclass
class Summary:
    name: str
    total: int
    passed: int
    failed: int
    ignored: int
    ignored_but_passed: int
    excluded: int
    excluded_after: int
