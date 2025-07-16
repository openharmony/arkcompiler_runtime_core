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

import re
import subprocess
import sys
from datetime import datetime, timezone
from functools import cache
from pathlib import Path
from typing import Any, TextIO

from hatchling.builders.hooks.plugin.interface import BuildHookInterface

g_compiler_dir = Path(__file__).parent
g_repo_dir = g_compiler_dir.parent
sys.path.insert(0, str(g_compiler_dir))
from taihe.utils.resources import Antlr, ResourceContext  # noqa: E402

ANTLR_PKG = "taihe.parse.antlr"


@cache
def get_parser():
    from taihe.parse.antlr.TaiheParser import TaiheParser

    return TaiheParser


def get_hint(attr_kind):
    if attr_kind.endswith("Lst"):
        return f'List["TaiheAST.{attr_kind[:-3]}"]'
    if attr_kind.endswith("Opt"):
        return f'Optional["TaiheAST.{attr_kind[:-3]}"]'
    return f'"TaiheAST.{attr_kind}"'


def get_attr_pairs(ctx):
    for attr_full_name in ctx.__dict__:
        if not attr_full_name.startswith("_") and attr_full_name != "parser":
            yield attr_full_name.split("_", 1)


def snake_case(name):
    """Convert CamelCase to snake_case."""
    return re.sub(r"(?<!^)(?=[A-Z])", "_", name).lower()


class Inspector:
    def __init__(self):
        self.parentCtx = None
        self.invokingState = None
        self.children = None
        self.start = None
        self.stop = None


def generate_ast(file: TextIO, parser: Any):
    file.write(
        "from dataclasses import dataclass\n"
        "from typing import Any, Union, List, Optional\n"
        "\n"
        "from taihe.utils.sources import SourceLocation\n"
        "\n"
        "\n"
        "class TaiheAST:\n"
        "    @dataclass(kw_only=True)\n"
        "    class any:\n"
        "        loc: SourceLocation\n"
        "\n"
        "        def _accept(self, visitor) -> Any:\n"
        "            raise NotImplementedError()\n"
        "\n"
        "\n"
        "    @dataclass\n"
        "    class TOKEN(any):\n"
        "        text: str\n"
        "\n"
        "        def __str__(self):\n"
        "            return self.text\n"
        "\n"
        "        def _accept(self, visitor) -> Any:\n"
        "            return visitor.visit_token(self)\n"
        "\n"
    )
    type_list = []
    for rule_name in parser.ruleNames:
        node_kind = rule_name[0].upper() + rule_name[1:]
        ctx_kind = node_kind + "Context"
        ctx_type = getattr(parser, ctx_kind)
        type_list.append((node_kind, ctx_type))
    for node_kind, ctx_type in type_list:
        subclasses = ctx_type.__subclasses__()
        if subclasses:
            file.write(f"    {node_kind} = Union[\n")
            for sub_type in subclasses:
                sub_kind = sub_type.__name__
                attr_kind = sub_kind[:-7]
                attr_hint = get_hint(attr_kind)
                type_list.append((attr_kind, sub_type))
                file.write(f"        {attr_hint},\n")
            file.write("    ]\n\n")
        else:
            ctx = ctx_type(None, Inspector())
            file.write(f"    @dataclass\n    class {node_kind}(any):\n")
            for attr_kind, attr_name in get_attr_pairs(ctx):
                attr_hint = get_hint(attr_kind)
                file.write(f"        {attr_name}: {attr_hint}\n")
            file.write(
                f"\n"
                f"        def _accept(self, visitor) -> Any:\n"
                f"            return visitor.visit_{snake_case(node_kind)}(self)\n"
                f"\n"
            )


def generate_visitor(file: TextIO, parser: Any):
    file.write(
        f"from {ANTLR_PKG}.TaiheAST import TaiheAST\n"
        f"\n"
        f"from typing import Any\n"
        f"\n"
        f"\n"
        f"class TaiheVisitor:\n"
        f"    def visit(self, node: TaiheAST.any) -> Any:\n"
        f"        return node._accept(self)\n"
        f"\n"
        f"    def visit_token(self, node: TaiheAST.TOKEN) -> Any:\n"
        f"        raise NotImplementedError()\n"
        f"\n"
    )
    type_list = []
    for rule_name in parser.ruleNames:
        node_kind = rule_name[0].upper() + rule_name[1:]
        ctx_kind = node_kind + "Context"
        ctx_type = getattr(parser, ctx_kind)
        type_list.append((node_kind, ctx_type))
    for node_kind, ctx_type in type_list:
        subclasses = ctx_type.__subclasses__()
        if subclasses:
            for sub_type in subclasses:
                sub_kind = sub_type.__name__
                attr_kind = sub_kind[:-7]
                file.write(
                    f"    def visit_{snake_case(attr_kind)}(self, node: TaiheAST.{attr_kind}) -> Any:\n"
                    f"        return self.visit_{snake_case(node_kind)}(node)\n"
                    f"\n"
                )
        file.write(
            f"    def visit_{snake_case(node_kind)}(self, node: TaiheAST.{node_kind}) -> Any:\n"
            f"        raise NotImplementedError()\n"
            f"\n"
        )


def has_generated(grammar_input: Path, parser_output: Path) -> bool:
    return (
        parser_output.exists()
        and grammar_input.stat().st_mtime < parser_output.stat().st_mtime
    )


class TaiheBuildHook(BuildHookInterface):
    """Hatch build hook for generating ANTLR-based AST and visitor classes."""

    PLUGIN_NAME = "taihe-build"

    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        # Build modes
        #
        # | Input | Target | Has Git? | Artifacts |
        # |-------+--------+----------+-----------|
        # | Git   | sdist  | Y        | Generate  |
        # | Git   | wheel  | Y        | Generate  |
        # | sdist | wheel  | N        | Reuse     |
        del version

        # Setup paths first.
        self.repo_root = g_repo_dir
        self.version_path = g_compiler_dir / "taihe/_version.py"

        self.antlr_in = g_compiler_dir / "Taihe.g4"
        self.antlr_dir = g_compiler_dir / ANTLR_PKG.replace(".", "/")
        self.antlr_out_example = self.antlr_dir / "TaiheParser.py"

        # Always bundle artifacts.
        self._setup_artifacts(build_data)

        # Only generate artifacts for in-tree build.
        if not self._is_inside_git_repo():
            return

        # Now generate version.py and antlr.
        self._generate_version()
        if has_generated(self.antlr_in, self.antlr_out_example):
            print("ANTLR: skipping generation, already generated")
        else:
            print("ANTLR: generating...")
            self._generate_grammar()

    def _setup_artifacts(self, build_data: dict[str, Any]):
        build_data["artifacts"] += [
            f"{self.version_path.relative_to(g_repo_dir)}",
            f"{self.antlr_dir.relative_to(g_repo_dir)}/*.py",
        ]

    def _is_inside_git_repo(self) -> bool:
        try:
            self._git("rev-parse", "--is-inside-work-tree")
            print(f"Build environment: git -> {self.target_name}")
            return True
        except subprocess.CalledProcessError:
            print(f"Build environment: ??? -> {self.target_name}")
        except FileNotFoundError:
            print("No git installed?")
        return False

    def _git(self, *args: str) -> str:
        return subprocess.run(
            ["git", *args],
            capture_output=True,
            text=True,
            check=True,
            cwd=self.repo_root,
        ).stdout

    def _generate_version(self):
        git_commit = self._git("rev-parse", "HEAD").strip()
        git_message = self._git("log", "-1", "--pretty=%B").splitlines()[0]

        now = datetime.now()
        build_timestamp = now.astimezone(timezone.utc).timestamp()
        build_date = now.strftime("%Y%m%d")

        self.version_path.write_text(
            f"""version = {self.metadata.version!r}
git_commit = {git_commit!r}
git_message = {git_message!r}
build_date = {build_date!r}
build_time_utc = {build_timestamp!r}
"""
        )

    def _generate_grammar(self):
        ResourceContext.initialize()
        antlr_opts = ["-Dlanguage=Python3", "-no-listener"]
        antlr_opts += [str(self.antlr_in), "-o", str(self.antlr_dir)]
        Antlr.resolve().run_tool(antlr_opts)

        p = get_parser()
        with open(self.antlr_dir / "TaiheAST.py", "w") as f:
            generate_ast(f, p)
        with open(self.antlr_dir / "TaiheVisitor.py", "w") as f:
            generate_visitor(f, p)