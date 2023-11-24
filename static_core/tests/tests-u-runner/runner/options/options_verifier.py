from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_int, _to_str, _to_path


class VerifierOptions:
    __DEFAULT_TIMEOUT = 60

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "timeout": self.timeout,
            "config": self.config,
        }

    @cached_property
    @value(yaml_path="verifier.enable", cli_name="enable_verifier", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return True

    @cached_property
    @value(yaml_path="verifier.timeout", cli_name="verifier_timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return VerifierOptions.__DEFAULT_TIMEOUT

    @cached_property
    @value(yaml_path="verifier.config", cli_name="verifier_config", cast_to_type=_to_path)
    def config(self) -> Optional[str]:
        return None

    def get_command_line(self) -> str:
        options = [
            '--disable-verifier' if not self.enable else '',
            f'--verifier-timeout={self.timeout}' if self.timeout != VerifierOptions.__DEFAULT_TIMEOUT else '',
            f'--verifier-config="{self.config}"' if self.config is not None else ''
        ]
        return ' '.join(options)
