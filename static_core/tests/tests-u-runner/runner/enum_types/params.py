from dataclasses import dataclass
from typing import Any, List, Set, Optional

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.options.config import Config
from runner.reports.report_format import ReportFormat
from runner.code_coverage.coverage import LlvmCov
from runner.plugins.work_dir import WorkDir


@dataclass
class TestEnv:
    config: Config

    conf_kind: ConfigurationKind

    cmd_prefix: List[str]
    cmd_env: Any

    coverage: LlvmCov

    # Path and parameters for es2panda binary
    es2panda: str
    es2panda_args: List[str]

    # Path and parameters for ark binary
    runtime: str
    runtime_args: List[str]

    # Path and parameters for ark_aot binary
    ark_aot: Optional[str]
    aot_args: List[str]

    # Path and parameters for ark_quick binary
    ark_quick: str
    quick_args: List[str]

    timestamp: int
    report_formats: Set[ReportFormat]
    work_dir: WorkDir

    # Path and parameters for verifier binary
    verifier: str
    verifier_args: List[str]

    util: Any = None


@dataclass(frozen=True)
class Params:
    timeout: int
    executor: str
    fail_kind_fail: FailKind
    fail_kind_timeout: FailKind
    fail_kind_other: FailKind
    flags: List[str]
    env: Any


@dataclass(frozen=True)
class TestReport:
    output: str
    error: str
    return_code: int
