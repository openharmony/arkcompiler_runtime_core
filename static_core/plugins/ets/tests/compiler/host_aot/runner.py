#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2026 Huawei Device Co., Ltd.
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

"""Host AOT integration runner for multiple method-config scenarios.

Scenarios:
1. Single abc: compile one abc with multiple methods and verify AN contains only configured methods.
2. Multi abc: compile two abcs with independent configs, verify each AN contains only corresponding methods,
   and thread-pool workers equal abc count.
3. Overload: compile one abc with overloaded methods and verify AN contains only configured signatures.
"""

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass
from typing import Dict, List, Set


def log_info(message):
    print(f"[host_aot_runner] {message}", flush=True)


def run_command(cmd, cwd=None):
    cmd_text = " ".join(cmd)
    log_info(f"run command: {cmd_text}")
    if cwd:
        log_info(f"command cwd: {cwd}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, check=False)
    log_info(f"command returncode: {result.returncode}")
    if result.stdout:
        log_info("command stdout begin")
        print(result.stdout, end="" if result.stdout.endswith("\n") else "\n", flush=True)
        log_info("command stdout end")
    if result.stderr:
        log_info("command stderr begin")
        print(result.stderr, end="" if result.stderr.endswith("\n") else "\n", flush=True)
        log_info("command stderr end")
    if result.returncode != 0:
        raise RuntimeError(f"command failed ({result.returncode}): {cmd_text}")
    return result


def ensure_file(path):
    if not os.path.isfile(path):
        raise FileNotFoundError(f"file not found: {path}")


def normalize_method_signature(method_signature):
    signature = method_signature.strip()
    return signature.replace(", ", ",")


def parse_method_config(config_path):
    with open(config_path, "r", encoding="utf-8") as cfg:
        lines = [
            line.strip()
            for line in cfg.read().splitlines()
            if line.strip() and not line.strip().startswith("#")
        ]
    if len(lines) < 2:
        raise RuntimeError(f"config requires abc path + at least one method: {config_path}")

    abc_device_path = lines[0]
    configured_methods = [normalize_method_signature(line) for line in lines[1:]]
    return abc_device_path, configured_methods


def parse_compiler_dump_methods(dump_path):
    with open(dump_path, "r", encoding="utf-8") as dump_file:
        return parse_compiler_dump_methods_text(dump_file.read())


def parse_compiler_dump_methods_text(dump_text):
    method_names = []
    for line in dump_text.splitlines():
        match = re.match(r"\s*name:\s*(.+)\s*$", line)
        if match:
            method_names.append(normalize_method_signature(match.group(1)))
    return method_names


def write_host_config(config_path):
    config = {
        "defaultAotArgs": [
            "--load-runtimes=ets",
            "--compiler-ignore-failures=false",
            "--paoc-mode=aot",
            "--compiler-regex=to_be_removed",
            "--compiler-disasm-dump:stdout=true,code=false",
        ],
        "components": [],
    }
    with open(config_path, "w", encoding="utf-8") as cfg:
        json.dump(config, cfg)


def prepare_workspace(work_dir):
    if os.path.exists(work_dir):
        shutil.rmtree(work_dir)
    os.makedirs(work_dir, exist_ok=True)


def copy_method_config(source_path, destination_path):
    abc_device_path, configured_methods = parse_method_config(source_path)
    os.makedirs(os.path.dirname(destination_path), exist_ok=True)
    with open(destination_path, "w", encoding="utf-8") as cfg:
        cfg.write(abc_device_path + "\n")
        cfg.write("\n".join(configured_methods) + "\n")


@dataclass
class ComponentInput:
    ets_path: str
    config_path: str


@dataclass
class Scenario:
    name: str
    components: List[ComponentInput]
    expected_only_methods: Dict[str, List[str]]
    expected_worker_count: int = 0


def scenario_single_abc(args):
    return Scenario(
        name="single_abc",
        components=[
            ComponentInput(args.single_ets, args.single_config),
        ],
        expected_only_methods={
            "/system/framework/single_case.abc": [
                "i32 single_case.ETSGLOBAL::keepAlpha()",
                "i32 single_case.ETSGLOBAL::keepBeta(i32)",
            ]
        },
        expected_worker_count=1,
    )


def scenario_multi_abc(args):
    return Scenario(
        name="multi_abc",
        components=[
            ComponentInput(args.multi_first_ets, args.multi_first_config),
            ComponentInput(args.multi_second_ets, args.multi_second_config),
        ],
        expected_only_methods={
            "/system/framework/multi_first.abc": [
                "i32 multi_first.ETSGLOBAL::firstKeep(i32)",
            ],
            "/system/framework/multi_second.abc": [
                "i32 multi_second.ETSGLOBAL::secondKeep()",
            ],
        },
        expected_worker_count=2,
    )


def scenario_overload(args):
    return Scenario(
        name="overload_abc",
        components=[
            ComponentInput(args.overload_ets, args.overload_config),
        ],
        expected_only_methods={
            "/system/framework/overload_case.abc": [
                "i32 overload_case.ETSGLOBAL::chooser()",
                "i32 overload_case.ETSGLOBAL::chooser(i32)",
            ]
        },
        expected_worker_count=1,
    )


def setup_system_root(system_root, ets_stdlib):
    framework_dir = os.path.join(system_root, "system", "framework")
    os.makedirs(framework_dir, exist_ok=True)

    stdlib_host_path = os.path.join(framework_dir, "etsstdlib.abc")
    copied_path = shutil.copyfile(ets_stdlib, stdlib_host_path)
    if copied_path != stdlib_host_path:
        raise RuntimeError("failed to copy stdlib")

    bootpath_json = os.path.join(framework_dir, "bootpath.json")
    with open(bootpath_json, "w", encoding="utf-8") as boot_cfg:
        json.dump({"bootpath": "/system/framework/etsstdlib.abc"}, boot_cfg)


def prepare_tool(tool_path, run_prefix_text, run_root, wrapper_name):
    if not run_prefix_text:
        return tool_path

    wrapper_path = os.path.join(run_root, wrapper_name)
    command = [*shlex.split(run_prefix_text), tool_path]
    with open(wrapper_path, "w", encoding="utf-8") as wrapper:
        wrapper.write("#!/usr/bin/env bash\n")
        wrapper.write("exec " + " ".join(shlex.quote(arg) for arg in command) + ' "$@"\n')
    os.chmod(wrapper_path, 0o755)
    return wrapper_path


def compile_abc_inputs(es2panda_path, system_root, components):
    abc_to_expected = {}
    for comp in components:
        abc_device_path, configured_methods = parse_method_config(comp.config_path)
        abc_host_path = os.path.join(system_root, abc_device_path.lstrip("/"))
        os.makedirs(os.path.dirname(abc_host_path), exist_ok=True)

        result = run_command([
            es2panda_path,
            "--output",
            abc_host_path,
            "--extension=ets",
            comp.ets_path,
        ])
        if result.returncode != 0:
            raise RuntimeError(f"failed to compile abc: {abc_host_path}")

        abc_to_expected[abc_device_path] = configured_methods

    return abc_to_expected


def run_host_aot(script_copy, system_root, ark_aot_path, host_config_path):
    return run_command([
        sys.executable,
        script_copy,
        "--system-path",
        system_root,
        "--aot-tool",
        ark_aot_path,
        "--config",
        host_config_path,
        "--is-framework",
        "true",
    ])


def assert_methods_exact(abc_device_path, actual_methods, expected_methods):
    expected_set: Set[str] = {normalize_method_signature(x) for x in expected_methods}
    actual_set: Set[str] = set(actual_methods)

    missing = sorted(expected_set - actual_set)
    unexpected = sorted(actual_set - expected_set)

    if missing or unexpected:
        raise RuntimeError(
            "method mismatch for {}: missing={}, unexpected={}".format(
                abc_device_path, missing, unexpected
            )
        )


def assert_worker_count(log_text, expected_worker_count):
    pattern = r"Using thread pool workers=(\d+)"
    match = re.search(pattern, log_text)
    if not match:
        raise RuntimeError("missing worker count log in host_aot output")
    actual = int(match.group(1))
    if actual != expected_worker_count:
        raise RuntimeError(
            f"worker count mismatch: expected={expected_worker_count}, actual={actual}"
        )


def extract_component_stdout(log_text, abc_device_path):
    marker = "[config:{}] stdout:\n".format(os.path.basename(abc_device_path))
    start = log_text.find(marker)
    if start < 0:
        raise RuntimeError(f"missing compiler stdout for {abc_device_path}")

    start += len(marker)
    next_log_record = re.search(r"\n\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3} ", log_text[start:])
    if next_log_record:
        return log_text[start:start + next_log_record.start()]
    return log_text[start:]


def run_scenario(args, scenario):
    log_info(f"scenario start: {scenario.name}")

    scenario_root = os.path.join(os.path.abspath(args.work_dir), scenario.name)
    run_root = os.path.join(scenario_root, "run_root")
    system_root = os.path.join(scenario_root, "device_root")
    config_dir = os.path.join(run_root, "system_framework_abc_config")
    os.makedirs(config_dir, exist_ok=True)

    script_copy = os.path.join(run_root, "host_aot_compile.py")
    copied_script = shutil.copyfile(args.host_aot_script, script_copy)
    if copied_script != script_copy:
        raise RuntimeError("failed to prepare host_aot script")

    es2panda_tool = prepare_tool(args.es2panda, args.es2panda_run_prefix, run_root, "es2panda_runner.sh")
    aot_tool = prepare_tool(args.ark_aot, args.ark_aot_run_prefix, run_root, "ark_aot_runner.sh")

    setup_system_root(system_root, args.ets_stdlib)

    abc_to_configured = compile_abc_inputs(es2panda_tool, system_root, scenario.components)
    if set(abc_to_configured.keys()) != set(scenario.expected_only_methods.keys()):
        raise RuntimeError(
            f"scenario {scenario.name} expected abc set mismatch: "
            f"compiled={sorted(abc_to_configured.keys())}, expected={sorted(scenario.expected_only_methods.keys())}"
        )

    for index, component in enumerate(scenario.components, start=1):
        target_name = f"scenario_{scenario.name}_{index}.txt"
        destination = os.path.join(config_dir, target_name)
        copy_method_config(component.config_path, destination)

    host_config_path = os.path.join(scenario_root, "host_aot_config.json")
    write_host_config(host_config_path)

    result = run_host_aot(script_copy, system_root, aot_tool, host_config_path)

    if scenario.expected_worker_count > 0:
        assert_worker_count(result.stderr, scenario.expected_worker_count)

    for abc_device_path, expected_methods in scenario.expected_only_methods.items():
        compiler_stdout = extract_component_stdout(result.stderr, abc_device_path)
        actual_methods = parse_compiler_dump_methods_text(compiler_stdout)
        if not actual_methods:
            raise RuntimeError(f"no methods parsed from compiler stdout for {abc_device_path}")
        assert_methods_exact(abc_device_path, actual_methods, expected_methods)

        an_device_path = abc_device_path[:-4] + ".an"
        an_host_path = os.path.join(system_root, an_device_path.lstrip("/"))
        ensure_file(an_host_path)

    log_info(f"scenario pass: {scenario.name}")


def run_all(args):
    ensure_file(args.es2panda)
    ensure_file(args.ark_aot)
    ensure_file(args.host_aot_script)
    ensure_file(args.ets_stdlib)

    ensure_file(args.single_ets)
    ensure_file(args.single_config)
    ensure_file(args.multi_first_ets)
    ensure_file(args.multi_first_config)
    ensure_file(args.multi_second_ets)
    ensure_file(args.multi_second_config)
    ensure_file(args.overload_ets)
    ensure_file(args.overload_config)

    prepare_workspace(os.path.abspath(args.work_dir))

    scenarios = [
        scenario_single_abc(args),
        scenario_multi_abc(args),
        scenario_overload(args),
    ]

    for scenario in scenarios:
        run_scenario(args, scenario)


def parse_args():
    parser = argparse.ArgumentParser(description="Host AOT integration runner")
    parser.add_argument("--es2panda", required=True, help="Path to es2panda")
    parser.add_argument("--es2panda-run-prefix", default="", help="Command prefix for running es2panda")
    parser.add_argument("--ark-aot", required=True, help="Path to ark_aot")
    parser.add_argument("--ark-aot-run-prefix", default="", help="Command prefix for running ark_aot")
    parser.add_argument("--host-aot-script", required=True, help="Path to host_aot_compile.py")
    parser.add_argument("--ets-stdlib", required=True, help="Path to ets stdlib abc")
    parser.add_argument("--work-dir", required=True, help="Runner working directory")

    parser.add_argument("--single-ets", required=True, help="Single abc ets input")
    parser.add_argument("--single-config", required=True, help="Single abc config")

    parser.add_argument("--multi-first-ets", required=True, help="Multi abc first ets input")
    parser.add_argument("--multi-first-config", required=True, help="Multi abc first config")
    parser.add_argument("--multi-second-ets", required=True, help="Multi abc second ets input")
    parser.add_argument("--multi-second-config", required=True, help="Multi abc second config")

    parser.add_argument("--overload-ets", required=True, help="Overload ets input")
    parser.add_argument("--overload-config", required=True, help="Overload config")

    parser.add_argument("--stamp", default="", help="Optional stamp output path")
    return parser.parse_args()


def main():
    args = parse_args()
    try:
        run_all(args)
    except Exception as err:  # pylint: disable=broad-exception-caught
        log_info(f"failed: {err}")
        return 1

    if args.stamp:
        os.makedirs(os.path.dirname(args.stamp), exist_ok=True)
        with open(args.stamp, "w", encoding="utf-8") as stamp_file:
            stamp_file.write("ok\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
