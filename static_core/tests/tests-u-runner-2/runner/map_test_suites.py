#!/usr/bin/env python3
# -- coding: utf-8 --
#
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
#

"""
Utilities for validating the compatibility of a workflow and a test suite.

Provides functions to check whether a given workflow is allowed to be used
with a given test suite, based on the mapping defined in test suite config files.
"""

from typing import Any, cast

from runner import utils
from runner.common_exceptions import InvalidConfiguration
from runner.logger import Log
from runner.options import cli_options_utils as cli_utils
from runner.options.cli_options import ConfigsLoader

_LOGGER = Log.get_logger(__file__)

CliArgsDict = dict[str, Any]  # type: ignore[explicit-any]


def get_test_suites_for_workflow(workflow: str) -> list[str]:
    """Return all test suites that list the given workflow as acceptable.

    Args:
        workflow: Name of the workflow to look up.

    Returns:
        A list of test suite names that support the specified workflow.
        Returns an empty list if no test suite accepts the workflow.
    """
    test_suite_list = cli_utils.get_config_list(utils.get_config_test_suite_folder())
    ts_for_wf = []
    for ts in test_suite_list:
        ts_path = utils.get_config_test_suite_folder() / f"{ts}.yaml"
        ts_config = utils.load_config(ts_path)
        acceptable_workflows: list[str] = cast(list[str], ts_config.get("workflows", []))
        if workflow in acceptable_workflows:
            ts_for_wf.append(ts)
    return ts_for_wf


def check_mapping_ts_vs_wf(args: CliArgsDict) -> bool:
    """Entry point for workflow vs test suite validation called from the runner startup.

    Reads the 'test-suite' and 'workflow' values from the parsed argument dictionary
    (produced by get_args / apply_cli_environment_overrides) and delegates to
    check_test_suite() for the actual compatibility check.
    Intended to be called once before the runner initialises its configuration,
    so that an invalid combination is caught early and the process exits with a
    non-zero code before any test execution begins.

    Args:
        args: Flat dictionary of resolved CLI/config arguments.
              Must contain the string keys 'test-suite' and 'workflow'.

    Returns:
        True if the workflow is listed as acceptable for the given test suite,
        False otherwise (an error message is logged in that case).
    """
    test_suite: str = get_param("test-suite", args)
    workflow: str = get_param("workflow", args)
    loader = ConfigsLoader(workflow, test_suite)
    test_suite_config = loader.load_test_suite_config()
    workflow_config = loader.load_workflow_config()
    independent_workflow = workflow_config.get("independent", True)
    acceptable_workflows = test_suite_config.get("workflows", [])

    if workflow in acceptable_workflows:
        if not independent_workflow:
            desc = workflow_config.get("description", "")
            _LOGGER.error(f"Attention: you are using workflow '{workflow}' that "
                            f"might require additional actions before. See its description:\n{desc}")
        return True

    error = f"The '{workflow}' workflow is not supported for the '{test_suite}' test suite.\n"
    if acceptable_workflows:
        error += f"For '{test_suite}' test suite use one of following workflows: {acceptable_workflows}\n"
    else:
        error += (f"For '{test_suite}' test suite there is no acceptable workflows.\n"
                  "The suite is either deprecated or not supported.\n"
                  "If this is a new test suite please fill up `workflows` item in the test suite config.\n"
                  "For example how to fill up `workflows`, see test suite config 'ets-cts.yaml' in cfg folder.\n")
    available_test_suites = get_test_suites_for_workflow(workflow)
    if available_test_suites:
        error += f"For '{workflow}' workflow use one of following test suites : {available_test_suites}"
    else:
        error += (f"For '{workflow}' workflow there is no acceptable test suites.\n"
                  "The workflow is either deprecated or not supported.\n"
                  "If this is a new workflow please mention it in `workflows` item "
                  "in the suitable test suite configs.\n"
                  "For example how to fill up `workflows`, see test suite config 'ets-cts.yaml' in cfg folder.\n")

    _LOGGER.error(error)
    return False


def get_param(param: str, args: CliArgsDict) -> str:
    """Retrieve a required string parameter from the parsed argument dictionary.

    Looks up *param* in *args* and returns its value as a string.
    Raises InvalidConfiguration when the key is absent or its value is None,
    so callers are guaranteed to receive a non-None string.

    Args:
        param: The name of the parameter to retrieve (e.g. 'test-suite', 'workflow').
        args: Flat dictionary of resolved CLI arguments.

    Returns:
        The string value associated with *param*.

    Raises:
        InvalidConfiguration: If *param* is not present in *args* or its value is None.
    """
    value = args.get(param)
    if value is None:
        raise InvalidConfiguration(f"Parameter '{param}' is not specified in the command line")
    return cast(str, value)
