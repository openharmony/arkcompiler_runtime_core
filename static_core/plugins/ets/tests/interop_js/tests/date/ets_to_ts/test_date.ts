/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE- 2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const etsVm = globalThis.gtest.etsVm;
const datestr = '2025-03-01T01:02:03.000Z';
const timestamp = 1609459200000;
let firstdate = etsVm.getClass('Ldate/ETSGLOBAL;').firstdate;
let datestring = etsVm.getClass('Ldate/ETSGLOBAL;').datestring;
let datetimestamp = etsVm.getClass('Ldate/ETSGLOBAL;').datetimestamp;

ASSERT_TRUE(firstdate instanceof Date);
ASSERT_TRUE(typeof firstdate === 'object');
ASSERT_TRUE(firstdate.getFullYear() === 2025);
ASSERT_TRUE(firstdate.getMonth() === 2);
ASSERT_TRUE(firstdate.getDate() === 1);
ASSERT_TRUE(firstdate.getDay() === 6);
ASSERT_TRUE(firstdate.getHours() === 1);
ASSERT_TRUE(firstdate.getMinutes() === 2);
ASSERT_TRUE(firstdate.getSeconds() === 3);
ASSERT_TRUE(firstdate.getMilliseconds() === 4);
ASSERT_TRUE(datestring.toISOString() === datestr);
ASSERT_TRUE(datetimestamp.getTime() === timestamp);
