import logging
from pathlib import Path
from typing import Optional

_LOGGER = logging.getLogger("runner.plugins.ets.es2panda_test_dir")


class RuntimeDefaultEtsTestDir:
    def __init__(self, panda_source_root: str, root: Optional[str] = None) -> None:
        self.__panda_source_root = Path(panda_source_root)
        self.__root = root

    @property
    def root(self) -> Path:
        return Path(self.__root) if self.__root else self.runtime_ets

    @property
    def es2panda_test(self) -> Path:
        return self.__panda_source_root / "plugins" / "ecmascript" / "es2panda" / "test"

    @property
    def runtime_ets(self) -> Path:
        return self.es2panda_test / "runtime" / "ets"
