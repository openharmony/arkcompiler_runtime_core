from enum import Enum


class VerboseKind(Enum):
    ALL = "all"
    SHORT = "short"
    NONE = ""


class VerboseFilter(Enum):
    ALL = "all"
    IGNORED = "ignored"
    NEW_FAILURES = "new"
