import logging
from typing import Dict, Optional, List, Union, Any

import yaml

from runner import utils
from runner.logger import Log

_LOGGER = logging.getLogger("runner.options.yaml_document")


class YamlDocument:
    _document: Optional[Dict[str, Any]] = None

    @staticmethod
    def setup(config_path: Optional[str]):
        YamlDocument._document = YamlDocument.load(config_path)

    @staticmethod
    def document() -> Optional[Dict[str, Any]]:
        return YamlDocument._document

    @staticmethod
    def load(config_path: Optional[str]) -> Dict[str, Any]:
        config_from_file = {}
        if config_path is not None:
            with open(config_path, "r", encoding="utf-8") as stream:
                try:
                    config_from_file = yaml.safe_load(stream)
                except yaml.YAMLError as exc:
                    Log.exception_and_raise(_LOGGER, str(exc), yaml.YAMLError)
        return config_from_file

    @staticmethod
    def save(config_path: str, data: Dict[str, Any]):
        data_to_save = yaml.dump(data, indent=4)
        utils.write_2_file(config_path, data_to_save)

    @staticmethod
    def get_value_by_path(yaml_path: str) -> Optional[Union[int, bool, str, List[str]]]:
        yaml_parts = yaml_path.split(".")
        current: Optional[Dict[str, Any]] = YamlDocument._document
        for part in yaml_parts:
            if current and isinstance(current, dict) and part in current.keys():
                current = current.get(part)
            else:
                return None
        if current is None or isinstance(current, (bool, int, list, str)):
            return current

        Log.exception_and_raise(_LOGGER, f"Unsupported value type '{type(current)}' for '{yaml_path}'")
