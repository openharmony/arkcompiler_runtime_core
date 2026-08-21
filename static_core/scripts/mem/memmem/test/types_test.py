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

import unittest

from src.types import positive_int, uint


class TypesTest(unittest.TestCase):
    def test_uint_accepts_zero(self) -> None:
        self.assertEqual(uint(0), 0)

    def test_uint_accepts_positive(self) -> None:
        self.assertEqual(uint(5), 5)

    def test_uint_rejects_negative(self) -> None:
        with self.assertRaises(ValueError):
            uint(-1)

    def test_positive_int_accepts_positive(self) -> None:
        self.assertEqual(positive_int(5), 5)

    def test_positive_int_rejects_zero(self) -> None:
        with self.assertRaises(ValueError):
            positive_int(0)

    def test_positive_int_rejects_negative(self) -> None:
        with self.assertRaises(ValueError):
            positive_int(-1)


if __name__ == "__main__":
    unittest.main()
