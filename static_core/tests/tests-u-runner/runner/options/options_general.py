from dataclasses import dataclass
from functools import cached_property
from typing import Dict, Optional

from runner.enum_types.qemu import QemuKind
from runner.enum_types.verbose_format import VerboseKind, VerboseFilter
from runner.options.decorator_value import value, _to_qemu, _to_bool, _to_enum, _to_str, _to_path, _to_int, \
    _to_processes
from runner.options.options_coverage import CoverageOptions
from runner.reports.report_format import ReportFormat


@dataclass
class GeneralOptions:
    def __str__(self) -> str:
        return _to_str(self, GeneralOptions, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "build": self.build,
            "processes": self.processes,
            "test-root": self.test_root,
            "list-root": self.list_root,
            "work-dir": self.work_dir,
            "ets-stdlib-root": self.ets_stdlib_root,
            "show-progress": self.show_progress,
            "gc_type": self.gc_type,
            "full-gc-bombing-frequency": self.full_gc_bombing_frequency,
            "run_gc_in_place": self.run_gc_in_place,
            "heap-verifier": self.heap_verifier,
            "report-format": self.report_format.value.upper(),
            "verbose": self.verbose.value.upper(),
            "verbose-filter": self.verbose_filter.value.upper(),
            "coverage": self.coverage.to_dict(),
            "force-download": self.force_download,
            "bco": self.bco,
            "qemu": self.qemu.value.upper(),
        }

    @cached_property
    @value(
        yaml_path="general.generate-config",
        cli_name="generate_config",
        cast_to_type=_to_path
    )
    def generate_config(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.processes", cli_name="processes", cast_to_type=_to_processes)
    def processes(self) -> int:
        return 1

    @cached_property
    def chunksize(self) -> int:
        return 32

    @cached_property
    @value(yaml_path="general.build", cli_name="build_dir", cast_to_type=_to_path, required=True)
    def build(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.test-root", cli_name="test_root", cast_to_type=_to_path)
    def test_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.list-root", cli_name="list_root", cast_to_type=_to_path)
    def list_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.work-dir", cli_name="work_dir", cast_to_type=_to_path)
    def work_dir(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.ets-stdlib-root", cli_name="ets_stdlib_root", cast_to_type=_to_path)
    def ets_stdlib_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.show-progress", cli_name="progress", cast_to_type=_to_bool)
    def show_progress(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.gc_type", cli_name="gc_type")
    def gc_type(self) -> str:
        return "g1-gc"

    @cached_property
    @value(yaml_path="general.full-gc-bombing-frequency", cli_name="full_gc_bombing_frequency", cast_to_type=_to_int)
    def full_gc_bombing_frequency(self) -> int:
        return 0

    @cached_property
    @value(yaml_path="general.run_gc_in_place", cli_name="run_gc_in_place", cast_to_type=_to_bool)
    def run_gc_in_place(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.heap-verifier", cli_name="heap_verifier")
    def heap_verifier(self) -> str:
        return "fail_on_verification"

    @cached_property
    @value(
        yaml_path="general.report-format",
        cli_name="report_formats",
        cast_to_type=lambda x: _to_enum(x, ReportFormat)
    )
    def report_format(self) -> ReportFormat:
        return ReportFormat.LOG

    @cached_property
    @value(
        yaml_path="general.verbose",
        cli_name="verbose",
        cast_to_type=lambda x: _to_enum(x, VerboseKind)
    )
    def verbose(self) -> VerboseKind:
        return VerboseKind.NONE

    @cached_property
    @value(
        yaml_path="general.verbose-filter",
        cli_name="verbose_filter",
        cast_to_type=lambda x: _to_enum(x, VerboseFilter)
    )
    def verbose_filter(self) -> VerboseFilter:
        return VerboseFilter.NEW_FAILURES

    coverage = CoverageOptions()

    @cached_property
    @value(yaml_path="general.force-download", cli_name="force_download", cast_to_type=_to_bool)
    def force_download(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.bco", cli_name="bco", cast_to_type=_to_bool)
    def bco(self) -> bool:
        return True

    @cached_property
    @value(yaml_path="general.with_js", cli_name="with_js", cast_to_type=_to_bool)
    def with_js(self) -> bool:
        return True

    @cached_property
    @value(yaml_path="general.qemu", cli_name=["arm64_qemu", "arm32_qemu"], cast_to_type=_to_qemu)
    def qemu(self) -> QemuKind:
        return QemuKind.NONE
