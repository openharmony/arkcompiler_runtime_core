from enum import Enum


class ConfigurationKind(Enum):
    INT = "INT"
    OTHER_INT = "INT-OTHER"
    AOT = "AOT"
    AOT_FULL = "AOT-FULL"
    JIT = "JIT"
    QUICK = "QUICK"


class ArchitectureKind(Enum):
    ARM32 = "ARM32"
    ARM64 = "ARM64"
    AMD32 = "AMD32"
    AMD64 = "AMD64"  # default value


class SanitizerKind(Enum):
    NONE = ""
    ASAN = "ASAN"
    TSAN = "TSAN"
