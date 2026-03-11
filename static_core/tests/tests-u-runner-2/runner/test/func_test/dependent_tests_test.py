#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

from pathlib import Path
from unittest import TestCase
from unittest.mock import MagicMock

from runner.options.config import Config
from runner.options.options_test_lists import TestListsOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.options_workflow import WorkflowOptions
from runner.suites.test_standard_flow import TestStandardFlow
from runner.suites.tests_flow_registry import TestFlowRegistry
from runner.suites.work_dir import WorkDir
from runner.types.test_env import TestEnv


def get_test_instance(test_id: str) -> TestStandardFlow:
    test_env = MagicMock(spec=TestEnv)
    test_env.config = MagicMock(spec=Config)
    test_env.config.test_suite = MagicMock(spec=TestSuiteOptions)
    test_env.config.test_suite.use_metadata = True
    test_env.config.test_suite.test_lists = MagicMock(spec=TestListsOptions)
    test_env.config.test_suite.test_lists.update_expected = False
    test_env.config.test_suite.validator_class = None
    test_env.work_dir = MagicMock(spec=WorkDir)
    test_env.work_dir.intermediate = Path("/tmp")
    test_env.config.workflow = MagicMock(spec=WorkflowOptions)
    test_env.config.workflow.is_panda = True
    test_env.test_flow_registry = TestFlowRegistry()
    params = MagicMock()

    data = Path(__file__).parent / "dependent_data"
    result: TestStandardFlow = TestStandardFlow(
        test_path=data / test_id,
        test_id=test_id,
        test_env=test_env,
        params=params,
        is_dependent=False,
        parent_test_id=""
    )
    return result


class DependentTestsTest(TestCase):
    """
    Test cases for the dependent_tests, search_dependent_tests and dependent_abc_files
    properties of TestStandardFlow.
    """

    def test_empty_dependent_tests(self) -> None:
        """
        Test: no dependent tests exist.
        Checks: all *dependent* properties are empty
        """
        main_instance = get_test_instance("test_no_dep.ets")

        self.assertListEqual([], main_instance.dependent_abc_files)
        self.assertListEqual([], main_instance.search_dependent_tests())
        self.assertListEqual([], list(main_instance.dependent_tests))

    def test_single_dependent_test_no_nested_dependencies(self) -> None:
        """
        Test: one dependent test and no nested dependencies.
        Checks: all *dependent* properties contains by one dependent test
        """
        main_instance = get_test_instance("test_single_dep.ets")

        self.assertEqual(len(list(main_instance.dependent_tests)), 1)
        dep_instance = main_instance.dependent_tests[0]
        self.assertEqual("test_dep1.ets", dep_instance.test_id)

        self.assertEqual(len(list(main_instance.search_dependent_tests())), 1)
        dep_instance = main_instance.search_dependent_tests()[0]
        self.assertEqual("test_dep1.ets", dep_instance.test_id)

        self.assertEqual(len(list(main_instance.dependent_abc_files)), 1)
        dep_abc = main_instance.dependent_abc_files[0]
        self.assertEqual("/tmp/test_dep1.ets.abc", dep_abc)

    def test_multiple_dependent_tests_no_nested_dependencies(self) -> None:
        """
        Test: several dependent tests and no nested dependencies.
        Structure: test_mult_dep1 -> test_dep2, test_dep1
        Checks: all *dependent* properties contains by 2 dependent tests
        """
        main_instance = get_test_instance("test_mult_dep1.ets")

        self.assertEqual(len(list(main_instance.dependent_tests)), 2)
        dep_instances = [test.test_id for test in main_instance.dependent_tests]
        self.assertListEqual(["test_dep1.ets", "test_dep2.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.search_dependent_tests())), 2)
        dep_instances = [test.test_id for test in main_instance.search_dependent_tests()]
        self.assertListEqual(["test_dep1.ets", "test_dep2.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_abc_files)), 2)
        self.assertEqual(["/tmp/test_dep1.ets.abc", "/tmp/test_dep2.ets.abc"],
                         main_instance.dependent_abc_files)

    def test_dependent_test_with_nested_dependencies(self) -> None:
        """
        Test: dependent test with nested dependencies.
        Structure: test_mult_dep3 -> test_dep3 -> test_dep2
        Checks: search_dependent_tests and  dependent_tests contains by 1 direct dependent test
        dependent_abc_files returns 2 abc files
        """
        main_instance = get_test_instance("test_mult_dep3.ets")

        self.assertEqual(len(list(main_instance.search_dependent_tests())), 1)
        dep_instances = [test.test_id for test in main_instance.search_dependent_tests()]
        self.assertListEqual(["test_dep3.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_tests)), 1)
        dep_instances = [test.test_id for test in main_instance.dependent_tests]
        self.assertListEqual(["test_dep3.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_abc_files)), 2)
        self.assertEqual(["/tmp/test_dep2.ets.abc", "/tmp/test_dep3.ets.abc"],
                         main_instance.dependent_abc_files)

    def test_duplicate_removal1(self) -> None:
        """
        Test: several dependent tests with nested dependencies.
        Structure:
            test_mult_dep3 -> test_dep2, test_dep3
            test_dep3 -> test_dep2
        Checks: search_dependent_tests and  dependent_tests contains by 2 direct dependent tests
        dependent_abc_files returns 2 abc files
        """
        main_instance = get_test_instance("test_mult_dep2.ets")

        self.assertEqual(len(list(main_instance.search_dependent_tests())), 2)
        dep_instances = [test.test_id for test in main_instance.search_dependent_tests()]
        self.assertListEqual(["test_dep2.ets", "test_dep3.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_tests)), 2)
        dep_instances = [test.test_id for test in main_instance.dependent_tests]
        self.assertListEqual(["test_dep2.ets", "test_dep3.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_abc_files)), 2)
        self.assertEqual(["/tmp/test_dep2.ets.abc", "/tmp/test_dep3.ets.abc"],
                         main_instance.dependent_abc_files)

    def test_duplicate_removal2(self) -> None:
        """
        Test: several dependent tests with nested dependencies.
        Structure:
            test_mult_dep4 -> test_dep3, test_dep4
            test_dep3 -> test_dep2
            test_dep4 -> test_dep2
        Checks: search_dependent_tests and  dependent_tests contains by 2 direct dependent tests
        dependent_abc_files returns 3 abc files
        """
        main_instance = get_test_instance("test_mult_dep4.ets")

        self.assertEqual(len(list(main_instance.search_dependent_tests())), 2)
        dep_instances = [test.test_id for test in main_instance.search_dependent_tests()]
        self.assertListEqual(["test_dep3.ets", "test_dep4.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_tests)), 2)
        dep_instances = [test.test_id for test in main_instance.dependent_tests]
        self.assertListEqual(["test_dep3.ets", "test_dep4.ets"], dep_instances)

        self.assertEqual(len(list(main_instance.dependent_abc_files)), 3)
        self.assertEqual(["/tmp/test_dep2.ets.abc", "/tmp/test_dep3.ets.abc", "/tmp/test_dep4.ets.abc"],
                         main_instance.dependent_abc_files)
