/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package escompat;
// Note: This file initialize all the `const` property that non-immediately initialized(spec-issue 217).
static {
    FROM_BUFFER = new FromBuffer()
    BigIntType = Type.of(new BigInt());
    hexChars = [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'A', c'B', c'C', c'D', c'E', c'F'];
    uriArray = fillInURINotComponent();
    uriSpecialChars = createAndFillInURISpecial();
    uriArrayComponent = fillInCommonURI();
    NaN = Double.NaN;
    Infinity = Double.POSITIVE_INFINITY;
    IS_LITTLE_ENDIAN = Runtime.isLittleEndianPlatform()

    dayNames = [
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    ];

    monthNames = [
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    ];

    firstDayInMonthNormal = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334];
    firstDayInMonthLeap = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335];

    maxDateOffset = 8.64e15.toLong();
    INVALID_DATE = maxDateOffset + 1;
}
