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
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from unittest.mock import MagicMock, patch

from runner.reports.pdf_loader import PdfLoader


@dataclass
class FakeDestination:
    """Passes PdfLoader.__is_destination() check: str(type(field)).find('Destination') > 0"""
    title: str


FakeDestination.__qualname__ = "FakeDestination"


def _make_reader(outline: list, creation_date: datetime | None = None) -> MagicMock:
    reader = MagicMock()
    reader.outline = outline
    if creation_date is not None:
        reader.metadata.creation_date = creation_date
    else:
        reader.metadata = None
    return reader


class PdfLoaderTest(unittest.TestCase):

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_flat_outline(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: parse a flat PDF outline with no nesting
        Expected:
        - 3 top-level children created under root
        - titles and sequential prefixes ("1", "2", "3") assigned correctly
        """
        mock_reader_cls.return_value = _make_reader([
            FakeDestination("Introduction"),
            FakeDestination("Lexical Elements"),
            FakeDestination("Types"),
        ])
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        root = loader.get_root_node()
        self.assertEqual(len(root.children), 3)
        self.assertEqual(root.children[0].title, "Introduction")
        self.assertEqual(root.children[0].prefix, "1")
        self.assertEqual(root.children[1].title, "Lexical Elements")
        self.assertEqual(root.children[1].prefix, "2")
        self.assertEqual(root.children[2].title, "Types")
        self.assertEqual(root.children[2].prefix, "3")

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_nested_outline(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: parse a PDF outline with one level of nesting
        Expected:
        - Ch1 gets 2 sub-sections with prefixes "1.1" and "1.2"
        - Ch2 remains a sibling at the top level with prefix "2"
        """
        mock_reader_cls.return_value = _make_reader([
            FakeDestination("Ch1"),
            [FakeDestination("Sec1.1"), FakeDestination("Sec1.2")],
            FakeDestination("Ch2"),
        ])
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        root = loader.get_root_node()
        self.assertEqual(len(root.children), 2)
        ch1 = root.children[0]
        self.assertEqual(ch1.title, "Ch1")
        self.assertEqual(ch1.prefix, "1")
        self.assertEqual(len(ch1.children), 2)
        self.assertEqual(ch1.children[0].prefix, "1.1")
        self.assertEqual(ch1.children[0].title, "Sec1.1")
        self.assertEqual(ch1.children[1].prefix, "1.2")
        self.assertEqual(ch1.children[1].title, "Sec1.2")
        self.assertEqual(root.children[1].title, "Ch2")
        self.assertEqual(root.children[1].prefix, "2")

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_deep_nesting(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: parse a PDF outline with three levels of nesting
        Expected:
        - deepest node gets prefix "1.1.1" and correct title
        """
        mock_reader_cls.return_value = _make_reader([
            FakeDestination("Ch1"),
            [FakeDestination("Sec1.1"), [FakeDestination("Sub1.1.1")]],
        ])
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        root = loader.get_root_node()
        ch1 = root.children[0]
        sec11 = ch1.children[0]
        sub111 = sec11.children[0]
        self.assertEqual(sub111.prefix, "1.1.1")
        self.assertEqual(sub111.title, "Sub1.1.1")

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_config_status(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: apply status from config dict to matching spec nodes
        Expected:
        - node "1" gets status from config, node "1.1" gets empty status (not in config),
          node "1.2" gets its own status from config
        """
        mock_reader_cls.return_value = _make_reader([
            FakeDestination("Ch1"),
            [FakeDestination("Sec1.1"), FakeDestination("Sec1.2")],
        ])
        config = {
            "1": {"status": "No testable assertions"},
            "1.2": {"status": "In progress"},
        }
        loader = PdfLoader(Path("spec.pdf"), config).parse()
        root = loader.get_root_node()
        self.assertEqual(root.children[0].status, "No testable assertions")
        self.assertEqual(root.children[0].children[0].status, "")
        self.assertEqual(root.children[0].children[1].status, "In progress")

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_none_config(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: parse PDF with None config (no status overrides)
        Expected:
        - all nodes get empty status string
        """
        mock_reader_cls.return_value = _make_reader([
            FakeDestination("Ch1"),
        ])
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        self.assertEqual(loader.get_root_node().children[0].status, "")

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_empty_outline(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: parse a PDF with an empty outline (no bookmarks)
        Expected:
        - root node exists with title "SUMMARY" and zero children
        """
        mock_reader_cls.return_value = _make_reader([])
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        root = loader.get_root_node()
        self.assertEqual(root.title, "SUMMARY")
        self.assertEqual(len(root.children), 0)

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_creation_date(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: extract creation date from PDF metadata
        Expected:
        - date formatted as ISO string "2026-01-15"
        """
        mock_reader_cls.return_value = _make_reader([], datetime(2026, 1, 15))
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        self.assertEqual(loader.get_creation_date(), "2026-01-15")

    @patch("builtins.open", MagicMock())
    @patch("runner.reports.pdf_loader.PdfReader")
    def test_creation_date_unknown(self, mock_reader_cls: MagicMock) -> None:
        """
        Feature: handle missing PDF metadata
        Expected:
        - creation date returns "unknown" when metadata is None
        """
        mock_reader_cls.return_value = _make_reader([])
        loader = PdfLoader(Path("spec.pdf"), None).parse()
        self.assertEqual(loader.get_creation_date(), "unknown")
