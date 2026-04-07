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

"""Host AOT compilation script for ArkCompiler.

Reads host_aot_config.json, compiles each enabled component using ark_aot_host
in parallel, and outputs .an files using host-side mapped paths.

Usage:
    python3 host_aot_compile.py \
        --system-path=/path/to/device_root \
        --aot-tool=/path/to/ark_aot_host \
        --config=/path/to/host_aot_config.json \
        --is-framework=true/false
"""

import argparse
import json
import logging
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

LOG = logging.getLogger("host_aot")


def parse_args():
    parser = argparse.ArgumentParser(description="Host AOT compilation for ArkCompiler")
    parser.add_argument("--system-path", required=True,
                        help="Host mapping prefix for device root paths.")
    parser.add_argument("--aot-tool", required=True,
                        help="Path to ark_aot_host binary")
    parser.add_argument("--config", required=True,
                        help="Path to host_aot_config.json")
    parser.add_argument("--is-framework", default=None,
                        help="Filter components by isFramework in config. "
                              "true: compile only framework abc"
                              "false: compile only hap components")
    return parser.parse_args()


def normalize_device_path(device_path):
    s = (device_path or "").strip()
    if not s:
        return ""
    if s.startswith("/"):
        return s
    if s.startswith("system/"):
        return "/" + s
    return "/" + s


def device_to_host_path(device_path, system_path):
    """Convert a device path to a host path by direct prefix mapping."""

    normalized = normalize_device_path(device_path)
    if not normalized:
        return ""
    return os.path.join(system_path, normalized.lstrip("/"))


def parse_boot_panda_locations(boot_json_path):
    """Parse bootpath.json and return normalized full boot locations."""

    with open(boot_json_path, 'r') as f:
        boot_data = json.load(f)

    bootpath = boot_data.get("bootpath", "")
    if not isinstance(bootpath, str):
        LOG.warning("bootpath.json has invalid bootpath field: %s", boot_json_path)
        return ""

    boot_files_list = bootpath.split(":")
    boot_files_list = [normalize_device_path(p) for p in boot_files_list if p.strip()]
    return ":".join(boot_files_list)


def get_boot_panda_files(system_path, boot_panda_locations):
    """Convert boot panda locations (device paths) to host file paths."""

    return ":".join(
        device_to_host_path(f, system_path)
        for f in boot_panda_locations.split(":")
    )


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def resolve_method_config_dir():
    """Resolve config directory as sibling folder of this script."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, "system_framework_abc_config")


def load_methods_method_configs(method_config_dir):
    """Load config from a directory.
    """
    abc_to_methods_file = {}
    needs_signature_match = False

    for entry in sorted(os.listdir(method_config_dir)):
        src_path = os.path.join(method_config_dir, entry)
        if not os.path.isfile(src_path):
            continue
        if not entry.lower().endswith(".txt"):
            continue

        try:
            with open(src_path, "r", encoding="utf-8") as f:
                raw_lines = f.read().splitlines()
        except Exception as e:
            LOG.warning("Failed to read method config file %s: %s", src_path, e)
            continue

        if not raw_lines:
            LOG.warning("Skip empty method config file: %s", src_path)
            continue

        device_abc = normalize_device_path(raw_lines[0].strip())
        if not device_abc:
            LOG.warning("method config file %s has invalid abc path line: %s", src_path, raw_lines[0])
            continue

        method_lines = [m.strip() for m in raw_lines[1:] if m.strip()]
        if not method_lines:
            LOG.warning("Skip method config file %s: abc path exists but no method entries", src_path)
            continue

        if not needs_signature_match:
            for mm in method_lines:
                if "(" in mm and ")" in mm:
                    needs_signature_match = True
                    break

        abc_to_methods_file[device_abc] = src_path

    if abc_to_methods_file:
        LOG.info("Loaded %d method config mapping(s) from %s", len(abc_to_methods_file), method_config_dir)
    return abc_to_methods_file, needs_signature_match


def enforce_signature_regex_only(aot_args):
    """Ensure we only use --compiler-regex-with-signature and never --compiler-regex.

    This repository disallows using both options together. The user requirement here is stronger:
    only signature-based matching is allowed.
    """
    if not aot_args:
        aot_args = []

    filtered = []
    removed = []
    skip_next = False
    for idx, arg in enumerate(aot_args):
        if skip_next:
            skip_next = False
            removed.append("(value for {})".format(aot_args[idx - 1] if idx > 0 else "--compiler-regex"))
            continue

        if arg == "--compiler-regex":
            removed.append(arg)
            skip_next = True
            continue
        if arg.startswith("--compiler-regex="):
            removed.append(arg)
            continue
        filtered.append(arg)

    if removed:
        LOG.warning("Removed disallowed --compiler-regex arguments: %s", " ".join(removed))

    if not any(a.startswith("--compiler-regex-with-signature") for a in filtered):
        filtered.append("--compiler-regex-with-signature=.*")

    return filtered


def default_an_install_path(device_abc_path):
    # Keep output next to the abc with .an extension.
    base, _ext = os.path.splitext(device_abc_path)
    return base + ".an"


def ensure_parent_dir(file_path):
    parent = os.path.dirname(file_path)
    if parent:
        ensure_dir(parent)


def ensure_component_paths(system_path, component, is_hap):
    out_host_path = device_to_host_path(component.get("anInstallPath", ""), system_path)
    if out_host_path:
        ensure_parent_dir(out_host_path)
    if is_hap:
        loc = component.get("abcInstallDir", "")
        loc_host = device_to_host_path(loc, system_path)
        if loc_host:
            ensure_dir(loc_host)


def build_config_components(abc_to_methods_file, existing_components):
    """Create config-based compile tasks from config mapping.

    The config abc set MUST NOT come from host_aot_config.json, which is reserved for full compilation.
    """
    existing_abc = set()
    for c in existing_components or []:
        p = c.get("abcPath", "")
        if p:
            existing_abc.add(p)

    components = []
    for device_abc in sorted(abc_to_methods_file.keys()):
        if device_abc in existing_abc:
            LOG.warning("config abc %s also appears in host_aot_config.json; skipping config task for it",
                        device_abc)
            continue
        components.append({
            "componentsName": "config:" + os.path.basename(device_abc),
            "enableHostAot": True,
            "abcPath": device_abc,
            "isFramework": True,
            "abcInstallDir": os.path.dirname(device_abc) + "/",
            "anInstallPath": default_an_install_path(device_abc),
        })
    return components


def compile_component(aot_tool, system_path, component, boot_json_path,
                      default_aot_args, abc_to_methods_file):
    """Compile a single component using ark_aot_host.

    Returns True on success, False on failure.
    """
    name = component.get("componentsName", "unknown")

    if not component.get("enableHostAot", False):
        LOG.info("Skipping %s: enableHostAot is false", name)
        return True

    abc_path = component.get("abcPath", "")
    hap_path = component.get("hapPath", "")
    module_path = component.get("modulePath", "")
    is_hap = bool(hap_path)
    if is_hap:
        missing = []
        if not component.get("abcInstallDir", ""):
            missing.append("abcInstallDir(paoc-location)")
        if not module_path:
            missing.append("modulePath(paoc-zip-panda-file)")
        if not component.get("anInstallPath", ""):
            missing.append("anInstallPath(paoc-output)")
        if missing:
            LOG.warning("Skip %s: HAP compile requires explicit config fields in host_aot_config.json: %s",
                        name, ", ".join(missing))
            return False

    cmd_args = []
    paoc_location = component.get("abcInstallDir", "")
    boot_locations = parse_boot_panda_locations(boot_json_path)
    if not boot_locations:
        LOG.warning("Skip %s: bootpath.json has no valid bootpath entries: %s", name, boot_json_path)
        return False
    boot_locations_host = get_boot_panda_files(system_path, boot_locations)

    cmd_args.append("--paoc-location={}".format(paoc_location))
    cmd_args.append("--paoc-boot-panda-locations={}".format(boot_locations))

    if is_hap:
        panda_files = hap_path.rstrip("/") + "/" + module_path
        host_hap_dir = device_to_host_path(hap_path, system_path)
        host_panda_files = device_to_host_path(panda_files, system_path)
        if not host_hap_dir or not os.path.isfile(host_hap_dir):
            LOG.warning("Skip %s: HAP path not found on host: %s", name, host_hap_dir)
            return False
        ensure_component_paths(system_path, component, True)
        cmd_args.append("--paoc-panda-files={}".format(host_panda_files))
        cmd_args.append("--paoc-zip-panda-file={}".format(module_path))
    else:
        host_abc = device_to_host_path(abc_path, system_path)
        if not host_abc or not os.path.isfile(host_abc):
            LOG.warning("Skip %s: ABC not found on host: %s", name, host_abc)
            return False
        cmd_args.append("--paoc-panda-files={}".format(host_abc))

    cmd_args.append("--boot-panda-files={}".format(boot_locations_host))
    cmd_args.append("--paoc-output={}".format(
        device_to_host_path(component.get("anInstallPath", ""), system_path)))

    methods_file = abc_to_methods_file.get(abc_path, "")
    if methods_file and not is_hap:
        cmd_args.append("--paoc-methods-from-file:path={},iswhite=true".format(methods_file))

    if component.get("defaultAotMode", "") == "partial":
        profile_path = component.get("arkProfilePath", "")
        if profile_path:
            cmd_args.append("--paoc-use-profile:path={}".format(profile_path))
            cmd_args.append("--paoc-use-profile:force")
        else:
            LOG.warning("Component %s: defaultAotMode=partial but arkProfilePath "
                        "is empty", name)

    cmd = [aot_tool] + list(default_aot_args) + cmd_args
    LOG.info("Host AOT start: %s, cmd: %s", name, " ".join(cmd))

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=False)
        if result.stdout:
            LOG.info("[%s] stdout:\n%s", name, result.stdout)
        if result.stderr:
            LOG.info("[%s] stderr:\n%s", name, result.stderr)
        if result.returncode != 0:
            LOG.warning("Host AOT failed for %s, returncode=%d", name,
                        result.returncode)
            return False
        LOG.info("Host AOT success: %s", name)
        return True
    except Exception as e:
        LOG.warning("Host AOT failed for %s (exception): %s", name, e)
        return False


def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s [%(name)s] %(message)s")

    args = parse_args()

    system_path = os.path.realpath(args.system_path)
    if not os.path.isdir(system_path):
        LOG.error("System path does not exist: %s", args.system_path)
        return 1

    LOG.info("Using device-root mapping prefix: %s (input: %s)", system_path, args.system_path)

    if not os.path.isfile(args.aot_tool):
        LOG.error("AOT tool not found: %s", args.aot_tool)
        return 1

    if not os.path.isfile(args.config):
        LOG.error("Config file not found: %s", args.config)
        return 1

    with open(args.config, 'r') as f:
        config = json.load(f)

    default_aot_args = config.get("defaultAotArgs", [])
    full_components = config.get("components", [])

    if args.is_framework is not None:
        flag = args.is_framework.lower()
        if flag not in ("true", "false"):
            LOG.error("Invalid --is-framework value: %s (must be true/false)", args.is_framework)
            return 1
        only_framework = flag == "true"
        before = len(full_components)
        full_components = [
            c for c in full_components
            if bool(c.get("isFramework", False)) == only_framework
        ]
        LOG.info("Filter components by isFramework=%s: %d -> %d",
                 flag, before, len(full_components))

    default_aot_args = enforce_signature_regex_only(default_aot_args)

    method_config_dir = resolve_method_config_dir()
    if not os.path.isdir(method_config_dir):
        LOG.warning("method config dir not found, continue without config: %s", method_config_dir)

    if not full_components:
        LOG.info("No full components in config")

    boot_json_path = os.path.join(system_path, "system", "framework", "bootpath.json")
    if not os.path.isfile(boot_json_path):
        LOG.error("bootpath.json not found: %s", boot_json_path)
        return 1

    abc_to_methods_file, needs_signature_match = {}, False
    if os.path.isdir(method_config_dir):
        abc_to_methods_file, needs_signature_match = load_methods_method_configs(method_config_dir)
        if needs_signature_match:
            LOG.info("Detected signature-style entries in config files")

    config_components = []
    if args.is_framework is None or args.is_framework.lower() == "true":
        config_components = build_config_components(abc_to_methods_file, full_components)
    components = list(full_components) + list(config_components)

    if not components:
        LOG.info("No components to compile (full=%d, config=%d)", len(full_components), len(config_components))
        return 0

    LOG.info("Host AOT compilation start, full=%d config=%d", len(full_components), len(config_components))

    # Compile components with bounded parallelism based on CPU cores
    results = {}
    cpu_count = os.cpu_count() or 1
    max_workers = max(1, min(len(components), cpu_count))
    LOG.info("Using thread pool workers=%d (cpu_count=%d)", max_workers, cpu_count)

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        future_to_name = {}
        for component in components:
            name = component.get("componentsName", "unknown")
            future = executor.submit(
                compile_component,
                args.aot_tool,
                system_path,
                component,
                boot_json_path,
                default_aot_args,
                abc_to_methods_file,
            )
            future_to_name[future] = name

        for future in as_completed(future_to_name):
            name = future_to_name[future]
            try:
                results[name] = future.result()
            except Exception as e:
                LOG.warning("Host AOT failed for %s: %s", name, e)
                results[name] = False

    failed = [name for name, ok in results.items() if not ok]
    if failed:
        LOG.warning("Host AOT completed with failures: %s", ", ".join(failed))
    else:
        LOG.info("Host AOT compilation complete, all succeeded")
    return 0


if __name__ == "__main__":
    sys.exit(main())
