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
import json
import logging
import os
import shutil
import subprocess
import sys
import tarfile
import time
from collections.abc import Iterable, Mapping, Sequence
from enum import Enum
from pathlib import Path
from typing import cast

from taihe.driver.backend import BackendRegistry
from taihe.driver.contexts import CompilerInstance, CompilerInvocation
from taihe.utils.logging import setup_logger
from taihe.utils.outputs import CMakeOutputConfig, DebugLevel, OutputConfig
from taihe.utils.resources import (
    PandaVm,
    ResourceContext,
    RuntimeHeader,
    RuntimeSource,
    StandardLibrary,
    fetch_url,
)

# A lower value means more verbosity
TRACE_CONCISE = logging.DEBUG - 1
TRACE_VERBOSE = TRACE_CONCISE - 1


class UserType(Enum):
    """User type for the build system."""

    STS = "sts"
    CPP = "cpp"


class BuildUtils:
    """Utility class for common operations."""

    def __init__(self):
        self.logger = logging.getLogger("build_system")

    def run_command(
        self,
        command: Sequence[Path | str],
        capture_output: bool = False,
        env: Mapping[str, Path | str] | None = None,
    ) -> float:
        """Run a command with environment variables."""
        command_str = " ".join(map(str, command))

        env_str = ""
        for key, val in (env or {}).items():
            env_str += f"{key}={val} "

        self.logger.debug("+ %s%s", env_str, command_str)

        try:
            start_time = time.time()
            subprocess.run(
                command,
                check=True,
                text=True,
                env=env,
                capture_output=capture_output,
            )
            end_time = time.time()
            elapsed_time = end_time - start_time
            return elapsed_time
        except subprocess.CalledProcessError as e:
            self.logger.error("Command failed with exit code %s", e.returncode)
            if e.stdout:
                self.logger.error("Standard output: %s", e.stdout)
            if e.stderr:
                self.logger.error("Standard error: %s", e.stderr)
            raise

    def create_directory(self, directory: Path) -> None:
        directory.mkdir(parents=True, exist_ok=True)
        self.logger.debug("Created directory: %s", directory)

    def clean_directory(self, directory: Path) -> None:
        if not directory.exists():
            return
        shutil.rmtree(directory)
        self.logger.debug("Cleaned directory: %s", directory)

    def move_directory(self, src: Path, dst: Path) -> None:
        if not src.exists():
            raise FileNotFoundError(f"Source directory does not exist: {src}")
        shutil.move(src, dst)
        self.logger.debug("Moved directory from %s to %s", src, dst)

    def copy_directory(self, src: Path, dst: Path) -> None:
        if not src.exists():
            raise FileNotFoundError(f"Source directory does not exist: {src}")
        shutil.copytree(src, dst, dirs_exist_ok=True)
        self.logger.debug("Copied directory from %s to %s", src, dst)

    def extract_file(
        self,
        target_file: Path,
        extract_dir: Path,
    ) -> None:
        """Extract a tar.gz file."""
        if not target_file.exists():
            raise FileNotFoundError(f"File to extract does not exist: {target_file}")

        self.create_directory(extract_dir)

        with tarfile.open(target_file, "r:gz") as tar:
            # Check for any unsafe paths before extraction
            for member in tar.getmembers():
                member_path = Path(member.name)
                if member_path.is_absolute() or ".." in member_path.parts:
                    raise ValueError(f"Unsafe path in archive: {member.name}")
            # Extract safely
            tar.extractall(path=extract_dir)

        self.logger.info("Extracted %s to %s", target_file, extract_dir)


class BuildConfig:
    """Configuration for the build process."""

    def __init__(self, vm: PandaVm | None):
        self.cxx = os.getenv("CXX", "clang++")
        self.cc = os.getenv("CC", "clang")
        self._vm = vm

    @property
    def vm(self) -> PandaVm:
        pass
        return self._vm


def _map_output_debug_level(verbosity: int) -> DebugLevel:
    if verbosity <= TRACE_VERBOSE:
        return DebugLevel.VERBOSE
    if verbosity <= TRACE_CONCISE:
        return DebugLevel.CONCISE
    return DebugLevel.NONE


class BuildSystem(BuildUtils):
    """Main build system class."""

    def __init__(
        self,
        target_dir: str,
        user: UserType,
        config: BuildConfig,
        verbosity: int = logging.INFO,
    ):
        super().__init__()
        self.config = config
        self.should_run_pretty_print = verbosity <= logging.DEBUG
        self.codegen_debug_level = _map_output_debug_level(verbosity)

        self.user = user

        # Build paths
        self.target_path = Path(target_dir).resolve()

        self.idl_dir = self.target_path / "idl"
        self.build_dir = self.target_path / "build"

        self.generated_dir = self.target_path / "generated"
        self.generated_include_dir = self.generated_dir / "include"
        self.generated_src_dir = self.generated_dir / "src"

        self.author_dir = self.target_path / "author"
        self.author_include_dir = self.author_dir / "include"
        self.author_src_dir = self.author_dir / "src"

        self.user_dir = self.target_path / "user"
        self.user_include_dir = self.user_dir / "include"
        self.user_src_dir = self.user_dir / "src"

        self.runtime_includes = [RuntimeHeader.resolve_path()]
        if self.user == UserType.STS:
            self.runtime_includes.append(config.vm.ani_header_dir)
        self.generated_includes = [*self.runtime_includes, self.generated_include_dir]
        self.author_includes = [*self.generated_includes, self.author_include_dir]
        self.user_includes = [*self.generated_includes, self.user_include_dir]

        # Build sub-directories
        self.build_generated_src_dir = self.build_dir / "generated" / "src"
        self.build_author_src_dir = self.build_dir / "author" / "src"
        self.build_runtime_src_dir = self.build_dir / "runtime" / "src"
        self.build_generated_dir = self.build_dir / "generated"
        self.build_user_dir = self.build_dir / "user"

        # Output files
        self.lib_name = self.target_path.absolute().name
        self.so_target = self.build_dir / f"lib{self.lib_name}.so"
        self.abc_target = self.build_dir / "main.abc"
        self.exe_target = self.build_dir / "main"
        self.arktsconfig_file = self.build_dir / "arktsconfig.json"

    def create(self) -> None:
        """Create a simple example project."""
        self.create_idl()
        self.create_author_cpp()
        if self.user == UserType.STS:
            self.create_user_ets()
        if self.user == UserType.CPP:
            self.create_user_cpp()

    def create_idl(self) -> None:
        """Create a simple example IDL file."""
        self.create_directory(self.idl_dir)
        with open(self.idl_dir / "hello.taihe", "w") as f:
            f.write(f"function sayHello(): void;\n")

    def create_author_cpp(self) -> None:
        """Create a simple example author source file."""
        self.create_directory(self.author_src_dir)
        with open(self.author_dir / "compile_flags.txt", "w") as f:
            for author_include_dir in self.author_includes:
                f.write(f"-I{author_include_dir}\n")
        with open(self.author_src_dir / "hello.impl.cpp", "w") as f:
            f.write(
                f'#include "hello.proj.hpp"\n'
                f'#include "hello.impl.hpp"\n'
                f"\n"
                f"#include <iostream>\n"
                f"\n"
                f"void sayHello() {{\n"
                f'    std::cout << "Hello, World!" << std::endl;\n'
                f"    return;\n"
                f"}}\n"
                f"\n"
                f"TH_EXPORT_CPP_API_sayHello(sayHello);\n"
            )

    def create_user_ets(self) -> None:
        """Create a simple example user ETS file."""
        self.create_directory(self.user_dir)
        with open(self.user_dir / "main.ets", "w") as f:
            f.write(
                f'import * as hello from "hello";\n'
                f"\n"
                f'loadLibrary("{self.lib_name}");\n'
                f"\n"
                f"function main() {{\n"
                f"    hello.sayHello();\n"
                f"}}\n"
            )

    def create_user_cpp(self) -> None:
        """Create a simple example user source file."""
        self.create_directory(self.user_src_dir)
        with open(self.user_dir / "compile_flags.txt", "w") as f:
            for user_include_dir in self.user_includes:
                f.write(f"-I{user_include_dir}\n")
        with open(self.user_src_dir / "main.cpp", "w") as f:
            f.write(
                f'#include "hello.user.hpp"\n'
                f"\n"
                f"int main() {{\n"
                f"    hello::sayHello();\n"
                f"    return 0;\n"
                f"}}\n"
            )

    def taihec(
        self,
        dst_dir: Path,
        src_files: list[Path],
        backend_names: list[str],
        cmake: bool = False,
        sts_keep_name: bool = False,
        arkts_module_prefix: str | None = None,
        arkts_path_prefix: str | None = None,
    ) -> None:
        registry = BackendRegistry()
        registry.register_all()
        backends = registry.collect_required_backends(backend_names)
        resolved_backends = [b() for b in backends]

        if cmake:
            output_config = CMakeOutputConfig(
                dst_dir=dst_dir,
                runtime_include_dir=RuntimeHeader.resolve_path(),
                runtime_src_dir=RuntimeSource.resolve_path(),
            )
        else:
            output_config = OutputConfig(
                dst_dir=dst_dir,
            )

        invocation = CompilerInvocation(
            src_files=src_files,
            output_config=output_config,
            backends=resolved_backends,
            sts_keep_name=sts_keep_name,
            arkts_module_prefix=arkts_module_prefix,
            arkts_path_prefix=arkts_path_prefix,
        )
        instance = CompilerInstance(invocation)
        if not instance.run():
            raise RuntimeError("Taihe compiler (taihec) failed to run")

    def generate(self, sts_keep_name: bool, cmake: bool) -> None:
        """Generate code from IDL files."""
        if not self.idl_dir.is_dir():
            raise FileNotFoundError(f"IDL directory not found: '{self.idl_dir}'")

        self.clean_directory(self.generated_dir)

        self.logger.info("Generating author and ani codes...")

        lib_files: list[Path] = []

        backend_names = ["cpp-author"]
        if self.user == UserType.STS:
            backend_names.append("ani-bridge")
            lib_files.append(
                StandardLibrary.resolve_path() / "taihe.platform.ani.taihe"
            )
        if self.user == UserType.CPP:
            backend_names.append("cpp-user")
        if self.should_run_pretty_print:
            backend_names.append("pretty-print")

        # Generate taihe stdlib codes
        self.taihec(
            dst_dir=self.generated_dir,
            src_files=lib_files,
            backend_names=["abi-source", "cpp-common"],
        )

        # Generate author codes
        self.taihec(
            dst_dir=self.generated_dir,
            src_files=list(self.idl_dir.glob("*.taihe")),
            backend_names=backend_names,
            cmake=cmake,
            sts_keep_name=sts_keep_name,
        )

    def build(self, opt_level: str) -> None:
        """Run the complete build process."""
        self.logger.info("Starting ANI compilation...")

        self.setup_build_directories()

        if self.user == UserType.STS:
            # Compile the shared library
            self.compile_shared_library(opt_level=opt_level)

            # Compile and link ABC files
            self.compile_and_link_ani()

            # Run with Ark runtime
            self.run_ani()
        elif self.user == UserType.CPP:
            # Compile the shared library
            self.compile_shared_library(opt_level=opt_level)

            # Compile the executable
            self.compile_and_link_exe(opt_level=opt_level)

            # Run the executable
            self.run_exe()

        self.logger.info("Build and execution completed successfully")

    def compile_shared_library(self, opt_level: str):
        """Compile the shared library."""
        self.logger.info("Compiling shared library...")

        runtime_src_dir = RuntimeSource.resolve_path()
        runtime_sources = [
            runtime_src_dir / "string.cpp",
            runtime_src_dir / "object.cpp",
        ]
        if self.user == UserType.STS:
            runtime_sources.append(runtime_src_dir / "runtime.cpp")

        # Compile each component
        runtime_objects = self.compile(
            self.build_runtime_src_dir,
            runtime_sources,
            self.runtime_includes,
            compile_flags=[f"-O{opt_level}"],
        )
        generated_objects = self.compile(
            self.build_generated_src_dir,
            self.generated_src_dir.glob("*.[cC]*"),
            self.generated_includes,
            compile_flags=[f"-O{opt_level}"],
        )
        author_objects = self.compile(
            self.build_author_src_dir,
            self.author_src_dir.glob("*.[cC]*"),
            self.author_includes,
            compile_flags=[f"-O{opt_level}"],
        )

        # Link all objects
        if all_objects := runtime_objects + generated_objects + author_objects:
            self.link(
                self.so_target,
                all_objects,
                shared=True,
                link_options=["-Wl,--no-undefined"],
            )
            self.logger.info("Shared library compiled: %s", self.so_target)
        else:
            self.logger.warning(
                "No object files to link, skipping shared library compilation"
            )

    def compile_and_link_exe(self, opt_level: str) -> None:
        """Compile and link the executable."""
        self.logger.info("Compiling and linking executable...")

        # Compile the user source files
        user_objects = self.compile(
            self.build_user_dir,
            self.user_src_dir.glob("*.[cC]*"),
            self.user_includes,
            compile_flags=[f"-O{opt_level}"],
        )

        # Link the executable
        if user_objects:
            self.link(
                self.exe_target,
                [self.so_target, *user_objects],
            )
            self.logger.info("Executable compiled: %s", self.so_target)
        else:
            self.logger.warning(
                "No object files to link, skipping executable compilation"
            )

    def run_exe(self) -> None:
        """Run the compiled executable."""
        self.logger.info("Running executable...")

        elapsed_time = self.run(
            self.exe_target,
            self.so_target.parent,
        )

        self.logger.info("Done, time = %f s", elapsed_time)

    def compile_and_link_ani(self):
        """Compile and link ABC files."""
        self.logger.info("Compiling and linking ABC files...")

        paths: dict[str, Path] = {}
        for path in self.generated_dir.glob("*.ets"):
            paths[path.stem] = path
        for path in self.user_dir.glob("*.ets"):
            paths[path.stem] = path

        self.create_arktsconfig(self.arktsconfig_file, paths)

        # Compile ETS files in each directory
        generated_abc = self.compile_abc(
            self.build_generated_dir,
            self.generated_dir.glob("*.ets"),
            self.arktsconfig_file,
        )
        user_abc = self.compile_abc(
            self.build_user_dir,
            self.user_dir.glob("*.ets"),
            self.arktsconfig_file,
        )

        # Link all ABC files
        if all_abc_files := generated_abc + user_abc:
            self.link_abc(
                self.abc_target,
                all_abc_files,
            )
            self.logger.info("ABC files linked: %s", self.abc_target)
        else:
            self.logger.warning("No ABC files to link, skipping ABC compilation")

    def run_ani(self) -> None:
        """Run the compiled ABC file with the Ark runtime."""
        self.logger.info("Running ABC file with Ark runtime...")

        elapsed_time = self.run_abc(
            self.abc_target,
            self.so_target.parent,
            entry="main.ETSGLOBAL::main",
        )

        self.logger.info("Done, time = %f s", elapsed_time)

    def setup_build_directories(self) -> None:
        """Set up necessary build directories."""
        # Clean and create directories
        self.clean_directory(self.build_dir)

        self.create_directory(self.build_dir)
        self.create_directory(self.build_runtime_src_dir)
        self.create_directory(self.build_generated_src_dir)
        self.create_directory(self.build_author_src_dir)
        self.create_directory(self.build_generated_dir)
        self.create_directory(self.build_user_dir)

    def compile(
        self,
        output_dir: Path,
        input_files: Iterable[Path],
        include_dirs: Sequence[Path] = (),
        compile_flags: Sequence[str] = (),
    ) -> list[Path]:
        """Compile source files."""
        output_files: list[Path] = []

        for input_file in input_files:
            name = input_file.name
            output_file = output_dir / f"{name}.o"

            if name.endswith(".c"):
                compiler = self.config.cc
                std = "gnu11"
            else:
                compiler = self.config.cxx
                std = "gnu++17"

            command = [
                compiler,
                "-c",
                "-fvisibility=hidden",
                "-fPIC",
                # "-Wall",
                # "-Wextra",
                f"-std={std}",
                "-o",
                output_file,
                input_file,
                *compile_flags,
            ]

            for include_dir in include_dirs:
                if include_dir.exists():  # Only include directories that exist
                    command.append(f"-I{include_dir}")

            self.run_command(command)

            output_files.append(output_file)

        return output_files

    def link(
        self,
        output_file: Path,
        input_files: Sequence[Path],
        shared: bool = False,
        link_options: Sequence[str] = (),
    ) -> None:
        """Link object files."""
        if len(input_files) == 0:
            self.logger.warning("No input files to link")
            return

        command = [
            self.config.cxx,
            "-fPIC",
            "-o",
            output_file,
            *input_files,
            *link_options,
        ]

        if shared:
            command.append("-shared")

        self.run_command(command)

    def run(
        self,
        target: Path,
        ld_lib_path: Path,
        args: Sequence[str] = (),
    ) -> float:
        """Run the compiled target."""
        command = [
            target,
            *args,
        ]

        return self.run_command(
            command,
            env={"LD_LIBRARY_PATH": ld_lib_path},
        )

    def create_arktsconfig(
        self,
        arktsconfig_file: Path,
        app_paths: dict[str, Path] | None = None,
    ) -> None:
        """Create ArkTS configuration file."""
        vm = PandaVm.resolve()
        paths = vm.stdlib_sources | (app_paths or {})

        config_content = {
            "compilerOptions": {
                "baseUrl": str(vm.host_tools_dir),
                "paths": {key: [str(value)] for key, value in paths.items()},
            }
        }

        with open(arktsconfig_file, "w") as json_file:
            json.dump(config_content, json_file, indent=2)

        self.logger.debug("Created configuration file at: %s", arktsconfig_file)

    def compile_abc(
        self,
        output_dir: Path,
        input_files: Iterable[Path],
        arktsconfig_file: Path,
    ) -> list[Path]:
        """Compile ETS files to ABC format."""
        output_files: list[Path] = []

        for input_file in input_files:
            name = input_file.name
            output_file = output_dir / f"{name}.abc"
            output_dump = output_dir / f"{name}.abc.dump"

            gen_abc_command = [
                self.config.vm.tool("es2panda"),
                input_file,
                "--output",
                output_file,
                "--extension",
                "ets",
                "--arktsconfig",
                arktsconfig_file,
            ]

            self.run_command(gen_abc_command)

            output_files.append(output_file)

            ark_disasm_path = self.config.vm.tool("ark_disasm")
            if not ark_disasm_path.exists():
                self.logger.warning(
                    "ark_disasm not found at %s, skipping disassembly", ark_disasm_path
                )
                continue

            gen_abc_dump_command = [
                ark_disasm_path,
                output_file,
                output_dump,
            ]

            self.run_command(gen_abc_dump_command)

        return output_files

    def link_abc(
        self,
        target: Path,
        input_files: Sequence[Path],
    ) -> None:
        """Link ABC files."""
        if len(input_files) == 0:
            self.logger.warning("No input files to link")
            return

        command = [
            self.config.vm.tool("ark_link"),
            "--output",
            target,
            "--",
            *input_files,
        ]

        self.run_command(command)

    def run_abc(
        self,
        abc_target: Path,
        ld_lib_path: Path,
        entry: str,
    ) -> float:
        """Run the compiled ABC file with the Ark runtime."""
        ark_path = self.config.vm.tool("ark")
        etsstdlib_path = self.config.vm.stdlib_lib

        command = [
            ark_path,
            f"--boot-panda-files={etsstdlib_path}",
            f"--load-runtimes=ets",
            abc_target,
            entry,
        ]

        return self.run_command(
            command,
            env={"LD_LIBRARY_PATH": ld_lib_path},
        )


class RepositoryUpgrader(BuildUtils):
    """Upgrade the code from a specified URL."""

    def __init__(self, repo_url: str, config: BuildConfig):
        super().__init__()
        self.config = config
        self.repo_url = repo_url

    def fetch_and_upgrade(self):
        filename = self.repo_url.split("/")[-1]
        version = self.repo_url.split("/")[-2]

        base_dir = ResourceContext.instance().base_dir
        extract_dir = base_dir / "../tmp"
        target_file = extract_dir / filename
        self.create_directory(extract_dir)
        fetch_url(self.repo_url, target_file)
        self.extract_file(target_file, extract_dir)

        tmp_taihe_pkg_dir = extract_dir / "taihe"
        self.clean_directory(base_dir)
        self.move_directory(tmp_taihe_pkg_dir, base_dir)
        self.clean_directory(extract_dir)

        self.logger.info("Successfully upgraded code to version %s", version)


class TaiheTryitParser(argparse.ArgumentParser):
    """Parser for the Taihe Tryit CLI."""

    def register_common_configs(self) -> None:
        self.add_argument(
            "-v",
            "--verbose",
            action="count",
            default=0,
            help="Increase verbosity (can be used multiple times)",
        )

    def register_project_configs(self) -> None:
        self.add_argument(
            "target_directory",
            type=str,
            nargs="?",
            default=".",
            help="The target directory containing source files for the project",
        )
        self.add_argument(
            "-u",
            "--user",
            type=UserType,
            choices=list(UserType),
            required=True,
            help="User type for the build system (ani/cpp)",
        )

    def register_build_configs(self) -> None:
        self.add_argument(
            "-O",
            "--optimization",
            type=str,
            nargs="?",
            default="0",
            const="0",
            help="Optimization level for compilation (0-3)",
        )

    def register_generate_configs(self) -> None:
        self.add_argument(
            "--sts-keep-name",
            action="store_true",
            help="Keep original function and interface method names",
        )
        self.add_argument(
            "--cmake",
            action="store_true",
            help="Generate CMake files for the project",
        )

    def register_update_configs(self) -> None:
        self.add_argument(
            "URL",
            type=str,
            help="The URL to fetch the code from",
        )


def main():
    parser = TaiheTryitParser(
        prog="taihe-tryit",
        description="Build and run project from a target directory",
    )
    parser.register_common_configs()

    subparsers = parser.add_subparsers(dest="command", required=True)

    parser_create = subparsers.add_parser(
        "create",
        help="Create a simple example",
    )
    parser_create.register_common_configs()
    parser_create.register_project_configs()

    parser_generate = subparsers.add_parser(
        "generate",
        help="Generate code from the target directory",
    )
    parser_generate.register_common_configs()
    parser_generate.register_project_configs()
    parser_generate.register_generate_configs()

    parser_build = subparsers.add_parser(
        "build",
        help="Build the project from the target directory",
    )
    parser_build.register_common_configs()
    parser_build.register_project_configs()
    parser_build.register_build_configs()

    parser_test = subparsers.add_parser(
        "test",
        help="Generate and build the project from the target directory",
    )
    parser_test.register_common_configs()
    parser_test.register_project_configs()
    parser_test.register_build_configs()
    parser_test.register_generate_configs()

    parser_upgrade = subparsers.add_parser(
        "upgrade",
        help="Upgrade using the specified URL",
    )
    parser_upgrade.register_common_configs()
    parser_upgrade.register_update_configs()

    ResourceContext.register_cli_options(parser)
    args = parser.parse_args()
    ResourceContext.initialize(args)

    match args.verbose:
        case 0:
            verbosity = logging.INFO
        case 1:
            verbosity = logging.DEBUG
        case 2:
            verbosity = TRACE_CONCISE
        case _:
            verbosity = TRACE_VERBOSE
    setup_logger(verbosity)

    user = cast(UserType, args.user)
    vm = PandaVm.resolve() if user == UserType.STS else None
    config = BuildConfig(vm)

    try:
        if args.command == "upgrade":
            upgrader = RepositoryUpgrader(args.URL, config=config)
            upgrader.fetch_and_upgrade()
        else:
            build_system = BuildSystem(
                args.target_directory,
                args.user,
                config=config,
                verbosity=verbosity,
            )
            if args.command == "create":
                build_system.create()
            if args.command in ("generate", "test"):
                build_system.generate(
                    sts_keep_name=args.sts_keep_name,
                    cmake=args.cmake,
                )
            if args.command in ("build", "test"):
                build_system.build(
                    opt_level=args.optimization,
                )
    except KeyboardInterrupt:
        print("\nInterrupted. Exiting build process.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()