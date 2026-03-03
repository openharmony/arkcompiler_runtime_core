#!/usr/bin/env python3
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

"""Tests for review_linter.py

Run from the scripts directory:
    cd static_core/docs/review/scripts
    python3 -m unittest test_review_linter -v
"""

import os
import tempfile
import textwrap
import unittest

from review_linter import (
    DiffFile,
    Rule,
    Violation,
    check_ets099,
    check_ets490_added,
    check_ets801,
    check_ets802,
    check_ets803,
    check_ets804,
    check_file_with_rule,
    check_line,
    filter_rules,
    is_camel_case,
    is_pascal_case,
    is_screaming_snake,
    is_test_path,
    matches_glob,
    parse_unified_diff,
    read_file_safe,
    run_diff_scan,
    run_full_scan,
    RULES,
)
import re


# ---------------------------------------------------------------------------
# Violation.__str__
# ---------------------------------------------------------------------------

class TestViolationStr(unittest.TestCase):
    def test_str(self):
        v = Violation("ETS097", "Error", "foo.cpp", 10, "bad stuff")
        self.assertEqual(str(v), "ETS097:Error:foo.cpp:10: bad stuff")


# ---------------------------------------------------------------------------
# Naming convention helpers
# ---------------------------------------------------------------------------

class TestIsCamelCase(unittest.TestCase):
    def test_valid(self):
        for name in ["foo", "fooBar", "fooBarBaz", "x", "myVar2", "_private"]:
            self.assertTrue(is_camel_case(name), name)

    def test_invalid(self):
        for name in ["FooBar", "FOO", "foo_bar", "FOO_BAR", "123abc"]:
            self.assertFalse(is_camel_case(name), name)


class TestIsPascalCase(unittest.TestCase):
    def test_valid(self):
        for name in ["Foo", "FooBar", "MyClass2", "A"]:
            self.assertTrue(is_pascal_case(name), name)

    def test_invalid(self):
        for name in ["fooBar", "foo", "FOO_BAR", "foo_bar"]:
            self.assertFalse(is_pascal_case(name), name)


class TestIsScreamingSnake(unittest.TestCase):
    def test_valid(self):
        for name in ["FOO", "FOO_BAR", "MY_CONST_123", "A1_B2"]:
            self.assertTrue(is_screaming_snake(name), name)

    def test_invalid(self):
        for name in ["fooBar", "foo_bar", "Foo_Bar", "_FOO"]:
            self.assertFalse(is_screaming_snake(name), name)


# ---------------------------------------------------------------------------
# Path helpers
# ---------------------------------------------------------------------------

class TestIsTestPath(unittest.TestCase):
    def test_test_dir(self):
        self.assertTrue(is_test_path("src/test/foo.cpp"))
        self.assertTrue(is_test_path("src/tests/foo.cpp"))

    def test_non_test(self):
        self.assertFalse(is_test_path("src/main/foo.cpp"))
        self.assertFalse(is_test_path("testing/foo.cpp"))

    def test_windows_separator(self):
        self.assertTrue(is_test_path("src\\test\\foo.cpp"))


class TestMatchesGlob(unittest.TestCase):
    def test_cpp(self):
        self.assertTrue(matches_glob("src/foo.cpp", ["*.cpp", "*.h"]))
        self.assertFalse(matches_glob("src/foo.py", ["*.cpp", "*.h"]))

    def test_wildcard(self):
        self.assertTrue(matches_glob("anything.txt", ["*"]))

    def test_ets(self):
        self.assertTrue(matches_glob("module.ets", ["*.ets", "*.sts"]))
        self.assertFalse(matches_glob("module.ts", ["*.ets", "*.sts"]))


class TestReadFileSafe(unittest.TestCase):
    def test_existing_file(self):
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("hello")
            path = f.name
        try:
            self.assertEqual(read_file_safe(path), "hello")
        finally:
            os.unlink(path)

    def test_missing_file(self):
        self.assertIsNone(read_file_safe("/nonexistent/path/xyz.txt"))


# ---------------------------------------------------------------------------
# Diff parser
# ---------------------------------------------------------------------------

class TestParseUnifiedDiff(unittest.TestCase):
    SAMPLE_DIFF = textwrap.dedent("""\
        diff --git a/foo.cpp b/foo.cpp
        --- a/foo.cpp
        +++ b/foo.cpp
        @@ -10,3 +10,4 @@ some context
         unchanged
        -old line
        +new line
        +added line
    """)

    def test_basic_parse(self):
        files = parse_unified_diff(self.SAMPLE_DIFF)
        self.assertEqual(len(files), 1)
        self.assertEqual(files[0].file_path, "foo.cpp")
        self.assertEqual(files[0].added_lines, {11: "new line", 12: "added line"})
        self.assertEqual(files[0].removed_lines, {11: "old line"})

    def test_multiple_files(self):
        diff = textwrap.dedent("""\
            diff --git a/a.cpp b/a.cpp
            --- a/a.cpp
            +++ b/a.cpp
            @@ -1,1 +1,1 @@
            -old
            +new
            diff --git a/b.cpp b/b.cpp
            --- a/b.cpp
            +++ b/b.cpp
            @@ -1,1 +1,2 @@
             keep
            +added
        """)
        files = parse_unified_diff(diff)
        self.assertEqual(len(files), 2)
        self.assertEqual(files[0].file_path, "a.cpp")
        self.assertEqual(files[1].file_path, "b.cpp")

    def test_empty_diff(self):
        self.assertEqual(parse_unified_diff(""), [])

    def test_no_hunk_header(self):
        diff = textwrap.dedent("""\
            diff --git a/x.h b/x.h
            --- a/x.h
            +++ b/x.h
        """)
        files = parse_unified_diff(diff)
        self.assertEqual(len(files), 1)
        self.assertEqual(files[0].added_lines, {})


# ---------------------------------------------------------------------------
# Checker functions
# ---------------------------------------------------------------------------

class TestCheckEts099(unittest.TestCase):
    def test_all_stages_present(self):
        content = 'check { cflow resolve-id typing absint }'
        self.assertEqual(check_ets099("f.verifier.config", content), [])

    def test_missing_stages(self):
        content = textwrap.dedent("""\
            check {
                cflow
                typing
            }
        """)
        results = check_ets099("f.verifier.config", content)
        self.assertEqual(len(results), 1)
        self.assertIn("absint", results[0][1])
        self.assertIn("resolve-id", results[0][1])

    def test_multiline_check_block(self):
        content = textwrap.dedent("""\
            check {
                cflow
                resolve-id
                typing
                absint
            }
        """)
        self.assertEqual(check_ets099("f.verifier.config", content), [])

    def test_no_check_block(self):
        self.assertEqual(check_ets099("f.verifier.config", "nothing here"), [])


class TestCheckEts801(unittest.TestCase):
    def test_valid_export(self):
        self.assertEqual(check_ets801("f.ets", "export function doStuff() {}"), [])

    def test_invalid_export(self):
        results = check_ets801("f.ets", "export function DoStuff() {}")
        self.assertEqual(len(results), 1)
        self.assertIn("DoStuff", results[0][1])

    def test_no_export(self):
        self.assertEqual(check_ets801("f.ets", "function doStuff() {}"), [])


class TestCheckEts802(unittest.TestCase):
    def test_valid_method(self):
        self.assertEqual(check_ets802("f.ets", "public doSomething() {"), [])

    def test_invalid_method(self):
        results = check_ets802("f.ets", "public DoSomething() {")
        self.assertEqual(len(results), 1)
        self.assertIn("DoSomething", results[0][1])

    def test_constructor_skipped(self):
        self.assertEqual(check_ets802("f.ets", "public constructor() {"), [])

    def test_static_native(self):
        self.assertEqual(check_ets802("f.ets", "public static native myMethod("), [])
        results = check_ets802("f.ets", "public static native MyMethod(")
        self.assertEqual(len(results), 1)


class TestCheckEts803(unittest.TestCase):
    def test_valid_class(self):
        self.assertEqual(check_ets803("f.ets", "export class MyClass {"), [])

    def test_invalid_class(self):
        results = check_ets803("f.ets", "export class myClass {")
        self.assertEqual(len(results), 1)
        self.assertIn("myClass", results[0][1])

    def test_interface(self):
        results = check_ets803("f.ets", "export interface myInterface {")
        self.assertEqual(len(results), 1)

    def test_abstract_class(self):
        self.assertEqual(check_ets803("f.ets", "export abstract class MyBase {"), [])

    def test_enum(self):
        results = check_ets803("f.ets", "export enum color {")
        self.assertEqual(len(results), 1)


class TestCheckEts804(unittest.TestCase):
    def test_screaming_snake(self):
        self.assertEqual(check_ets804("f.ets", "const MAX_SIZE = 10"), [])

    def test_camel_case_allowed(self):
        self.assertEqual(check_ets804("f.ets", "const maxSize = 10"), [])

    def test_invalid_name(self):
        results = check_ets804("f.ets", "const Max_size = 10")
        self.assertEqual(len(results), 1)

    def test_export_const(self):
        self.assertEqual(check_ets804("f.ets", "export const MY_VAL = 1"), [])

    def test_not_top_level(self):
        # Indented const should not match (pattern uses ^)
        self.assertEqual(check_ets804("f.ets", "  const BAD_Name = 1"), [])


class TestCheckEts490Added(unittest.TestCase):
    def test_matching_file(self):
        results = check_ets490_added("astNodeMapping.h", "  SomeNode(")
        self.assertEqual(len(results), 1)

    def test_non_matching_file(self):
        self.assertEqual(check_ets490_added("other.h", "  SomeNode("), [])


# ---------------------------------------------------------------------------
# check_line
# ---------------------------------------------------------------------------

class TestCheckLine(unittest.TestCase):
    def test_match(self):
        rule = Rule(
            rule_id="TEST", severity="Error",
            file_globs=["*"], message="test msg",
            pattern=re.compile(r"badpattern"),
        )
        v = check_line("f.cpp", 5, "has badpattern here", rule)
        self.assertIsNotNone(v)
        self.assertEqual(v.rule_id, "TEST")
        self.assertEqual(v.line, 5)

    def test_no_match(self):
        rule = Rule(
            rule_id="TEST", severity="Error",
            file_globs=["*"], message="test msg",
            pattern=re.compile(r"badpattern"),
        )
        self.assertIsNone(check_line("f.cpp", 1, "clean line", rule))

    def test_no_pattern(self):
        rule = Rule(
            rule_id="TEST", severity="Error",
            file_globs=["*"], message="test msg",
        )
        self.assertIsNone(check_line("f.cpp", 1, "anything", rule))


# ---------------------------------------------------------------------------
# check_file_with_rule
# ---------------------------------------------------------------------------

class TestCheckFileWithRule(unittest.TestCase):
    def test_pattern_rule(self):
        rule = Rule(
            rule_id="R1", severity="Warning",
            file_globs=["*"], message="found it",
            pattern=re.compile(r"TODO"),
        )
        vs = check_file_with_rule("f.cpp", "line1\nTODO fix\nline3", rule)
        self.assertEqual(len(vs), 1)
        self.assertEqual(vs[0].line, 2)

    def test_checker_rule(self):
        def my_checker(fp, content):
            return [(1, "problem")]
        rule = Rule(
            rule_id="R2", severity="Error",
            file_globs=["*"], message="msg",
            checker=my_checker,
        )
        vs = check_file_with_rule("f.cpp", "content", rule)
        self.assertEqual(len(vs), 1)

    def test_path_filter_excludes(self):
        rule = Rule(
            rule_id="R3", severity="Error",
            file_globs=["*"], message="msg",
            pattern=re.compile(r"anything"),
            path_filter=lambda p: False,
        )
        vs = check_file_with_rule("f.cpp", "anything", rule)
        self.assertEqual(vs, [])


# ---------------------------------------------------------------------------
# run_diff_scan
# ---------------------------------------------------------------------------

class TestRunDiffScan(unittest.TestCase):
    def test_added_line_violation(self):
        diff = textwrap.dedent("""\
            diff --git a/foo.cpp b/foo.cpp
            --- a/foo.cpp
            +++ b/foo.cpp
            @@ -1,1 +1,2 @@
             ok
            +EAllocator usage
        """)
        vs = run_diff_scan(diff, RULES)
        ids = [v.rule_id for v in vs]
        self.assertIn("ETS402", ids)

    def test_removed_line_assertion(self):
        diff = textwrap.dedent("""\
            diff --git a/foo.cpp b/foo.cpp
            --- a/foo.cpp
            +++ b/foo.cpp
            @@ -1,2 +1,1 @@
             ok
            -ASSERT(true)
        """)
        vs = run_diff_scan(diff, RULES)
        ids = [v.rule_id for v in vs]
        self.assertIn("ETS011", ids)

    def test_clean_diff(self):
        diff = textwrap.dedent("""\
            diff --git a/foo.cpp b/foo.cpp
            --- a/foo.cpp
            +++ b/foo.cpp
            @@ -1,1 +1,2 @@
             ok
            +int x = 1;
        """)
        vs = run_diff_scan(diff, RULES)
        self.assertEqual(vs, [])


# ---------------------------------------------------------------------------
# run_full_scan
# ---------------------------------------------------------------------------

class TestRunFullScan(unittest.TestCase):
    def test_finds_violation(self):
        with tempfile.TemporaryDirectory() as d:
            fpath = os.path.join(d, "bad.cpp")
            with open(fpath, "w") as f:
                f.write("EAllocator x;\n")
            vs = run_full_scan(d, RULES)
            ids = [v.rule_id for v in vs]
            self.assertIn("ETS402", ids)

    def test_skips_diff_only_rules(self):
        with tempfile.TemporaryDirectory() as d:
            fpath = os.path.join(d, "ok.cpp")
            with open(fpath, "w") as f:
                f.write("ASSERT(true);\n")
            vs = run_full_scan(d, RULES)
            ids = [v.rule_id for v in vs]
            # ETS011 is diff_only, should not fire in full scan
            self.assertNotIn("ETS011", ids)

    def test_skips_excluded_dirs(self):
        with tempfile.TemporaryDirectory() as d:
            tp = os.path.join(d, "third_party")
            os.makedirs(tp)
            fpath = os.path.join(tp, "bad.cpp")
            with open(fpath, "w") as f:
                f.write("EAllocator x;\n")
            vs = run_full_scan(d, RULES)
            self.assertEqual(vs, [])

    def test_no_matching_files(self):
        with tempfile.TemporaryDirectory() as d:
            fpath = os.path.join(d, "readme.md")
            with open(fpath, "w") as f:
                f.write("safe content\n")
            vs = run_full_scan(d, RULES)
            self.assertEqual(vs, [])


# ---------------------------------------------------------------------------
# filter_rules
# ---------------------------------------------------------------------------

class TestFilterRules(unittest.TestCase):
    def test_include(self):
        result = filter_rules(list(RULES), "ETS097,ETS098", None)
        ids = {r.rule_id for r in result}
        self.assertEqual(ids, {"ETS097", "ETS098"})

    def test_exclude(self):
        result = filter_rules(list(RULES), None, "ETS097")
        ids = {r.rule_id for r in result}
        self.assertNotIn("ETS097", ids)
        self.assertIn("ETS098", ids)

    def test_no_filter(self):
        result = filter_rules(list(RULES), None, None)
        self.assertEqual(len(result), len(RULES))


# ---------------------------------------------------------------------------
# Pattern-based rule integration tests
# ---------------------------------------------------------------------------

class TestPatternRules(unittest.TestCase):
    """Test that each regex-based rule fires on expected input."""

    def _check(self, rule_id, filename, line, should_match):
        rule = next(r for r in RULES if r.rule_id == rule_id)
        v = check_line(filename, 1, line, rule)
        if should_match:
            self.assertIsNotNone(v, f"{rule_id} should match: {line!r}")
        else:
            self.assertIsNone(v, f"{rule_id} should NOT match: {line!r}")

    def test_ets097_match(self):
        self._check("ETS097", "run.sh", "--verification-mode=disabled", True)

    def test_ets097_no_match(self):
        self._check("ETS097", "run.sh", "--verification-mode=enabled", False)

    def test_ets098_match(self):
        self._check("ETS098", "foo.cpp", 'SetDefaultVerificationMode(false)', True)

    def test_ets098_no_match(self):
        self._check("ETS098", "foo.cpp", 'SetDefaultVerificationMode(true)', False)

    def test_ets402_match(self):
        self._check("ETS402", "a.cpp", "EAllocator alloc;", True)

    def test_ets402_no_match(self):
        self._check("ETS402", "a.cpp", "Allocator alloc;", False)

    def test_ets403_match(self):
        self._check("ETS403", "a.cpp", "CreateScopedAllocator()", True)
        self._check("ETS403", "a.cpp", "NewScopedAllocator(x)", True)

    def test_ets403_no_match(self):
        self._check("ETS403", "a.cpp", "ScopedAllocator x;", False)

    def test_ets807_match(self):
        self._check("ETS807", "f.ets", "if (typeof x === 'string')", True)

    def test_ets807_no_match(self):
        self._check("ETS807", "f.ets", "if (x instanceof String)", False)

    def test_ets814_match(self):
        self._check("ETS814", "f.ets", "overload myFunc {", True)

    def test_ets814_no_match(self):
        self._check("ETS814", "f.ets", "function myFunc() {", False)

    def test_ets805_match(self):
        self._check("ETS805", "f.ets", "for (let x of arr)", True)

    def test_ets805_no_match(self):
        self._check("ETS805", "f.ets", "for (let i = 0; i < n; i++)", False)

    def test_ets806_match(self):
        self._check("ETS806", "f.ets", "  outer: for (let i = 0;", True)

    def test_ets806_no_match(self):
        self._check("ETS806", "f.ets", "for (let i = 0;", False)

    def test_ets812_match(self):
        self._check("ETS812", "f.ets", 'let s = "hello" + x', True)

    def test_ets812_no_match(self):
        self._check("ETS812", "f.ets", "let x = a + b", False)

    def test_ets813_match(self):
        self._check("ETS813", "f.ets", "let x = 1e10", True)

    def test_ets813_no_match(self):
        self._check("ETS813", "f.ets", "let x = 10000000000", False)

    def test_ets011_removed_line(self):
        # ETS011 is diff_removed_only; check that the pattern matches
        rule = next(r for r in RULES if r.rule_id == "ETS011")
        self.assertTrue(rule.pattern.search("ASSERT(x > 0)"))
        self.assertTrue(rule.pattern.search("ES2PANDA_ASSERT(y)"))
        self.assertTrue(rule.pattern.search("arktest.assert"))


if __name__ == "__main__":
    unittest.main()
