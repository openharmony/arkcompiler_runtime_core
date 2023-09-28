from functools import cached_property
from pathlib import Path
from runner.options.options_general import GeneralOptions
from runner.code_coverage.coverage_dir import CoverageDir


class WorkDir:
    def __init__(self, general: GeneralOptions, default_work_dir: Path):
        self.__general = general
        self.__default_work_dir = default_work_dir

    @cached_property
    def root(self) -> Path:
        root = Path(self.__general.work_dir) if self.__general.work_dir else self.__default_work_dir
        root.mkdir(parents=True, exist_ok=True)
        return root

    @property
    def report(self) -> Path:
        return self.root / "report"

    @property
    def gen(self) -> Path:
        return self.root / "gen"

    @property
    def intermediate(self) -> Path:
        return self.root / "intermediate"

    @cached_property
    def coverage_dir(self) -> CoverageDir:
        return CoverageDir(self.__general, self.root)
