#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import os
import unittest
from pathlib import Path
from typing import cast

from runner.common_exceptions import InvalidConfiguration
from runner.suites.test_metadata import TestMetadata
from runner.test import test_utils
from runner.utils import ExpectedField


class MetadataTest(unittest.TestCase):
    current_folder = os.path.dirname(__file__)

    def test_empty_metadata(self) -> None:
        metadata = TestMetadata.create_filled_metadata({}, Path(__file__))
        self.assertIsNotNone(metadata)
        self.assertEqual(str(metadata.tags), '[]')
        self.assertIsNone(metadata.desc)
        self.assertIsNone(metadata.files)
        self.assertEqual(str(metadata.expected_out), '{}')
        self.assertEqual(str(metadata.expected_error), '{}')

    def test_metadata_main_items(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'desc': "abc",
            'tags': [],
            'files': [],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)
        self.assertEqual(metadata.desc, "abc")
        self.assertIsNotNone(metadata.files)
        self.assertListEqual(cast(list[str], metadata.files), [])

    def test_metadata_tags_compile_only(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compile-only'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertTrue(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)

    def test_metadata_tags_negative(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['negative'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertTrue(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)

    def test_metadata_tags_not_a_test(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['not-a-test'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertTrue(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)

    def test_metadata_tags_no_warmup(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertTrue(metadata.tags.no_warmup)

    def test_metadata_tags_all(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compile-only', 'negative', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertTrue(metadata.tags.compile_only)
        self.assertTrue(metadata.tags.negative)
        self.assertTrue(metadata.tags.not_a_test)
        self.assertTrue(metadata.tags.no_warmup)

    def test_metadata_files(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'files': ['test1.sts', 'test2.sts'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(cast(list[str], metadata.files)), 2)
        self.assertListEqual(cast(list[str], metadata.files), ['test1.sts', 'test2.sts'])

    def test_non_existing_path(self) -> None:
        with self.assertRaises(FileNotFoundError):
            metadata = TestMetadata.create_filled_metadata({}, Path("path"))
            self.assertIsNone(metadata)

    def test_one_wrong_tag(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compily-only', 'negative', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(metadata.tags.invalid_tags), 1)
        self.assertEqual(metadata.tags.invalid_tags, ['compily-only'])

    def test_wrong_tags(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compily-only', 'negatives', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(metadata.tags.invalid_tags), 2)
        self.assertEqual(metadata.tags.invalid_tags, ['compily-only', 'negatives'])

    def test_all_valid_tags(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compile-only', 'negative', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(metadata.tags.invalid_tags), 0)

    @test_utils.parametrized_test_cases([
        ("ark-timeout=60", ["ark-timeout=60"]),
        (["ark-timeout=60"], ["ark-timeout=60"]),
    ])
    def test_ark_options_validator(self, test_param: str | list[str], expected_res: list[str]) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'ark_options': test_param
        }, Path(__file__))

        self.assertEqual(metadata.ark_options, expected_res)

    @test_utils.parametrized_test_cases([
        ("ark_options", {},),
        ("test_cli", "abc",),
        ("files", "abc",),
        ("es2panda_options", "abc",),
    ])
    def test_invalid_vals(self, field_name: str, field_val: str | dict) -> None:
        with self.assertRaises(InvalidConfiguration):
            _ = TestMetadata.create_filled_metadata({
                field_name: field_val
            }, Path(__file__))

    @test_utils.parametrized_test_cases([
        ("test", {'compiler': ['test']}),
        (["test1"], {'compiler': ['test1']}),
        (["test1", "test2"], {'compiler': ['test1', 'test2']}),
        ({"runtime": ["test1", "test2"]}, {'runtime': ['test1', 'test2']}),
    ])
    def test_expected_output_validator(self, test_param: str | list[str] | dict, expected_res: ExpectedField) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': test_param
        }, Path(__file__))

        self.assertEqual(metadata.expected_out, expected_res)

    @test_utils.parametrized_test_cases([
        ("ark", ["ark"]),
        (["ark1", "ark2"], ["ark1", "ark2"]),
    ])
    def test_negative_steps(self, neg_steps: str | list[str], expected_neg_steps: list[str]) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['negative'],
            'negative_steps': neg_steps
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.negative_steps, expected_neg_steps)

    @test_utils.parametrized_test_cases([
        (["negative"], {"step": "ark"}),
        (["negative"], ["ark", "ark"]),
        (["compile-only"], ["ark"])
    ])
    def test_negative_steps_exceptions(self, tag: list[str], negative_steps: str | list) -> None:
        with self.assertRaises(InvalidConfiguration):
            _ = TestMetadata.create_filled_metadata({
                'tags': tag,
                'negative_steps': negative_steps
            }, Path(__file__))
