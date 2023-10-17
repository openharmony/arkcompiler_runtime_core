from dataclasses import dataclass
from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_int, _to_str, _to_path


@dataclass
class Es2PandaOptions:
    def __str__(self) -> str:
        return _to_str(self, Es2PandaOptions, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "timeout": self.timeout,
            "opt-level": self.opt_level,
            "custom-path": self.custom_path,
            "arktsconfig": self.arktsconfig,
        }

    @cached_property
    @value(yaml_path="es2panda.timeout", cli_name="es2panda_timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return 60

    @cached_property
    @value(yaml_path="es2panda.opt-level", cli_name="es2panda_opt_level", cast_to_type=_to_int)
    def opt_level(self) -> int:
        return 2

    @cached_property
    @value(yaml_path="es2panda.custom-path", cli_name="custom_es2panda_path", cast_to_type=_to_path)
    def custom_path(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="es2panda.arktsconfig", cli_name="arktsconfig", cast_to_type=_to_path)
    def arktsconfig(self) -> Optional[str]:
        return None
