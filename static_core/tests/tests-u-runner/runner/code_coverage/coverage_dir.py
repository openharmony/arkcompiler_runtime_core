from functools import cached_property
from pathlib import Path
from runner.options.options_general import GeneralOptions


class CoverageDir:
    def __init__(self, general: GeneralOptions, work_dir: Path):
        self.__general = general
        self.__work_dir = work_dir

    @cached_property
    def root(self) -> Path:
        root = self.__work_dir / "coverage"
        root.mkdir(parents=True, exist_ok=True)
        return root

    @property
    def html_report_dir(self):
        if self.__general.coverage.llvm_cov_html_out_path:
            html_out_path = Path(self.__general.coverage.llvm_cov_html_out_path)
        else:
            html_out_path = self.root / "html_report_dir"
        html_out_path.mkdir(parents=True, exist_ok=True)
        return html_out_path

    @cached_property
    def profdata_dir(self):
        if self.__general.coverage.llvm_profdata_out_path:
            profdata_out_path = Path(self.__general.coverage.llvm_profdata_out_path)
        else:
            profdata_out_path = self.root / "profdata_dir"
        profdata_out_path.mkdir(parents=True, exist_ok=True)
        return profdata_out_path

    @cached_property
    def coverage_work_dir(self):
        work_dir = self.root / "work_dir"
        work_dir.mkdir(parents=True, exist_ok=True)
        return work_dir

    @property
    def info_file(self):
        return self.coverage_work_dir / "coverage_lcov_format.info"

    @property
    def profdata_files_list_file(self):
        return self.coverage_work_dir / "profdatalist.txt"

    @property
    def profdata_merged_file(self):
        return self.coverage_work_dir / "merged.profdata"
