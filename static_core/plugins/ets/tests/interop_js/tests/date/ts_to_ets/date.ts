/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
const dateString = '2025-03-01T01:02:03.000Z';
const timeStamp = 1609459200000;

let funDateInstanceOf = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateInstanceOf');
let funDateTypeOf = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateTypeOf');
let funDateGetFullYear = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetFullYear');
let funDateGetMonth = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetMonth');
let funDateGetDate = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetDate');
let funDateGetDay = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetDay');
let funDateGetHours = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetHours');
let funDateGetMinutes = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetMinutes');
let funDateGetSeconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetSeconds');
let funDateGetMilliseconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetMilliseconds');
let funDateToISOString = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateToISOString');
let funDateGetTime = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetTime');
let funObject = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funObject');
let funDateGetUTCDate = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCDate');
let funDateGetUTCFullYear = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCFullYear');
let funDateGetUTCMonth = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCMonth');
let funDateGetUTCDay = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCDay');
let funDateGetUTCHours = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCHours');
let funDateGetUTCMinutes = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCMinutes');
let funDateGetUTCSeconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCSeconds');
let funDateGetUTCMilliSeconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetUTCMilliSeconds');
let funDateGetTimeZoneOffSet = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateGetTimeZoneOffSet');
let funDateToJSON = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateToJSON');
let funDateToDateString = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateToDateString');
let funDateValueOf = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'funDateValueOf');

let testSetDateOfSetTime = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetTime');
let testSetDateOfSetDate = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetDate');
let testSetDateOfSetFullYear = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetFullYear');
let testSetDateOfSetMonth = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetMonth');
let testSetDateOfSetUTCDate = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetUTCDate');
let testSetDateOfSetUTCFullYear = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetUTCFullYear');
let testSetDateOfSetUTCMonth = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfSetUTCMonth');
let testSetDateOfMilliSeconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testSetDateOfMilliSeconds');
let testDateOfSetUTCMilliSeconds = etsVm.getFunction('Ltest_date/ETSGLOBAL;', 'testDateOfSetUTCMilliSeconds');

let dateStr = new Date(dateString);
let dateTimeStamp = new Date(timeStamp);

ASSERT_TRUE(funObject(dateStr));
ASSERT_TRUE(funDateInstanceOf(dateStr));
ASSERT_TRUE(funDateTypeOf(dateStr));
ASSERT_TRUE(funDateGetFullYear(dateStr));
ASSERT_TRUE(funDateGetMonth(dateStr));
ASSERT_TRUE(funDateGetDate(dateStr));
ASSERT_TRUE(funDateGetDay(dateStr));
ASSERT_TRUE(funDateGetHours(dateStr));
ASSERT_TRUE(funDateGetMinutes(dateStr));
ASSERT_TRUE(funDateGetSeconds(dateStr));
ASSERT_TRUE(funDateGetMilliseconds(dateStr));
ASSERT_TRUE(funDateToISOString(dateStr));
ASSERT_TRUE(funDateGetTime(dateTimeStamp));
ASSERT_TRUE(funDateGetUTCDate(dateTimeStamp, dateTimeStamp.getUTCDate()));
ASSERT_TRUE(funDateGetUTCFullYear(dateTimeStamp, dateTimeStamp.getUTCFullYear()));
ASSERT_TRUE(funDateGetUTCMonth(dateTimeStamp, dateTimeStamp.getUTCMonth()));
ASSERT_TRUE(funDateGetUTCDay(dateTimeStamp, dateTimeStamp.getUTCDay()));
ASSERT_TRUE(funDateGetUTCHours(dateTimeStamp, dateTimeStamp.getUTCHours()));
ASSERT_TRUE(funDateGetUTCMinutes(dateTimeStamp, dateTimeStamp.getUTCMinutes()));
ASSERT_TRUE(funDateGetUTCSeconds(dateTimeStamp, dateTimeStamp.getUTCSeconds()));
ASSERT_TRUE(funDateGetUTCMilliSeconds(dateTimeStamp, dateTimeStamp.getUTCMilliseconds()));
ASSERT_TRUE(funDateGetTimeZoneOffSet(dateTimeStamp, dateTimeStamp.getTimezoneOffset()));
ASSERT_TRUE(funDateToJSON(dateTimeStamp, dateTimeStamp.toJSON()));
ASSERT_TRUE(funDateToDateString(dateTimeStamp, dateTimeStamp.toDateString()));
ASSERT_TRUE(funDateValueOf(dateTimeStamp, dateTimeStamp.valueOf()));

ASSERT_TRUE(testSetDateOfSetTime(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetDate(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetFullYear(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetMonth(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetUTCDate(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetUTCFullYear(dateTimeStamp));
ASSERT_TRUE(testSetDateOfSetUTCMonth(dateTimeStamp));
ASSERT_TRUE(testSetDateOfMilliSeconds(dateTimeStamp));
ASSERT_TRUE(testDateOfSetUTCMilliSeconds(dateTimeStamp));
