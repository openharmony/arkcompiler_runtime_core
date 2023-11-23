import logging
from pathlib import Path
from typing import Optional

_LOGGER = logging.getLogger("runner.plugins.ets.es2panda_test_dir")


class RuntimeDefaultEtsTestDir:
    def __init__(self, static_core_root: str, root: Optional[str] = None) -> None:
        self.__static_core_root = Path(static_core_root)
        self.__root = root

    @property
    def root(self) -> Path:
        return Path(self.__root) if self.__root else self.runtime_ets

    @property
    def es2panda_test(self) -> Path:
        return self.__static_core_root / "tools" / "es2panda" / "test"

    @property
    def runtime_ets(self) -> Path:
        return self.es2panda_test / "runtime" / "ets"
