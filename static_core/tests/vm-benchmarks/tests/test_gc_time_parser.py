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

import unittest

from vmb.gclog.fw_time_parser import FwTimeParser


class FwTimeParserTests(unittest.TestCase):

    def test_parse_text(self):
        fw_start_time = 1624469473444
        fw_end_time = 1624469503456
        vm_start_time = 116359999212857
        text = """
1624469503548 INFO [Host] - VM 1/1 output
{} INFO - Startup execution started: {}
1624469476451 INFO - Warmup 1: 8387 ops, 357863.083105 ns/op
1624469479452 INFO - Warmup 2: 9280 ops, 323313.791272 ns/op
1624469482453 INFO - Warmup 3: 9239 ops, 324789.918714 ns/op
1624469485453 INFO - Warmup 4: 9281 ops, 323282.864131 ns/op
1624469488454 INFO - Iter 1: 8982 ops, 333967.421176 ns/op
1624469491454 INFO - Iter 2: 9100 ops, 329618.112747 ns/op
1624469494454 INFO - Iter 3: 9112 ops, 329214.669008 ns/op
1624469497454 INFO - Iter 4: 9286 ops, 322999.009584 ns/op
1624469500455 INFO - Iter 5: 9334 ops, 321456.791408 ns/op
1624469503456 INFO - Iter 6: 9223 ops, 325344.669413 ns/op
{} INFO - Benchmark result: FragmentedHeap_allocateYoungAndSave 327044.073024
        """.format(fw_start_time, vm_start_time, fw_end_time)

        result = FwTimeParser().parse_text(text)
        self.assertEqual(fw_start_time * 1000.0 * 1000.0, result.get('fw_start_time'))
        self.assertEqual(fw_end_time * 1000.0 * 1000.0, result.get('fw_end_time'))
        self.assertEqual(vm_start_time, result.get('vm_start_time'))


if __name__ == '__main__':
    unittest.main()
