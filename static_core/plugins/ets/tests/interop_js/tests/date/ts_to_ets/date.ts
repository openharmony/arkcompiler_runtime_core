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

let FunDateInstanceof = etsVm.getFunction('LETSGLOBAL;', 'FunDateInstanceof');
let FunDatetypeof = etsVm.getFunction('LETSGLOBAL;', 'FunDatetypeof');
let FunDategetFullYear = etsVm.getFunction('LETSGLOBAL;', 'FunDategetFullYear');
let FunDategetMonth = etsVm.getFunction('LETSGLOBAL;', 'FunDategetMonth');
let FunDategetDate = etsVm.getFunction('LETSGLOBAL;', 'FunDategetDate');
let FunDategetDay = etsVm.getFunction('LETSGLOBAL;', 'FunDategetDay');
let FunDategetHours = etsVm.getFunction('LETSGLOBAL;', 'FunDategetHours');
let FunDategetMinutes = etsVm.getFunction('LETSGLOBAL;', 'FunDategetMinutes');
let FunDategetSeconds = etsVm.getFunction('LETSGLOBAL;', 'FunDategetSeconds');
let FunDategetMilliseconds = etsVm.getFunction('LETSGLOBAL;', 'FunDategetMilliseconds');
let FunDatetoISOString = etsVm.getFunction('LETSGLOBAL;', 'FunDatetoISOString');
let FunDategetTime = etsVm.getFunction('LETSGLOBAL;', 'FunDategetTime');
let FunObject = etsVm.getFunction('LETSGLOBAL;', 'FunObject');


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
