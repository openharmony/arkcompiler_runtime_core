# coding=utf-8
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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
import sys
from pathlib import Path

from taihe.driver.backend import BackendRegistry
from taihe.driver.contexts import CompilerInstance, CompilerInvocation
from taihe.utils.outputs import CMakeOutputConfig, OutputConfig
from taihe.utils.resources import ResourceLocator, ResourceType


def main():
    registry = BackendRegistry()
    registry.register_all()

    parser = argparse.ArgumentParser(
        prog="taihec",
        description="generates source code from taihe files",
    )
    parser.add_argument(
        "src_files",
        type=Path,
        nargs="*",
        default=[],
        help="input .taihe files, if not provided, will read from stdin",
    )
    parser.add_argument(
        "-I",
        type=Path,
        dest="src_dirs",
        nargs="*",
        default=[],
        help="directories of .taihe source files",
    )  # deprecated
    parser.add_argument(
        "--output",
        "-O",
        type=Path,
        dest="dst_dir",
        required=True,
        help="directory for generated files",
    )
    parser.add_argument(
        "--generate",
        "-G",
        dest="backends",
        nargs="*",
        default=[],
        choices=registry.get_backend_names(),
        help="backends to generate sources, default: abi-header, abi-source, c-author",
    )
    parser.add_argument(
        "--codegen",
        "-C",
        dest="config",
        action="append",
        default=[],
        help="additional code generation configuration",
    )
    parser.add_argument(
        "--build",
        "-B",
        dest="build_system",
        choices=["cmake"],
        help="build system to use for generated sources",
    )
    args = parser.parse_args()

    backends = registry.collect_required_backends(args.backends)
    resolved_backends = [b() for b in backends]

    locator = ResourceLocator.detect()
    if args.build_system == "cmake":
        output_config = CMakeOutputConfig(
            dst_dir=Path(args.dst_dir),
            runtime_include_dir=locator.get(ResourceType.RUNTIME_HEADER),
            runtime_src_dir=locator.get(ResourceType.RUNTIME_SOURCE),
        )
    else:
        output_config = OutputConfig(
            dst_dir=Path(args.dst_dir),
        )

    invocation = CompilerInvocation(
        src_files=args.src_files,
        src_dirs=args.src_dirs,
        output_config=output_config,
        backends=resolved_backends,
    )

    for config in args.config:
        k, *v = config.split("=", 1)
        if k == "sts:keep-name":
            invocation.sts_keep_name = True
        elif k == "arkts:module-prefix":
            invocation.arkts_module_prefix = v[0] if v else None
        elif k == "arkts:path-prefix":
            invocation.arkts_path_prefix = v[0] if v else None
        else:
            raise ValueError(f"unknown codegen config {k!r}")

    instance = CompilerInstance(invocation)

    if not instance.run():
        return -1
    return 0


if __name__ == "__main__":
    sys.exit(main())