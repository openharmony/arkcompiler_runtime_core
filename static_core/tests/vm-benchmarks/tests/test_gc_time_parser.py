#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

import unittest

from vmb.gclog.fw_time_parser import FwTimeParser


class FwTimeParserTests(unittest.TestCase):

    def test_parse_text(self):
        fw_start_time = 1772113412313872014
        fw_end_time = 1772113425669628037
        vm_start_time = 1772113412313830511
        format1 = f'''
1772113412313872013 INFO [Host] - VM 1/1 output
1772113412313872014 Startup execution started: 1772113412313830511
1772113412314816621 Starting ArraySort_sort @ distribution="sorted";size=100...
1772113418071415812 Warmup 1:1349620 ops, 1413.4493175856908 ns/op
1772113419964128722 Warmup 2:1349620 ops, 1402.174200145226 ns/op
1772113421844422082 Iter 1:1349620 ops, 1393.0042048873015 ns/op
1772113423731378243 Iter 2:1349620 ops, 1397.9681710407374 ns/op
1772113425669414820 Iter 3:1349620 ops, 1435.7891376831997 ns/op
1772113425669628037 Benchmark result: ArraySort_sort 1408.9205045370795'''
        format2 = f'''
1772113412313872013 INFO [Host] - VM 1/1 output
1772113412313872014 Startup execution started: 1772113412313830511
1772113412314816621 Starting ArraySort_sort @ distribution="sorted";size=100...
1772113418071415812 Warmup 1:1349620 ops, 1413.4493175856908 ns/op
1772113425669414820 Iter 1:1349620 ops, 1435.7891376831997 ns/op
1772113425669628037 Benchmark result: ArraySort_sort 1435.7891376831997'''

        for text in (format1, format2):
            result = FwTimeParser().parse_text(text)
            self.assertIsNotNone(result, 'Parsing fw time failed')
            self.assertEqual(fw_start_time,
                             result.get('fw_start_time'))
            self.assertEqual(fw_end_time,
                             result.get('fw_end_time'))
            self.assertEqual(vm_start_time,
                             result.get('vm_start_time'))


if __name__ == '__main__':
    unittest.main()
