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

from taihe.utils.outputs import DEFAULT_INDENT, FileWriter, OutputManager


class StsWriter(FileWriter):
    """Represents a static type script (sts) file."""

    @override
    def __init__(self, om: OutputManager, path: str, indent_unit: str = DEFAULT_INDENT):
        super().__init__(om, path=path, indent_unit=indent_unit)
        self.import_dict: dict[str, str] = {}

    @override
    def write_prologue(self, f: TextIO):
        for import_name, module_name in self.import_dict.items():
            f.write(f'import * as {import_name} from "./{module_name}";\n')

    def import_module(self, import_name: str, module_name: str):
        self.import_dict.setdefault(import_name, module_name)