#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import unittest

from runner.reports.spec_node import SpecNode


class SpecNodeTest(unittest.TestCase):

    def test_root_node(self) -> None:
        """
        Feature: create a root node with no parent
        Expected:
        - title, prefix, status set correctly; parent is None
        - children list is empty; all counters initialized to zero
        """
        node = SpecNode("SUMMARY", "", "", None)
        self.assertEqual(node.title, "SUMMARY")
        self.assertEqual(node.prefix, "")
        self.assertEqual(node.status, "")
        self.assertIsNone(node.parent)
        self.assertEqual(node.children, [])
        self.assertEqual(node.dir, "")
        for attr in ["total_acc", "passed_acc", "failed_acc", "ignored_acc",
                      "ignored_but_passed_acc", "excluded_acc", "excluded_after_acc",
                      "total", "passed", "failed", "ignored",
                      "ignored_but_passed", "excluded", "excluded_after"]:
            with self.subTest(attr=attr):
                self.assertEqual(getattr(node, attr), 0)

    def test_child_auto_appended(self) -> None:
        """
        Feature: automatic parent-child linkage on node creation
        Expected:
        - child's parent reference points to the parent node
        - parent's children list contains the child
        """
        parent = SpecNode("Root", "", "", None)
        child = SpecNode("Chapter 1", "1", "", parent)
        self.assertIs(child.parent, parent)
        self.assertEqual(len(parent.children), 1)
        self.assertIs(parent.children[0], child)

    def test_multiple_children_order(self) -> None:
        """
        Feature: children preserve insertion order
        Expected:
        - two children appear in the order they were created
        """
        parent = SpecNode("Root", "", "", None)
        child1 = SpecNode("Ch1", "1", "", parent)
        child2 = SpecNode("Ch2", "2", "", parent)
        self.assertEqual(len(parent.children), 2)
        self.assertIs(parent.children[0], child1)
        self.assertIs(parent.children[1], child2)

    def test_to_dict_minimal(self) -> None:
        """
        Feature: to_dict with only title set (empty prefix, status, dir, zero counters)
        Expected:
        - dict contains only the "title" key
        """
        node = SpecNode("SUMMARY", "", "", None)
        result = node.to_dict()
        self.assertEqual(result, {"title": "SUMMARY"})

    def test_to_dict_with_prefix_and_status(self) -> None:
        """
        Feature: to_dict includes prefix and status when they are non-empty
        Expected:
        - dict contains "prefix" and "status" keys with correct values
        """
        node = SpecNode("Generics", "5", "No testable assertions", None)
        result = node.to_dict()
        self.assertEqual(result.get("prefix"), "5")
        self.assertEqual(result.get("status"), "No testable assertions")

    def test_to_dict_with_dir(self) -> None:
        """
        Feature: to_dict includes test_dir when dir field is set
        Expected:
        - dict contains "test_dir" key with the directory path
        """
        node = SpecNode("Generics", "5", "", None)
        node.dir = "05.generics"
        result = node.to_dict()
        self.assertEqual(result.get("test_dir"), "05.generics")

    def test_to_dict_with_acc_counters(self) -> None:
        """
        Feature: to_dict includes accumulated counters when total_acc > 0
        Expected:
        - dict contains all *_acc counter keys with their values
        """
        node = SpecNode("Root", "", "", None)
        node.total_acc = 10
        node.passed_acc = 7
        node.failed_acc = 1
        node.ignored_acc = 1
        node.ignored_but_passed_acc = 1
        node.excluded_acc = 0
        node.excluded_after_acc = 0
        result = node.to_dict()
        self.assertEqual(result.get("total_acc"), 10)
        self.assertEqual(result.get("passed_acc"), 7)
        self.assertEqual(result.get("failed_acc"), 1)
        self.assertEqual(result.get("ignored_acc"), 1)
        self.assertEqual(result.get("ignored_but_passed_acc"), 1)
        self.assertEqual(result.get("excluded_acc"), 0)
        self.assertEqual(result.get("excluded_after_acc"), 0)

    def test_to_dict_with_non_acc_counters(self) -> None:
        """
        Feature: to_dict includes own (non-accumulated) counters when total > 0
        Expected:
        - dict contains total, passed, failed keys with their values
        """
        node = SpecNode("Leaf", "5.3", "", None)
        node.total = 5
        node.passed = 4
        node.failed = 1
        node.ignored = 0
        node.ignored_but_passed = 0
        node.excluded = 0
        node.excluded_after = 0
        result = node.to_dict()
        self.assertEqual(result.get("total"), 5)
        self.assertEqual(result.get("passed"), 4)
        self.assertEqual(result.get("failed"), 1)

    def test_to_dict_zero_counters_omitted(self) -> None:
        """
        Feature: to_dict omits counter sections when all counters are zero
        Expected:
        - dict does not contain total, total_acc, passed, passed_acc keys
        """
        node = SpecNode("Empty", "1", "", None)
        result = node.to_dict()
        self.assertNotIn("total", result)
        self.assertNotIn("total_acc", result)
        self.assertNotIn("passed", result)
        self.assertNotIn("passed_acc", result)

    def test_to_dict_with_children(self) -> None:
        """
        Feature: to_dict includes subsections when node has children
        Expected:
        - dict contains "subsections" list with the child node reference
        """
        parent = SpecNode("Root", "", "", None)
        child = SpecNode("Ch1", "1", "", parent)
        result = parent.to_dict()
        subsections = result.get("subsections")
        self.assertIsNotNone(subsections)
        assert subsections is not None
        self.assertEqual(len(subsections), 1)
        self.assertIs(subsections[0], child)

    def test_to_dict_no_children(self) -> None:
        """
        Feature: to_dict omits subsections when node has no children
        Expected:
        - dict does not contain "subsections" key
        """
        node = SpecNode("Leaf", "1.1", "", None)
        result = node.to_dict()
        self.assertNotIn("subsections", result)

    def test_deep_hierarchy(self) -> None:
        """
        Feature: three-level parent-child hierarchy (root > chapter > section)
        Expected:
        - parent references chain correctly through all levels
        - each level has exactly one child
        """
        root = SpecNode("Root", "", "", None)
        ch1 = SpecNode("Ch1", "1", "", root)
        sec1 = SpecNode("Sec1.1", "1.1", "", ch1)
        self.assertIs(sec1.parent, ch1)
        self.assertIs(ch1.parent, root)
        self.assertEqual(len(root.children), 1)
        self.assertEqual(len(ch1.children), 1)
        self.assertEqual(len(sec1.children), 0)
