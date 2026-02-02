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

from collections.abc import Callable
from threading import Lock

from runner.extensions.flows.itest_flow import ITestFlow

PredicateFunction = Callable[[ITestFlow], bool]


def is_valid_test(test: ITestFlow) -> bool:
    return test.is_valid_test


def is_runtime_test(test: ITestFlow) -> bool:
    return not test.is_negative_compile and test.is_valid_test


class TestFlowRegistry:
    """Thread-safe registry that stores and provides access to test flow objects.

    Tests are indexed by their unique ``test_id`` and can be queried in bulk
    using predicate functions or via the built-in convenience accessors.
    """

    def __init__(self) -> None:
        """Initialize an empty registry with a reentrant lock for thread safety."""
        self.data: dict[str, ITestFlow] = {}
        self._registry_lock: Lock = Lock()

    @property
    def valid_tests(self) -> list[ITestFlow]:
        """Return all tests that are considered valid.

        A test is valid when its ``is_valid_test`` property is ``True``.

        :return: A list of valid :class:`ITestFlow` instances.
        """
        return self.filter(is_valid_test)

    @property
    def runtime_tests(self) -> list[ITestFlow]:
        """Return all tests that are eligible for runtime execution.

        A test qualifies as a runtime test when it is valid (``is_valid_test``
        is ``True``) and is not a negative-compile test
        (``is_negative_compile`` is ``False``).

        :return: A list of runtime-eligible :class:`ITestFlow` instances.
        """
        return self.filter(is_runtime_test)

    def register(self, test: ITestFlow) -> ITestFlow:
        """Add a test to the registry in a thread-safe manner.

        If a test with the same ``test_id`` already exists, the existing entry
        is kept and returned instead of overwriting it.

        :param test: The test flow instance to register.
        :return: The registered test flow — either the newly added one or the
            pre-existing entry that shares the same ``test_id``.
        """
        with self._registry_lock:
            if test.test_id not in self.data:
                self.data[test.test_id] = test
            return self.data[test.test_id]

    def reset(self) -> None:
        """Clear up the registry."""
        with self._registry_lock:
            self.data.clear()

    def find(self, test_id: str) -> ITestFlow | None:
        """Retrieve a test by its unique identifier.

        :param test_id: The unique identifier of the test to look up.
        :return: The matching :class:`ITestFlow` instance, or ``None`` if no
            test with the given ``test_id`` is registered.
        """
        with self._registry_lock:
            return self.data.get(test_id, None)

    def filter(self, predicate: PredicateFunction) -> list[ITestFlow]:
        """Return all tests that satisfy the given predicate.

        :param predicate: A callable that accepts an :class:`ITestFlow` and
            returns ``True`` for tests that should be included in the result.
        :return: A list of :class:`ITestFlow` instances for which *predicate*
            returned ``True``.
        """
        with self._registry_lock:
            return [test for test_id, test in self.data.items() if predicate(test)]
