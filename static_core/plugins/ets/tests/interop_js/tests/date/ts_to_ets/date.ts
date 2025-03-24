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

let FunDateInstanceof = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDateInstanceof');
let FunDatetypeof = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDatetypeof');
let FunDategetFullYear = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetFullYear');
let FunDategetMonth = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetMonth');
let FunDategetDate = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetDate');
let FunDategetDay = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetDay');
let FunDategetHours = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetHours');
let FunDategetMinutes = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetMinutes');
let FunDategetSeconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetSeconds');
let FunDategetMilliseconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetMilliseconds');
let FunDatetoISOString = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDatetoISOString');
let FunDategetTime = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunDategetTime');
let FunObject = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'FunObject');


let datestring = new Date(datestr);
let datetimestamp = new Date(timestamp);

ASSERT_TRUE(FunObject(datestring));
ASSERT_TRUE(FunDateInstanceof(datestring));
ASSERT_TRUE(FunDatetypeof(datestring));
ASSERT_TRUE(FunDategetFullYear(datestring));
ASSERT_TRUE(FunDategetMonth(datestring));
ASSERT_TRUE(FunDategetDate(datestring));
ASSERT_TRUE(FunDategetDay(datestring));
ASSERT_TRUE(FunDategetHours(datestring));
ASSERT_TRUE(FunDategetMinutes(datestring));
ASSERT_TRUE(FunDategetSeconds(datestring));
ASSERT_TRUE(FunDategetMilliseconds(datestring));
ASSERT_TRUE(FunDatetoISOString(datestring));
ASSERT_TRUE(FunDategetTime(datetimestamp));
