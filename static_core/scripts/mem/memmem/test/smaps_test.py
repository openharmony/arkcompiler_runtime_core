# -- coding: utf-8 --
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

import pathlib
import re
import unittest

from src.smaps import MemProfile, parse_smaps_text


_FIXTURES = pathlib.Path(__file__).parent / "fixtures"


class SmapsTest(unittest.TestCase):
    def test_smaps_python(self) -> None:
        summary = parse_smaps_text(
            (_FIXTURES / "smaps_python.smaps").read_text())
        assert summary is not None

        self.assertEqual(summary.total, MemProfile(
            14896, 10048, 5178, 10048, 5792, 4256, 0, 0, 4256))
        self.assertEqual(
            summary.breakdown,
            {
                "/usr/bin/python3.10": MemProfile(5776, 3528, 1112, 3528, 3248, 280, 0, 0, 280),
                "/usr/lib/locale/C.utf8/LC_CTYPE": MemProfile(348, 128, 14, 128, 128, 0, 0, 0, 0),
                "/usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache": MemProfile(28, 28, 1, 28, 28, 0, 0, 0, 0),
                "/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2": MemProfile(236, 236, 19, 236, 220, 16, 0, 0, 16),
                "/usr/lib/x86_64-linux-gnu/libc.so.6": MemProfile(2160, 1560, 55, 1560, 1536, 24, 0, 0, 24),
                "/usr/lib/x86_64-linux-gnu/libexpat.so.1.8.7": MemProfile(196, 92, 24, 92, 80, 12, 0, 0, 12),
                "/usr/lib/x86_64-linux-gnu/libm.so.6": MemProfile(924, 484, 28, 484, 476, 8, 0, 0, 8),
                "/usr/lib/x86_64-linux-gnu/libz.so.1.2.11": MemProfile(112, 80, 17, 80, 72, 8, 0, 0, 8),
                "[anonymous]": MemProfile(3960, 2904, 2904, 2904, 0, 2904, 0, 0, 2904),
                "[heap]": MemProfile(992, 928, 928, 928, 0, 928, 0, 0, 928),
                "[stack]": MemProfile(132, 76, 76, 76, 0, 76, 0, 0, 76),
                "[vdso]": MemProfile(8, 4, 0, 4, 4, 0, 0, 0, 0),
                "[vvar]": MemProfile(16, 0, 0, 0, 0, 0, 0, 0, 0),
                "[vvar_vclock]": MemProfile(8, 0, 0, 0, 0, 0, 0, 0, 0),
            },
        )

    def test_smaps_self(self) -> None:
        summary = parse_smaps_text(
            (_FIXTURES / "smaps_self.smaps").read_text())
        assert summary is not None

        self.assertEqual(summary.total, MemProfile(
            4312, 2420, 368, 2420, 2108, 312, 0, 0, 156))
        self.assertEqual(
            summary.breakdown,
            {
                "/usr/bin/cp": MemProfile(144, 144, 144, 144, 0, 144, 0, 0, 8),
                "/usr/lib/locale/C.utf8/LC_ADDRESS": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_COLLATE": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_CTYPE": MemProfile(348, 128, 14, 128, 128, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_IDENTIFICATION": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_MEASUREMENT": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_MESSAGES/SYS_LC_MESSAGES": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_MONETARY": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_NAME": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_NUMERIC": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_PAPER": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_TELEPHONE": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_TIME": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache": MemProfile(28, 28, 1, 28, 28, 0, 0, 0, 0),
                "/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2": MemProfile(236, 236, 19, 236, 220, 16, 0, 0, 16),
                "/usr/lib/x86_64-linux-gnu/libacl.so.1.1.2301": MemProfile(40, 32, 9, 32, 24, 8, 0, 0, 8),
                "/usr/lib/x86_64-linux-gnu/libattr.so.1.1.2501": MemProfile(32, 28, 28, 28, 0, 28, 0, 0, 8),
                "/usr/lib/x86_64-linux-gnu/libc.so.6": MemProfile(2160, 1496, 53, 1496, 1472, 24, 0, 0, 24),
                "/usr/lib/x86_64-linux-gnu/libpcre2-8.so.0.10.4": MemProfile(604, 80, 11, 80, 72, 8, 0, 0, 8),
                "/usr/lib/x86_64-linux-gnu/libselinux.so.1": MemProfile(168, 124, 13, 124, 116, 8, 0, 0, 8),
                "[anonymous]": MemProfile(212, 44, 44, 44, 0, 44, 0, 0, 44),
                "[heap]": MemProfile(132, 20, 20, 20, 0, 20, 0, 0, 20),
                "[stack]": MemProfile(132, 12, 12, 12, 0, 12, 0, 0, 12),
                "[vdso]": MemProfile(8, 4, 0, 4, 4, 0, 0, 0, 0),
                "[vvar]": MemProfile(16, 0, 0, 0, 0, 0, 0, 0, 0),
                "[vvar_vclock]": MemProfile(8, 0, 0, 0, 0, 0, 0, 0, 0),
            },
        )

    def test_smaps_shell(self) -> None:
        summary = parse_smaps_text(
            (_FIXTURES / "smaps_shell.smaps").read_text())
        assert summary is not None

        self.assertEqual(summary.total, MemProfile(
            4792, 3352, 483, 3352, 3080, 272, 0, 0, 272))
        self.assertEqual(
            summary.breakdown,
            {
                "/usr/bin/bash": MemProfile(1364, 1116, 204, 1116, 1064, 52, 0, 0, 52),
                "/usr/lib/locale/C.utf8/LC_ADDRESS": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_COLLATE": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_CTYPE": MemProfile(348, 128, 10, 128, 128, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_IDENTIFICATION": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_MEASUREMENT": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_MESSAGES/SYS_LC_MESSAGES": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_MONETARY": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_NAME": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_NUMERIC": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_PAPER": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_TELEPHONE": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/locale/C.utf8/LC_TIME": MemProfile(4, 4, 0, 4, 4, 0, 0, 0, 0),
                "/usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache": MemProfile(28, 28, 1, 28, 28, 0, 0, 0, 0),
                "/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2": MemProfile(236, 236, 19, 236, 220, 16, 0, 0, 16),
                "/usr/lib/x86_64-linux-gnu/libc.so.6": MemProfile(2160, 1496, 53, 1496, 1472, 24, 0, 0, 24),
                "/usr/lib/x86_64-linux-gnu/libtinfo.so.6.3": MemProfile(200, 140, 36, 140, 120, 20, 0, 0, 20),
                "[anonymous]": MemProfile(116, 52, 52, 52, 0, 52, 0, 0, 52),
                "[heap]": MemProfile(132, 92, 92, 92, 0, 92, 0, 0, 92),
                "[stack]": MemProfile(132, 16, 16, 16, 0, 16, 0, 0, 16),
                "[vdso]": MemProfile(8, 4, 0, 4, 4, 0, 0, 0, 0),
                "[vvar]": MemProfile(16, 0, 0, 0, 0, 0, 0, 0, 0),
                "[vvar_vclock]": MemProfile(8, 0, 0, 0, 0, 0, 0, 0, 0),
            },
        )

    def test_empty_input(self) -> None:
        summary = parse_smaps_text("")

        self.assertIsNone(summary)

    def test_mapping_without_metrics_is_not_none(self) -> None:
        text = "55f000000000-55f000001000 r--p 00000000 00:00 0 /bin/x\n"

        summary = parse_smaps_text(text)
        assert summary is not None

        self.assertEqual(
            summary.breakdown,
            {"/bin/x": MemProfile(0, 0, 0, 0, 0, 0, 0, 0, 0)},
        )
        self.assertEqual(summary.total,
                         MemProfile(0, 0, 0, 0, 0, 0, 0, 0, 0))

    def test_tag_filter_keeps_matching_mappings_only(self) -> None:
        text = (_FIXTURES / "smaps_shell.smaps").read_text()

        summary = parse_smaps_text(text, re.compile(r".*\.so"))
        assert summary is not None

        self.assertEqual(
            summary.breakdown,
            {
                "/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2": MemProfile(
                    236, 236, 19, 236, 220, 16, 0, 0, 16),
                "/usr/lib/x86_64-linux-gnu/libc.so.6": MemProfile(
                    2160, 1496, 53, 1496, 1472, 24, 0, 0, 24),
                "/usr/lib/x86_64-linux-gnu/libtinfo.so.6.3": MemProfile(
                    200, 140, 36, 140, 120, 20, 0, 0, 20),
            },
        )
        self.assertEqual(summary.total, MemProfile(
            2596, 1872, 108, 1872, 1812, 60, 0, 0, 60))

    def test_tag_filter_is_start_anchored(self) -> None:
        text = (_FIXTURES / "smaps_shell.smaps").read_text()

        no_match = parse_smaps_text(text, re.compile(r"\.so$"))
        self.assertIsNone(no_match)
        matched = parse_smaps_text(text, re.compile(r".*\.so"))
        assert matched is not None
        self.assertNotEqual(matched.breakdown, {})

    def test_tag_filter_matches_normalized_tag(self) -> None:
        text = (
            "55f000000000-55f000001000 rw-p 00000000 00:00 0 \n"
            "Size:                4 kB\n"
            "Rss:                 4 kB\n"
            "Pss:                 4 kB\n"
        )

        summary = parse_smaps_text(text, re.compile(r"\[anonymous\]"))
        assert summary is not None

        self.assertEqual(summary.breakdown,
                         {"[anonymous]": MemProfile(4, 4, 4, 0, 0, 0, 0, 0, 0)})
        self.assertEqual(summary.total,
                         MemProfile(4, 4, 4, 0, 0, 0, 0, 0, 0))

    def test_tag_filter_no_match_returns_none(self) -> None:
        text = (_FIXTURES / "smaps_shell.smaps").read_text()

        summary = parse_smaps_text(text, re.compile(r"no-such-tag"))

        self.assertIsNone(summary)

    def test_tag_filter_none_unchanged(self) -> None:
        text = (_FIXTURES / "smaps_shell.smaps").read_text()
        summary = parse_smaps_text(text, None)
        assert summary is not None

        self.assertEqual(summary, parse_smaps_text(text))


if __name__ == "__main__":
    unittest.main()
