#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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
#

from __future__ import annotations

import difflib
from dataclasses import dataclass
from io import TextIOWrapper
import json
from os import path, makedirs
from pathlib import Path
import re
from typing import Any, Callable, Iterable, List, Optional, TypeVar

from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.test_file_based import TestFileBased

ListOrDict = TypeVar("ListOrDict", list, dict)

@dataclass
class EvaluateOptions:
    line: int
    source_path: str
    panda_files_paths: List[str]

class TestEvaluate(TestFileBased):
    # `difflib` different strings are returned with either `-` or `+` as the first symbol.
    # Must filter out all non-diff strings
    NON_DIFF_MARKS = set([' ', '?'])
    EVAL_PATCH_FUNCTION_NAME = 'evalPatchFunction'
    EVAL_PATCH_RETURN_VALUE = 'evalGeneratedResult'
    EVAL_METHOD_GENERATION_PATTERN = r'^[a-zA-Z\-_\d]*_[\d]+_eval'
    EVAL_METHOD_GENERATED_NAME_RE = re.compile(EVAL_METHOD_GENERATION_PATTERN + r'$')
    EVAL_METHOD_RETURN_VALUE_RE = re.compile(EVAL_METHOD_GENERATION_PATTERN + r'_generated_var$')

    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        super().__init__(test_env, test_path, flags, test_id)

        self.metadata: TestMetadata = get_metadata(Path(test_path))
        assert isinstance(self.metadata.params, dict)

        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind = None

        test_id_as_path = Path(self.test_id)
        self.bytecode_path: Path = test_env.work_dir.intermediate
        # Must get the directory of the test, as it represents the test's name.
        makedirs(self.bytecode_path / test_id_as_path.parents[0], exist_ok=True)

        self.base_filename = Path(test_id_as_path.stem).stem
        self.base_abc = str(self.bytecode_path / f"{self.test_id}.abc")

        self.base_path = self.__class__.get_file_base_name(self.path)
        self.patch_path = f"{self.base_path}.patch.ets"
        self.patch_abc = str(self.bytecode_path / f"{self.test_id}.patch.abc")
        self.patch_disasm = str(self.bytecode_path / f"{self.test_id}.patch.disasm")
        self.expected_path = f"{self.base_path}.expected.ets"
        self.expected_abc = str(self.bytecode_path / f"{self.test_id}.expected.abc")
        self.expected_disasm = str(self.bytecode_path / f"{self.test_id}.expected.disasm")
        self._eval_method_name: str | None = None

        self.expected_func_pattern = r"^\.function [a-zA-Z\d\.]+ " +\
            TestEvaluate.EVAL_PATCH_FUNCTION_NAME +\
            r"\." +\
            TestEvaluate.EVAL_PATCH_FUNCTION_NAME +\
            r"\." +\
            TestEvaluate.EVAL_PATCH_FUNCTION_NAME +\
            r"\(\)"

    @staticmethod
    def get_file_base_name(target_path: str) -> str:
        return path.splitext(path.splitext(target_path)[0])[0]

    @property
    def is_negative_compile(self) -> bool:
        """ True if a test is expected to fail on es2panda """
        return self.metadata.tags.negative

    @property
    def is_valid_test(self) -> bool:
        """ True if a test is valid """
        return not self.metadata.tags.not_a_test

    @property
    def evaluate_line(self) -> int:
        return self.metadata.params["evaluate_line"]

    @property
    def expected_file_imports_base(self) -> bool:
        return bool(self.metadata.params.get("expected_imports_base", False))

    @property
    def disable_ast_comparison(self) -> bool:
        return bool(self.metadata.params.get("disable_ast_comparison", False))

    @property
    def disable_asm_comparison(self) -> bool:
        return bool(self.metadata.params.get("disable_asm_comparison", False))

    @property
    def eval_method_name(self) -> str:
        assert self._eval_method_name
        return self._eval_method_name

    @property
    def patch_func_pattern(self) -> str:
        # Methods cannot have dots in name, replace is not required.
        return r"^\.function [a-zA-Z\d\.]+ " +\
            self.eval_method_name +\
            r"\." +\
            self.eval_method_name +\
            r"\." +\
            self.eval_method_name +\
            r"\(\)"

    # pylint: disable=too-many-return-statements
    def do_run(self) -> TestEvaluate:
        # NOTE(dslynko): must support cases with multiple context files
        self._run_compiler(self.path, self.base_abc, dump_ast=False)
        if not self.passed:
            return self

        self._run_compiler(self.expected_path, self.expected_abc, ets_module=True, dump_ast=True)
        if not self.passed:
            return self
        expected_output = self.report.output

        self._run_disasm(self.expected_abc, self.expected_disasm)
        if not self.passed:
            return self
        
        self._run_compiler(
            self.patch_path,
            self.patch_abc,
            ets_module=True,
            dump_ast=True,
            eval_options=self._create_evaluate_options(),
            log_level="warning"
        )
        if not self.passed:
            return self
        patch_output = self.report.output

        self._run_disasm(self.patch_abc, self.patch_disasm)
        if not self.passed:
            return self

        if not self.disable_ast_comparison:
            self._compare_ast(patch_output, expected_output)
            if not self.passed:
                return self
        else:
            # Must initialize evaluation method name.
            splitted_patched_output = self._initialize_eval_method_name(patch_output)
            if splitted_patched_output is None:
                return self

        if not self.disable_asm_comparison:
            self._compare_bytecode()
        return self

# ==========================================================================================================
# AST comparison 
# ===========================================================================================================

    def _compare_ast(self, patched_output: str, expected_output: str) -> None:
        """
        Compares two AST dumps modulo nodes locations.
        Import specifiers are compared regardless of the order in AST.
        :param patched_output: AST dump of tested code
        :param expected_output: AST dump of expected logically-equal code
        """
        expected_output = self._filter_expected_ast(expected_output)
        expected_output, expected_imports = self.__class__._prepare_ast(json.loads(expected_output))

        splitted_patched_output = self._initialize_eval_method_name(patched_output)
        if splitted_patched_output is None:
            return
        patched_output, patched_imports = splitted_patched_output

        if not self._compare_ast_statements(patched_output, expected_output):
            return
        self._compare_ast_import_decls(patched_imports, expected_imports)

    def _compare_ast_statements(self, patched_output: str, expected_output: str) -> bool:
        diff = difflib.ndiff(patched_output.splitlines(), expected_output.splitlines())
        # Find strings indicating differences other than IR locations.
        error_list = [
            x for x in diff
            if x[0] not in self.__class__.NON_DIFF_MARKS
        ]
        if error_list:
            self.passed = False
            self.fail_kind = FailKind.COMPARE_FAIL
            error_report = "AST global statements comparison failed:\n" + "\n".join(error_list)
            self.report = TestReport("", error_report, 0)
            return False
        
        return True

    def _compare_ast_import_decls(self, patched_imports: str, expected_imports: str) -> bool:
        patch_specifiers_list: list[dict] = []
        expected_specifiers_map: dict = {}

        patch_obj: list[dict] = list(json.loads(patched_imports))
        expected_obj: list[dict] = list(json.loads(expected_imports))

        for patch_import_decl in patch_obj:
            specifiers_list = patch_import_decl.get("specifiers")
            for s in specifiers_list:
                patch_specifiers_list.append(s)
        
        for expected_import_decl in expected_obj:
            specifiers: list[dict] = expected_import_decl.get("specifiers")
            for s in specifiers:
                local_import_name = s.get("local").get("name")
                expected_specifiers_map[local_import_name] = s

        if len(expected_specifiers_map) != len(patch_specifiers_list):
            self.passed = False
            self.fail_kind = FailKind.COMPARE_FAIL
            error_report = f"Imports expected size {len(expected_specifiers_map)}"\
                            f" do not match with patch imports size {len(patch_specifiers_list)}"
            self.report = TestReport("", error_report, 0)
            return False

        for specifier in patch_specifiers_list:
            local_import_name = specifier.get("local").get("name")
            expected_specifier = expected_specifiers_map.get(local_import_name, None)
            if expected_specifier == None:
                self.passed = False
                self.fail_kind = FailKind.COMPARE_FAIL
                error_report = f"Patch import specifier {local_import_name} do not contained in expected specifiers"
                self.report = TestReport("", error_report, 0)
                return False

            diff = difflib.ndiff(str(specifier).splitlines(), str(expected_specifier).splitlines())
            error_list = [
                x for x in diff
                    if x[0] not in self.__class__.NON_DIFF_MARKS
            ]
            if error_list:
                self.passed = False
                self.fail_kind = FailKind.COMPARE_FAIL
                error_report = "AST import specifiers comparison failed:\n" + "\n".join(error_list)
                self.report = TestReport("", error_report, 0)
                return False

        return True

    def _initialize_eval_method_name(self, patched_output: str) -> tuple[str, str] | None:
        eval_method_names: set[str] = set()
        def replace_eval_method_name(obj: ListOrDict) -> ListOrDict:
            if isinstance(obj, dict):
                for key, value in obj.copy().items():
                    if key == 'name' and isinstance(value, str):
                        if TestEvaluate.EVAL_METHOD_GENERATED_NAME_RE.match(value):
                            eval_method_names.add(value)
                            obj[key] = TestEvaluate.EVAL_PATCH_FUNCTION_NAME
                        elif TestEvaluate.EVAL_METHOD_RETURN_VALUE_RE.match(value):
                            obj[key] = TestEvaluate.EVAL_PATCH_RETURN_VALUE
                    else:
                        obj[key] = replace_eval_method_name(value)
            elif isinstance(obj, list):
                obj = [replace_eval_method_name(value) for value in obj.copy()]
            return obj

        patched_output, patched_imports = TestEvaluate._prepare_ast(
            json.loads(patched_output),
            additional_filters=[replace_eval_method_name],
        )

        eval_method_names_list = list(eval_method_names)
        if len(eval_method_names_list) != 1 or eval_method_names_list[0] == "":
            self.passed = False
            self.fail_kind = FailKind.COMPARE_FAIL
            error_report = "Failed to find expected evaluation method name"
            self.report = TestReport("", error_report, 0)
            return None

        self._eval_method_name = eval_method_names_list[0]
        return patched_output, patched_imports

    @staticmethod
    def _prepare_ast(obj: dict[str, Any], additional_filters: list[Callable[[ListOrDict], ListOrDict]] | None = None):
        obj = TestEvaluate._del_locations(obj)
        if additional_filters:
            for f in additional_filters:
                obj = f(obj)

        imports_list = []
        statements_list = obj.get("statements", None)

        def filter_lambda(statement: dict[str, str]) -> bool:
            if statement.get("type") == "ImportDeclaration":
                imports_list.append(statement)
                return False
            return True
    
        # Collect import declarations in imports_list and remove from AST.
        filtered_list = filter(filter_lambda, statements_list)

        return json.dumps(list(filtered_list), indent=4), json.dumps(list(imports_list), indent=4)

    @staticmethod
    def _del_locations(obj: ListOrDict) -> ListOrDict:
        if isinstance(obj, dict):
            for key, value in obj.copy().items():
                if key == 'loc':
                    del obj[key]
                else:
                    obj[key] = TestEvaluate._del_locations(value)
        elif isinstance(obj, list):
            obj = [TestEvaluate._del_locations(value) for value in obj.copy()]
        return obj

    @staticmethod
    def _import_statements_sources_filter(paths_replacement_map: dict[str, str]) -> Callable[[dict], dict]:
        def imports_filter(stmt: dict) -> dict:
            if "type" not in stmt or stmt["type"] != "ImportDeclaration":
                return stmt

            assert "source" in stmt and "value" in stmt["source"]
            import_path: str = stmt["source"]["value"]
            if (pos := import_path.rfind("/")) != -1:
                if (new_import_path := paths_replacement_map.get(import_path[pos+1:])) is not None:
                    stmt["source"]["value"] = new_import_path
            return stmt

        return imports_filter

    # Saves all statements for which the callback returns True.
    def _filter_expected_ast(self, expected_output: str) -> str:
        as_dict: dict = self.__class__._del_locations(json.loads(expected_output))
        if self.expected_file_imports_base:
            statements_filter = self.__class__._import_statements_sources_filter({self.base_filename: ""})

            for key, value in as_dict.items():
                if key != "statements":
                    continue

                assert isinstance(value, list)
                as_dict[key] = [statements_filter(stmt) for stmt in value]

        as_str = json.dumps(as_dict, indent=4)
        return as_str

# ==========================================================================================================
# Bytecode comparison
# ==========================================================================================================

    def _compare_bytecode(self) -> None:
        """
        Compares two bytecode files according to disasm output.
        """
        with Path(self.patch_disasm).open() as patch:
            with Path(self.expected_disasm).open() as expected:
                prefixes_to_delete = [f"{self.__class__.EVAL_PATCH_FUNCTION_NAME}.", f"{self.base_filename}.base."]

                expected_func_body = self._fetch_bytecode_function(expected, self.expected_func_pattern, prefixes_to_delete)
                patch_func_body = self._fetch_bytecode_function(patch, self.patch_func_pattern, prefixes_to_delete)

                error_report: str = ""
                if expected_func_body is None:
                    error_report = "Expected bytecode function was not found."
                else:
                    assert len(expected_func_body) > 0
                    # Restore fully qualified function name after prefixes removal.
                    method_name = self.__class__.EVAL_PATCH_FUNCTION_NAME
                    expected_func_body[0] = expected_func_body[0].replace(
                        f" {method_name}()",
                        f" {method_name}.{method_name}.{method_name}()",
                    )

                if patch_func_body is None:
                    error_report += "\tEvaluation patch bytecode function was not found"

                if error_report == "" and len(expected_func_body) != len(patch_func_body):
                    error_report = "Expected and patch bytecode differ in count"

                if error_report != "":
                    self.passed = False
                    self.fail_kind = FailKind.COMPARE_FAIL
                    self.report = TestReport("", error_report, 0)
                    return

                patch_func_body[0] = patch_func_body[0].replace(self.eval_method_name, TestEvaluate.EVAL_PATCH_FUNCTION_NAME)
                diff = difflib.ndiff("".join(expected_func_body).splitlines(), "".join(patch_func_body).splitlines())
                error_list = [
                    x for x in diff
                        if x[0] not in self.__class__.NON_DIFF_MARKS
                ]
                if error_list:
                    self.passed = False
                    self.fail_kind = FailKind.COMPARE_FAIL
                    error_report = "Bytecode comparison failed:\n" + "\n".join(error_list)
                    self.report = TestReport("", error_report, 0)

    @staticmethod
    def _fetch_bytecode_function(file: TextIOWrapper, function_name_pattern: str, prefixes: Iterable[str]) -> list[str] | None:
        """
        Find function by 'name' in the disassembled code in the given 'file'.
        Return a list of the found function body lines.
        """
        func_body = []
        lines = file.readlines()
        idx = 0

        while idx < len(lines):
            if re.match(function_name_pattern, lines[idx]):
                while True:
                    func_body.append(TestEvaluate._remove_prefix(lines[idx], prefixes))
                    if lines[idx][0] == "}":
                        return func_body
                    idx += 1
            idx += 1
        return None

    @staticmethod
    def _remove_prefix(line: str, prefixes: Iterable[str]) -> str:
        for p in prefixes:
            line = line.replace(p, "")
        return line

# =====================================================================================================================
# Compiler and disassembler launchers
# =====================================================================================================================

    def _create_evaluate_options(self) -> EvaluateOptions:
        base_file_abs_path = f"{self.base_path}.base.ets"
        context_panda_files = ",".join([self.base_abc])
        return EvaluateOptions(
            self.evaluate_line,
            base_file_abs_path,
            [context_panda_files],
        )

    def _run_compiler(
        self,
        source_ets: str,
        output_abc: str,
        ets_module: bool = False,
        dump_ast: bool = False,
        eval_options: Optional[EvaluateOptions] = None,
        log_level: str = ""
    ) -> None:
        es2panda_flags = ["--extension=ets"]
        if log_level:
            es2panda_flags.append(f"--log-level={log_level}")
        if ets_module:
            es2panda_flags.append("--ets-module")
        if eval_options:
            es2panda_flags.extend([
                "--debugger-eval-mode",
                f"--debugger-eval-panda-files={','.join(eval_options.panda_files_paths)}",
                f"--debugger-eval-source={eval_options.source_path}",
                f"--debugger-eval-line={self.evaluate_line}",
            ])
        else:
            es2panda_flags.append("--debug-info")
        if dump_ast:
            # Added for AST comparison, must be done after all compiler phases.
            es2panda_flags.append("--dump-dynamic-ast")

        prev_path = self.path
        self.path = source_ets
        validator = lambda _1, _2, rc: self._default_validator(rc, output_abc)
        self.passed, self.report, self.fail_kind = self.run_es2panda(es2panda_flags, output_abc, validator)
        self.path = prev_path

    def _run_disasm(
        self,
        source_abc: str,
        output_disasm: str,
    ) -> None:
        ark_disasm_flags = [source_abc, output_disasm]

        params = Params(
            executor=self.test_env.ark_disasm,
            flags=ark_disasm_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.es2panda.timeout,
            fail_kind_fail=FailKind.ARK_DISASM_FAIL,
            fail_kind_timeout=FailKind.ARK_DISASM_TIMEOUT,
            fail_kind_other=FailKind.ARK_DISASM_OTHER,
        )

        self.passed, self.report, self.fail_kind = self.run_one_step(
            name="ark_disasm",
            params=params,
            result_validator=lambda _1, _2, rc: self._default_validator(rc, output_disasm)
        )

    def _default_validator(self, return_code: int, output_path: str) -> bool:
        if self.is_negative_compile:
            return return_code != 0
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0
