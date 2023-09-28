from pathlib import Path
from typing import Any, Dict

from runner.plugins.ets.utils.test_parameters import parse_yaml

YAML_EXTENSIONS = ".yaml"
PARAM_SUFFIX = ".params"


class Params:
    def __init__(self, input_path: Path, bench_name: str) -> None:
        param_name = f"{bench_name}{PARAM_SUFFIX}{YAML_EXTENSIONS}"
        self.__param_path = input_path.parent / param_name

    def generate(self) -> Dict[str, Any]:
        if not self.__param_path.exists():
            return {}
        params: Dict[str, Any] = parse_yaml(str(self.__param_path))
        return params
