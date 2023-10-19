from functools import cached_property
from pathlib import Path
from typing import Optional


class EtsTestDir:
    def __init__(self, panda_source_root: str, root: Optional[str] = None) -> None:
        self.__panda_source_root = Path(panda_source_root)
        self.__root = root

    @cached_property
    def root(self) -> Path:
        return Path(self.__root) if self.__root else self.__panda_source_root / "plugins" / "ets"

    @property
    def tests(self) -> Path:
        return self.root / "tests"

    @property
    def ets_templates(self) -> Path:
        return self.tests / "ets-templates"

    @property
    def ets_func_tests(self) -> Path:
        return self.tests / "ets_func_tests"

    @property
    def stdlib_templates(self) -> Path:
        return self.tests / "stdlib-templates"

    @property
    def gc_stress(self) -> Path:
        return self.tests / "ets_test_suite" / "gc" / "stress"
