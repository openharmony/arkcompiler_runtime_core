from dataclasses import dataclass
from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_str, _to_dir


@dataclass
class CoverageOptions:
    def __str__(self) -> str:
        return _to_str(self, CoverageOptions, 2)

    def to_dict(self) -> Dict[str, object]:
        return {
            "use-llvm-cov": self.use_llvm_cov,
            "llvm-cov-profdata-out-path": self.llvm_profdata_out_path,
            "llvm-cov-html-out-path": self.llvm_cov_html_out_path,
        }

    @cached_property
    @value(
        yaml_path="general.coverage.use-llvm-cov",
        cli_name="use_llvm_cov",
        cast_to_type=_to_bool
    )
    def use_llvm_cov(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="general.coverage.llvm-cov-profdata-out-path",
        cli_name="llvm_profdata_out_path",
        cast_to_type=_to_dir
    )
    def llvm_profdata_out_path(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="general.coverage.llvm-cov-html-out-path",
        cli_name="llvm_cov_html_out_path",
        cast_to_type=_to_dir
    )
    def llvm_cov_html_out_path(self) -> Optional[str]:
        return None
