#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2026 Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import json
import shlex
import subprocess
import sys
from dataclasses import asdict, dataclass
from json import JSONDecodeError
from typing import Any


DEFAULT_HDC_PATH = "hdc"
CPU_SYSFS_ROOT = "/sys/devices/system/cpu"


class InputParseError(Exception):
    """Raised when the input configuration cannot be parsed."""


class ValidationError(Exception):
    """Raised when a parsed CPU request is not valid for execution."""


@dataclass(frozen=True)
class DeviceConfig:
    serial: str | None
    connection: str


@dataclass(frozen=True)
class CpuClusterRequest:
    name: str
    cores: list[int]
    online: bool | None
    governor: str | None
    min_freq: int | None
    max_freq: int | None


@dataclass(frozen=True)
class CpuCoreRequest:
    core: int
    online: bool | None
    governor: str | None
    min_freq: int | None
    max_freq: int | None


@dataclass(frozen=True)
class CpuRequest:
    device: DeviceConfig
    clusters: list[CpuClusterRequest]
    cores: list[CpuCoreRequest]
    dry_run: bool
    rollback_on_error: bool
    disable_freq_autoscalling: bool = False


@dataclass(frozen=True)
class CpuValidationPlan:
    request: CpuRequest
    touched_cores: list[int]


@dataclass(frozen=True)
class CpuWriteOperation:
    core: int
    path: str
    value: str


@dataclass(frozen=True)
class DeviceFeatureOperation:
    name: str
    command: list[str]
    applied: bool
    message: str | None = None


@dataclass(frozen=True)
class CpuApplyResult:
    dry_run: bool
    operations: list[CpuWriteOperation]
    applied_operations: list[CpuWriteOperation]
    rolled_back_operations: list[CpuWriteOperation]
    request: CpuRequest
    feature_operations: list[DeviceFeatureOperation] | None = None
    messages: list[str] | None = None


class JsonInputParser:
    """Parses a JSON configuration into a CPU request model."""

    def __init__(self, source_name: str):
        self.source_name = source_name

    def parse_text(self, raw_json: str) -> CpuRequest:
        data = self._load_json(raw_json)
        return self._parse_root(data)

    def _load_json(self, raw_json: str) -> dict[str, Any]:
        try:
            data = json.loads(raw_json)
        except JSONDecodeError as error:
            message = f"Invalid JSON in '{self.source_name}': line {error.lineno} column {error.colno}: {error.msg}"
            raise InputParseError(message) from error

        if not isinstance(data, dict):
            raise InputParseError("Root JSON value must be an object")

        return data

    def _parse_root(self, data: dict[str, Any]) -> CpuRequest:
        options = self._optional_object(data, "options")
        disable_freq_autoscalling = self._optional_bool(options, "disable_freq_autoscalling", False)

        if "cpu" in data:
            cpu = self._required_object(data, "cpu")
        elif disable_freq_autoscalling:
            cpu = {}
        else:
            raise InputParseError("Missing required field: cpu")

        clusters = self._parse_clusters(cpu)
        cores = self._parse_individual_cores(cpu)

        if not clusters and not cores and not disable_freq_autoscalling:
            raise InputParseError("Field cpu must contain at least one of: clusters, cores")

        return CpuRequest(
            device=self._parse_device(data),
            clusters=clusters,
            cores=cores,
            dry_run=self._optional_bool(options, "dry_run", True),
            rollback_on_error=self._optional_bool(options, "rollback_on_error", True),
            disable_freq_autoscalling=disable_freq_autoscalling,
        )

    def _parse_device(self, data: dict[str, Any]) -> DeviceConfig:
        device = self._optional_object(data, "device")
        serial = self._optional_string(device, "serial", None)
        connection = self._optional_string(device, "connection", "hdc")

        return DeviceConfig(serial=serial, connection=connection)

    def _parse_clusters(self, cpu: dict[str, Any]) -> list[CpuClusterRequest]:
        raw_clusters = self._optional_list(cpu, "clusters", "cpu.clusters")
        clusters = []

        for index, raw_cluster in enumerate(raw_clusters):
            path = f"cpu.clusters[{index}]"
            if not isinstance(raw_cluster, dict):
                raise InputParseError(f"Field {path} must be an object")

            clusters.append(
                CpuClusterRequest(
                    name=self._optional_string(raw_cluster, "name", f"cluster_{index}", path),
                    cores=self._parse_cores(raw_cluster, path),
                    online=self._optional_bool(raw_cluster, "online", None, path),
                    governor=self._optional_string(raw_cluster, "governor", None, path),
                    min_freq=self._optional_int(raw_cluster, "min_freq", None, path),
                    max_freq=self._optional_int(raw_cluster, "max_freq", None, path),
                )
            )

        return clusters

    def _parse_individual_cores(self, cpu: dict[str, Any]) -> list[CpuCoreRequest]:
        raw_cores = self._optional_list(cpu, "cores", "cpu.cores")
        cores = []

        for index, raw_core in enumerate(raw_cores):
            path = f"cpu.cores[{index}]"
            if not isinstance(raw_core, dict):
                raise InputParseError(f"Field {path} must be an object")

            cores.append(
                CpuCoreRequest(
                    core=self._required_int(raw_core, "core", f"{path}.core"),
                    online=self._optional_bool(raw_core, "online", None, path),
                    governor=self._optional_string(raw_core, "governor", None, path),
                    min_freq=self._optional_int(raw_core, "min_freq", None, path),
                    max_freq=self._optional_int(raw_core, "max_freq", None, path),
                )
            )

        return cores

    def _parse_cores(self, cluster: dict[str, Any], cluster_path: str) -> list[int]:
        raw_cores = self._required_list(cluster, "cores", f"{cluster_path}.cores")

        for index, core in enumerate(raw_cores):
            if not isinstance(core, int) or isinstance(core, bool):
                raise InputParseError(f"Field {cluster_path}.cores[{index}] must be an integer")

        return raw_cores

    def _required_object(self, data: dict[str, Any], key: str) -> dict[str, Any]:
        if key not in data:
            raise InputParseError(f"Missing required field: {key}")

        value = data[key]
        if not isinstance(value, dict):
            raise InputParseError(f"Field {key} must be an object")

        return value

    def _optional_object(self, data: dict[str, Any], key: str) -> dict[str, Any]:
        if key not in data:
            return {}

        value = data[key]
        if not isinstance(value, dict):
            raise InputParseError(f"Field {key} must be an object")

        return value

    def _required_list(self, data: dict[str, Any], key: str, path: str) -> list[Any]:
        if key not in data:
            raise InputParseError(f"Missing required field: {path}")

        return self._parse_list(data, key, path)

    def _optional_list(self, data: dict[str, Any], key: str, path: str) -> list[Any]:
        if key not in data:
            return []

        return self._parse_list(data, key, path)

    def _parse_list(self, data: dict[str, Any], key: str, path: str) -> list[Any]:
        value = data[key]
        if not isinstance(value, list):
            raise InputParseError(f"Field {path} must be a list")

        return value

    def _required_int(self, data: dict[str, Any], key: str, path: str) -> int:
        if key not in data:
            raise InputParseError(f"Missing required field: {path}")

        value = data[key]
        if not isinstance(value, int) or isinstance(value, bool):
            raise InputParseError(f"Field {path} must be an integer")

        return value

    def _optional_bool(
        self, data: dict[str, Any], key: str, default: bool | None, parent_path: str | None = None
    ) -> bool | None:
        return self._optional_typed_value(data, key, default, bool, "boolean", parent_path)

    def _optional_int(
        self, data: dict[str, Any], key: str, default: int | None, parent_path: str | None = None
    ) -> int | None:
        if key not in data:
            return default

        value = data[key]
        if value is None and default is None:
            return value

        if not isinstance(value, int) or isinstance(value, bool):
            raise InputParseError(f"Field {self._json_path(key, parent_path)} must be an integer")

        return value

    def _optional_string(
        self, data: dict[str, Any], key: str, default: str | None, parent_path: str | None = None
    ) -> str | None:
        return self._optional_typed_value(data, key, default, str, "string", parent_path)

    def _optional_typed_value(
        self,
        data: dict[str, Any],
        key: str,
        default: Any,
        expected_type: type,
        type_name: str,
        parent_path: str | None,
    ) -> Any:
        if key not in data:
            return default

        value = data[key]
        if value is None and default is None:
            return value

        if not isinstance(value, expected_type):
            raise InputParseError(f"Field {self._json_path(key, parent_path)} must be a {type_name}")

        return value

    def _json_path(self, key: str, parent_path: str | None) -> str:
        if parent_path is None:
            return key

        return f"{parent_path}.{key}"


class DeviceShell:
    """Device shell interface used by validators and appliers."""

    def read(self, path: str) -> str:
        raise NotImplementedError

    def write(self, path: str, value: str) -> None:
        raise NotImplementedError

    def exists(self, path: str) -> bool:
        raise NotImplementedError

    def target_mount(self) -> None:
        raise NotImplementedError

    def shell_script(self, script: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        raise NotImplementedError


class HdcDeviceShell(DeviceShell):
    """Read-only shell adapter for OpenHarmony hdc."""

    def __init__(self, serial: str | None = None, hdc_path: str = DEFAULT_HDC_PATH):
        self.serial = serial
        self.hdc_path = hdc_path

    def read(self, path: str) -> str:
        return self._run_shell(["cat", path]).stdout

    def write(self, path: str, value: str) -> None:
        command = f"printf %s {shlex.quote(value)} > {shlex.quote(path)}"
        self._run_shell(["sh", "-c", command])

    def exists(self, path: str) -> bool:
        result = self._run_shell(["test", "-e", path], check=False)
        return result.returncode == 0

    def target_mount(self) -> None:
        command = [self.hdc_path]
        if self.serial is not None:
            command.extend(["-t", self.serial])
        command.extend(["target", "mount"])

        try:
            result = subprocess.run(command, capture_output=True, check=False, encoding="utf-8")
        except OSError as error:
            raise ValidationError(f"Cannot run hdc: {error}") from error

        if result.returncode != 0:
            stderr = result.stderr.strip()
            detail = f": {stderr}" if stderr else ""
            raise ValidationError(f"hdc target mount failed{detail}")

    def shell_script(self, script: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        return self._run_shell(["sh", "-c", script], check=check)

    def _run_shell(self, command: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
        hdc_command = [self.hdc_path]
        if self.serial is not None:
            hdc_command.extend(["-t", self.serial])
        hdc_command.extend(["shell", *command])

        try:
            result = subprocess.run(hdc_command, capture_output=True, check=False, encoding="utf-8")
        except OSError as error:
            raise ValidationError(f"Cannot run hdc: {error}") from error

        if check and result.returncode != 0:
            stderr = result.stderr.strip()
            detail = f": {stderr}" if stderr else ""
            raise ValidationError(f"hdc shell command failed for {' '.join(command)}{detail}")

        return result


class CpuRequestValidator:
    """Validates parsed CPU requests before any device state is changed."""

    def __init__(self, shell: DeviceShell | None = None):
        self.shell = shell

    def validate(self, request: CpuRequest) -> CpuValidationPlan:
        self._validate_static_rules(request)
        touched_cores = self._collect_touched_cores(request)

        if self.shell is not None:
            self._validate_device_rules(request, touched_cores)

        return CpuValidationPlan(request=request, touched_cores=touched_cores)

    def _validate_static_rules(self, request: CpuRequest) -> None:
        individual_cores = set()

        for cluster_index, cluster in enumerate(request.clusters):
            cluster_path = f"cpu.clusters[{cluster_index}]"
            if not cluster.cores:
                raise ValidationError(f"Field {cluster_path}.cores must not be empty")

            seen_cluster_cores = set()
            for core in cluster.cores:
                if core < 0:
                    raise ValidationError(f"Field {cluster_path}.cores contains negative core: {core}")
                if core in seen_cluster_cores:
                    raise ValidationError(f"Field {cluster_path}.cores contains duplicate core: {core}")
                seen_cluster_cores.add(core)

            self._validate_action_fields(cluster, cluster_path)

        for core_index, core_request in enumerate(request.cores):
            core_path = f"cpu.cores[{core_index}]"
            if core_request.core < 0:
                raise ValidationError(f"Field {core_path}.core must be non-negative")
            if core_request.core in individual_cores:
                raise ValidationError(f"Duplicate individual core action for core {core_request.core}")
            individual_cores.add(core_request.core)

            self._validate_action_fields(core_request, core_path)

        self._validate_cluster_core_conflicts(request)

    def _validate_action_fields(self, action: CpuClusterRequest | CpuCoreRequest, path: str) -> None:
        if action.min_freq is not None and action.min_freq <= 0:
            raise ValidationError(f"Field {path}.min_freq must be positive")
        if action.max_freq is not None and action.max_freq <= 0:
            raise ValidationError(f"Field {path}.max_freq must be positive")
        if action.min_freq is not None and action.max_freq is not None and action.min_freq > action.max_freq:
            raise ValidationError(f"Field {path}.min_freq must be less than or equal to {path}.max_freq")
        if all(
            value is None
            for value in (action.online, action.governor, action.min_freq, action.max_freq)
        ):
            raise ValidationError(f"Field {path} must contain at least one requested state change")

    def _validate_cluster_core_conflicts(self, request: CpuRequest) -> None:
        cluster_actions_by_core: dict[int, dict[str, Any]] = {}

        for cluster in request.clusters:
            action = self._action_values(cluster)
            for core in cluster.cores:
                cluster_actions_by_core.setdefault(core, {}).update(action)

        for core_request in request.cores:
            cluster_action = cluster_actions_by_core.get(core_request.core, {})
            core_action = self._action_values(core_request)
            for property_name, value in core_action.items():
                if property_name in cluster_action and cluster_action[property_name] != value:
                    raise ValidationError(
                        f"Conflicting {property_name} action for core {core_request.core}: "
                        f"cluster requests {cluster_action[property_name]!r}, individual core requests {value!r}"
                    )

    def _action_values(self, action: CpuClusterRequest | CpuCoreRequest) -> dict[str, Any]:
        values = {}
        for property_name in ("online", "governor", "min_freq", "max_freq"):
            value = getattr(action, property_name)
            if value is not None:
                values[property_name] = value
        return values

    def _collect_touched_cores(self, request: CpuRequest) -> list[int]:
        touched_cores = []
        seen = set()

        for cluster in request.clusters:
            for core in cluster.cores:
                if core not in seen:
                    touched_cores.append(core)
                    seen.add(core)

        for core_request in request.cores:
            if core_request.core not in seen:
                touched_cores.append(core_request.core)
                seen.add(core_request.core)

        return touched_cores

    def _validate_device_rules(self, request: CpuRequest, touched_cores: list[int]) -> None:
        for core in touched_cores:
            cpu_path = self._cpu_path(core)
            if not self.shell.exists(cpu_path):
                raise ValidationError(f"CPU core {core} does not exist at {cpu_path}")

        for action, cores in self._iter_actions_with_target_cores(request):
            for core in cores:
                self._validate_core_action_on_device(core, action)

    def _validate_core_action_on_device(self, core: int, action: CpuClusterRequest | CpuCoreRequest) -> None:
        if action.online is not None:
            online_path = f"{self._cpu_path(core)}/online"
            if not self.shell.exists(online_path):
                raise ValidationError(f"CPU core {core} does not support online control at {online_path}")

        if action.governor is not None or action.min_freq is not None or action.max_freq is not None:
            cpufreq_path = self._cpufreq_path(core)
            if not self.shell.exists(cpufreq_path):
                raise ValidationError(f"CPU core {core} does not expose cpufreq at {cpufreq_path}")

        if action.governor is not None:
            governors_path = f"{self._cpufreq_path(core)}/scaling_available_governors"
            governors = self._read_words(governors_path)
            if action.governor not in governors:
                raise ValidationError(f"Governor {action.governor!r} is not supported by CPU core {core}")

        if action.min_freq is not None:
            self._validate_frequency(core, action.min_freq, "min_freq")
        if action.max_freq is not None:
            self._validate_frequency(core, action.max_freq, "max_freq")

    def _validate_frequency(self, core: int, frequency: int, property_name: str) -> None:
        available_frequencies_path = f"{self._cpufreq_path(core)}/scaling_available_frequencies"
        if self.shell.exists(available_frequencies_path):
            available_frequencies = self._read_int_words(available_frequencies_path)
            if frequency not in available_frequencies:
                raise ValidationError(f"Field {property_name} frequency {frequency} is not supported by CPU core {core}")
            return

        min_frequency = self._read_int(f"{self._cpufreq_path(core)}/cpuinfo_min_freq")
        max_frequency = self._read_int(f"{self._cpufreq_path(core)}/cpuinfo_max_freq")
        if frequency < min_frequency or frequency > max_frequency:
            raise ValidationError(
                f"Field {property_name} frequency {frequency} is outside CPU core {core} range "
                f"{min_frequency}..{max_frequency}"
            )

    def _iter_actions_with_target_cores(self, request: CpuRequest):
        for cluster in request.clusters:
            yield cluster, cluster.cores
        for core_request in request.cores:
            yield core_request, [core_request.core]

    def _read_words(self, path: str) -> set[str]:
        return set(self.shell.read(path).split())

    def _read_int_words(self, path: str) -> set[int]:
        return {int(value) for value in self._read_words(path)}

    def _read_int(self, path: str) -> int:
        return int(self.shell.read(path).strip())

    def _cpu_path(self, core: int) -> str:
        return f"{CPU_SYSFS_ROOT}/cpu{core}"

    def _cpufreq_path(self, core: int) -> str:
        return f"{self._cpu_path(core)}/cpufreq"


class CpuRequestApplier:
    """Applies a validated CPU plan to the device sysfs state."""

    def __init__(self, shell: DeviceShell | None = None):
        self.shell = shell

    def apply(self, plan: CpuValidationPlan) -> CpuApplyResult:
        operations = self._build_operations(plan.request)
        if plan.request.dry_run:
            return CpuApplyResult(
                dry_run=True,
                operations=operations,
                applied_operations=[],
                rolled_back_operations=[],
                request=plan.request,
            )

        if self.shell is None:
            raise ValidationError("Device shell is required when dry_run is false")

        snapshot = self._snapshot_operations(operations)
        applied_operations = []
        rolled_back_operations = []

        try:
            for operation in operations:
                self.shell.write(operation.path, operation.value)
                applied_operations.append(operation)
        except Exception as error:
            if plan.request.rollback_on_error:
                rolled_back_operations = self._rollback(snapshot, applied_operations)
            raise ValidationError(f"Failed to apply CPU operation: {error}") from error

        return CpuApplyResult(
            dry_run=False,
            operations=operations,
            applied_operations=applied_operations,
            rolled_back_operations=rolled_back_operations,
            request=plan.request,
        )

    def _build_operations(self, request: CpuRequest) -> list[CpuWriteOperation]:
        operations = []

        for cluster in request.clusters:
            for core in cluster.cores:
                operations.extend(self._build_core_operations(core, cluster))

        for core_request in request.cores:
            operations.extend(self._build_core_operations(core_request.core, core_request))

        return operations

    def _build_core_operations(self, core: int, action: CpuClusterRequest | CpuCoreRequest) -> list[CpuWriteOperation]:
        operations = []

        if action.online is not None:
            operations.append(CpuWriteOperation(core, f"{self._cpu_path(core)}/online", "1" if action.online else "0"))
        if action.governor is not None:
            operations.append(CpuWriteOperation(core, f"{self._cpufreq_path(core)}/scaling_governor", action.governor))

        operations.extend(self._build_frequency_operations(core, action))
        return operations

    def _build_frequency_operations(self, core: int, action: CpuClusterRequest | CpuCoreRequest) -> list[CpuWriteOperation]:
        min_operation = self._frequency_operation(core, "scaling_min_freq", action.min_freq)
        max_operation = self._frequency_operation(core, "scaling_max_freq", action.max_freq)

        if min_operation is None and max_operation is None:
            return []
        if min_operation is None:
            return [max_operation]
        if max_operation is None:
            return [min_operation]

        if self.shell is None:
            return [min_operation, max_operation]

        current_max = int(self.shell.read(f"{self._cpufreq_path(core)}/scaling_max_freq").strip())
        current_min = int(self.shell.read(f"{self._cpufreq_path(core)}/scaling_min_freq").strip())

        if action.min_freq is not None and action.min_freq > current_max:
            return [max_operation, min_operation]
        if action.max_freq is not None and action.max_freq < current_min:
            return [min_operation, max_operation]
        return [min_operation, max_operation]

    def _frequency_operation(self, core: int, file_name: str, frequency: int | None) -> CpuWriteOperation | None:
        if frequency is None:
            return None
        return CpuWriteOperation(core, f"{self._cpufreq_path(core)}/{file_name}", str(frequency))

    def _snapshot_operations(self, operations: list[CpuWriteOperation]) -> dict[str, CpuWriteOperation]:
        snapshot = {}
        for operation in operations:
            if operation.path not in snapshot:
                snapshot[operation.path] = CpuWriteOperation(
                    core=operation.core,
                    path=operation.path,
                    value=self.shell.read(operation.path).strip(),
                )
        return snapshot

    def _rollback(
        self, snapshot: dict[str, CpuWriteOperation], applied_operations: list[CpuWriteOperation]
    ) -> list[CpuWriteOperation]:
        rolled_back_operations = []
        for operation in reversed(applied_operations):
            rollback_operation = snapshot[operation.path]
            self.shell.write(rollback_operation.path, rollback_operation.value)
            rolled_back_operations.append(rollback_operation)
        return rolled_back_operations

    def _cpu_path(self, core: int) -> str:
        return f"{CPU_SYSFS_ROOT}/cpu{core}"

    def _cpufreq_path(self, core: int) -> str:
        return f"{self._cpu_path(core)}/cpufreq"


class DeviceFeatureApplier:
    """Applies device-level feature toggles that are not tied to CPU cores."""

    SYS_PROD_PATH = "/sys_prod"
    SOC_PREF_XML_GLOB = "/sys_prod/etc/soc_pref/*.xml"

    def __init__(self, shell: DeviceShell | None = None, hdc_path: str = DEFAULT_HDC_PATH, serial: str | None = None):
        self.shell = shell
        self.hdc_path = hdc_path
        self.serial = serial

    def apply(self, request: CpuRequest) -> tuple[list[DeviceFeatureOperation], list[str]]:
        if not request.disable_freq_autoscalling:
            return [], []

        if request.dry_run:
            return self._planned_operations(applied=False), []

        if self.shell is None:
            raise ValidationError("Device shell is required when disable_freq_autoscalling is true and dry_run is false")

        operations = []
        self.shell.target_mount()
        operations.append(self._target_mount_operation(applied=True))

        self.shell.shell_script("mount | grep overlay", check=False)
        operations.append(self._shell_operation("mount | grep overlay", applied=True))

        operations.append(self._shell_operation(f"test -e {self.SYS_PROD_PATH}", applied=True))

        if not self.shell.exists(self.SYS_PROD_PATH):
            return operations, ["autoscalling нет: /sys_prod does not exist"]

        self.shell.shell_script(f"truncate -s 0 {self.SOC_PREF_XML_GLOB}")
        operations.append(
            self._shell_operation(
                f"truncate -s 0 {self.SOC_PREF_XML_GLOB}",
                applied=True,
                message="Skipped when /sys_prod does not exist.",
            )
        )
        return operations, []

    def _planned_operations(self, applied: bool) -> list[DeviceFeatureOperation]:
        return [
            self._target_mount_operation(applied),
            self._shell_operation("mount | grep overlay", applied),
            self._shell_operation(f"test -e {self.SYS_PROD_PATH}", applied),
            self._shell_operation(
                f"truncate -s 0 {self.SOC_PREF_XML_GLOB}",
                applied,
                message="Skipped when /sys_prod does not exist.",
            ),
        ]

    def _target_mount_operation(self, applied: bool) -> DeviceFeatureOperation:
        return DeviceFeatureOperation(
            name="disable_freq_autoscalling",
            command=self._hdc_command(["target", "mount"]),
            applied=applied,
        )

    def _shell_operation(self, script: str, applied: bool, message: str | None = None) -> DeviceFeatureOperation:
        return DeviceFeatureOperation(
            name="disable_freq_autoscalling",
            command=self._hdc_shell_command(script),
            applied=applied,
            message=message,
        )

    def _hdc_command(self, command: list[str]) -> list[str]:
        hdc_command = [self.hdc_path]
        if self.serial is not None:
            hdc_command.extend(["-t", self.serial])
        hdc_command.extend(command)
        return hdc_command

    def _hdc_shell_command(self, script: str) -> list[str]:
        return self._hdc_command(["shell", "sh", "-c", script])


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Parse and validate phone CPU configuration from JSON.",
        epilog=(
            "By default, dry_run is true and no phone state is changed. Set options.dry_run to false "
            "to apply validated writes through hdc.\n"
            "Set options.disable_freq_autoscalling to true to run the device-level frequency "
            "autoscaling disable flow before CPU apply. In dry-run mode, this is only shown as a plan.\n"
            "\n"
            "examples:\n"
            "  python3 cpu_control.py cpu_config.json\n"
            "  python3 cpu_control.py --hdc /path/to/hdc cpu_config.json\n"
            "\n"
            "example cpu_config.json:\n"
            "  {\n"
            "    \"cpu\": {\n"
            "      \"clusters\": [\n"
            "        {\"name\": \"little\", \"cores\": [0, 1, 2, 3], \"governor\": \"schedutil\"}\n"
            "      ],\n"
            "      \"cores\": [\n"
            "        {\"core\": 3, \"online\": false}\n"
            "      ]\n"
            "    },\n"
            "    \"options\": {\"dry_run\": true}\n"
            "  }"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("--hdc", default=DEFAULT_HDC_PATH, help="Path to hdc executable. Defaults to 'hdc'.")
    parser.add_argument("config", help="Path to JSON config file.")
    return parser.parse_args(argv)


def read_config_file(config_path: str) -> str:
    try:
        with open(config_path, "r", encoding="utf-8") as config_file:
            return config_file.read()
    except OSError as error:
        raise InputParseError(f"Cannot read config file '{config_path}': {error}") from error


def load_cpu_request(config_path: str) -> CpuRequest:
    raw_json = read_config_file(config_path)
    return JsonInputParser(config_path).parse_text(raw_json)


def execute_cpu_request(config_path: str, hdc_path: str) -> CpuApplyResult:
    request = load_cpu_request(config_path)
    CpuRequestValidator().validate(request)
    shell = None if request.dry_run else HdcDeviceShell(request.device.serial, hdc_path)
    feature_operations, messages = DeviceFeatureApplier(shell, hdc_path, request.device.serial).apply(request)
    plan = CpuRequestValidator(shell).validate(request)
    result = CpuRequestApplier(shell).apply(plan)
    return CpuApplyResult(
        dry_run=result.dry_run,
        operations=result.operations,
        applied_operations=result.applied_operations,
        rolled_back_operations=result.rolled_back_operations,
        request=result.request,
        feature_operations=feature_operations,
        messages=messages,
    )


def main(argv: list[str]) -> int:
    args = parse_args(argv)

    try:
        result = execute_cpu_request(args.config, args.hdc)
    except (InputParseError, ValidationError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    print(json.dumps(asdict(result), indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
