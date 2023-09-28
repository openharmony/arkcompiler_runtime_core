import logging
import multiprocessing
import os
from enum import Enum
from os import path
from typing import TypeVar, Union, List, Tuple, Any, Callable, Optional, Set, Type

from runner import utils
from runner.enum_types.qemu import QemuKind
from runner.logger import Log
from runner.options.cli_args_wrapper import CliArgsWrapper
from runner.options.yaml_document import YamlDocument

T = TypeVar("T")
CliOptionType = Union[str, List[str]]
CliValueType = Optional[Any]
CastToTypeFunction = Callable[[Any], Any]

_LOGGER = logging.getLogger("runner.options.decorator_value")


def value(
        *,
        yaml_path: str,
        cli_name: CliOptionType,
        required: bool = False,
        cast_to_type: Optional[CastToTypeFunction] = None
):
    def decorator(func):
        def decorated(*args, **kwargs):
            cli = _process_cli_option(cli_name, cast_to_type)
            if cli is not None:
                return cli
            yaml = YamlDocument.get_value_by_path(yaml_path)
            yaml = cast_to_type(yaml) if cast_to_type is not None and yaml is not None else yaml
            if yaml is not None:
                return yaml
            default_value = func(*args, **kwargs)
            if default_value is None and required:
                Log.exception_and_raise(
                    _LOGGER,
                    f"Missed required property. "
                    f"Expected either '{yaml_path}' in the config file "
                    f"or '{cli_name}' cli option(s)"
                )
            return default_value

        return decorated

    return decorator


def _process_cli_option(
        cli_name: CliOptionType,
        cast_to_type: Optional[CastToTypeFunction] = None
) -> CliValueType:
    cli = None
    if isinstance(cli_name, str):
        cli = CliArgsWrapper.get_by_name(cli_name)
        cli = cast_to_type(cli) if cast_to_type is not None and cli is not None else cli
    elif isinstance(cli_name, list):
        clis = [(n, CliArgsWrapper.get_by_name(n)) for n in cli_name]
        if cast_to_type is None:
            Log.exception_and_raise(
                _LOGGER,
                f"Cannot convert {cli_name} from command line. Provide 'cast_to_type' parameter"
            )
        cli = cast_to_type(clis)
    else:
        Log.exception_and_raise(_LOGGER, f"Cannot process CLI names {cli_name}")
    return cli


def _to_qemu(names: Union[str, List[Tuple[str, bool]], None]) -> Optional[QemuKind]:
    if names is None:
        return None
    if isinstance(names, str):
        return utils.enum_from_str(names, QemuKind)
    result = [n for n in names if n[1] is not None]
    if len(result) == 0:
        return None
    if result[0][0] == "arm64_qemu":
        return QemuKind.ARM64
    if result[0][0] == "arm32_qemu":
        return QemuKind.ARM32
    return None


TestSuiteFromCliValue = Optional[Union[bool, List[str]]]
TestSuiteFromCliKey = str
TestSuitesFromCliTuple = Tuple[TestSuiteFromCliKey, TestSuiteFromCliValue]


def _to_test_suites(names: Optional[List[Union[str, TestSuitesFromCliTuple]]]) -> Optional[Set[str]]:
    if names is None:
        return None
    suites: Set[str] = set([])
    for name in names:
        if isinstance(name, str):
            # from yaml: simple list of strings
            suites.add(name)
        else:
            # from cli: name has type TestSuitesFromCliTuple
            name_key: TestSuiteFromCliKey = name[0]
            name_value: TestSuiteFromCliValue = name[1]
            if name_value is not None and isinstance(name_value, list):
                suites.update(name_value)
            elif name_value:
                suites.add(name_key)
    return suites if len(suites) > 0 else None


def _to_bool(cli_value: Union[str, bool, None]) -> Optional[bool]:
    if cli_value is None:
        return None
    if isinstance(cli_value, bool):
        return cli_value
    return cli_value.lower() == "true"


def _to_int(cli_value: Union[str, int, None]) -> Optional[int]:
    if cli_value is None:
        return None
    if isinstance(cli_value, int):
        return cli_value
    return int(cli_value)


def _to_processes(cli_value: Union[str, int, None]) -> Optional[int]:
    if cli_value is None:
        return None
    if isinstance(cli_value, str) and cli_value.lower() == "all":
        return multiprocessing.cpu_count()
    return _to_int(cli_value)


def _to_jit_preheats(cli_value: Union[str, int, None], *, prop: str, default_if_empty: int) -> Optional[int]:
    if cli_value is None:
        return None
    if isinstance(cli_value, int):
        return cli_value
    if cli_value == "":
        return default_if_empty
    parts = [p.strip() for p in cli_value.split(",") if p.strip().startswith(prop)]
    if len(parts) == 0:
        return None
    prop_parts = [p.strip() for p in parts[0].split("=")]
    if len(prop_parts) != 2:
        Log.exception_and_raise(_LOGGER, f"Incorrect value for option --jit-preheat-repeats '{cli_value}'")
    return int(prop_parts[1])


def _to_time_edges(cli_value: Union[str, List[int], None]) -> Optional[List[int]]:
    if cli_value is None:
        return None
    if isinstance(cli_value, str):
        return [int(n.strip()) for n in cli_value.split(",")]
    return cli_value


EnumClass = TypeVar("EnumClass", bound=Enum)


def _to_enum(cli_value: Union[str, EnumClass, None], enum_cls: Type[EnumClass]) -> Optional[EnumClass]:
    if isinstance(cli_value, str):
        return utils.enum_from_str(cli_value, enum_cls)
    return cli_value


def _to_str(obj: T, obj_cls: Type[T], indent: int = 0) -> str:
    attrs = dir(obj)
    attrs = [n for n in attrs if not n.startswith("_") and not utils.is_type_of(getattr(obj, n), "method")]
    result = [f"{obj_cls.__name__}"]
    indent_str = "\n" + "\t" * (indent + 1)
    for attr in attrs:
        result.append(f"{indent_str}{attr}: {getattr(obj, attr)}")
    return "".join(result)


def _to_path(cli_value: Optional[str]) -> Optional[str]:
    if cli_value is None:
        return None
    return path.abspath(path.expanduser(cli_value))


def _to_dir(cli_value: Optional[str]) -> Optional[str]:
    _path = _to_path(cli_value)
    if _path is None:
        return None
    os.makedirs(_path, exist_ok=True)
    return _path
