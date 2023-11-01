from functools import cached_property
from pathlib import Path
from typing import Optional


class EtsTestDir:
    def __init__(self, static_core_root: str, root: Optional[str] = None) -> None:
        self.__static_core_root = Path(static_core_root)
        self.__root = root

    @cached_property
    def root(self) -> Path:
        return Path(self.__root) if self.__root else self.__static_core_root / "plugins" / "ets"

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
