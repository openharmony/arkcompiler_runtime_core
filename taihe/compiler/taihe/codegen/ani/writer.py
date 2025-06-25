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

from typing import TextIO

from typing_extensions import override

from taihe.utils.outputs import DEFAULT_INDENT, FileKind, FileWriter, OutputManager


class StsWriter(FileWriter):
    """Represents a static type script (sts) file."""

    @override
    def __init__(
        self,
        om: OutputManager,
        relative_path: str,
        file_kind: FileKind,
        indent_unit: str = DEFAULT_INDENT,
    ):
        super().__init__(
            om,
            relative_path=relative_path,
            file_kind=file_kind,
            default_indent=indent_unit,
            comment_prefix="// ",
        )
        self.import_dict: dict[str, tuple[str, str | None]] = {}

    @override
    def write_prologue(self, f: TextIO):
        f.write("'use static'\n")
        for import_name, decl_pair in self.import_dict.items():
            module_name, decl_name = decl_pair
            if decl_name is None:
                import_str = f"* as {import_name}"
            elif decl_name == "default":
                import_str = import_name
            elif decl_name == import_name:
                import_str = f"{{{decl_name}}}"
            else:
                import_str = f"{{{decl_name} as {import_name}}}"
            f.write(f"import {import_str} from '{module_name}';\n")

    def add_import_module(
        self,
        module_name: str,
        import_name: str,
    ):
        self._add_import(import_name, (module_name, None))

    def add_import_default(
        self,
        module_name: str,
        import_name: str,
    ):
        self._add_import(import_name, (module_name, "default"))

    def add_import_decl(
        self,
        module_name: str,
        decl_name: str,
        import_name: str | None = None,
    ):
        if import_name is None:
            import_name = decl_name
        self._add_import(import_name, (module_name, decl_name))

    def _add_import(
        self,
        import_name: str,
        new_pair: tuple[str, str | None],
    ):
        import_dict = self.import_dict
        old_pair = import_dict.setdefault(import_name, new_pair)
        if old_pair != new_pair:
            raise ValueError(
                f"Duplicate import for {import_name!r}: {old_pair} vs {new_pair}"
            )