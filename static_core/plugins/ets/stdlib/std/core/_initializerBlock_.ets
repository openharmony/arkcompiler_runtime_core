/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

package std.core;
// Note: This file initialize all the `const` property that non-immediately initialized(spec-issue 217).
static {
    FROM_BUFFER = new FromBuffer()
    // 5 lower bits stores kind's id
    TypeKindMask = ((1 << 6) - 1).toByte();
    BigIntType = Class.from<BigInt>();
    ObjectType = Type.of(new Object()) as ClassType;
    console = Console.getInstance();
    coroutine = new Coroutine();
    mainEAWorker = new MainEAWorker();
    NaN = Double.NaN;
    Infinity = Double.POSITIVE_INFINITY;

    MAX_REGULAR_INTEGER = new BigInt(Number.MAX_SAFE_INTEGER)
    FAST_TO_STRING_HANDLER_MAP = () => {
        let result = new ToStringHandlerMap
        result[2] = (instance: BigInt):string => { return instance.toStringAsBinary() }
        result[4] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(4) }
        result[8] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(8) }
        result[10] = (instance: BigInt):string => { return instance.toStringAsDecimal() }
        result[16] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(16) }
        result[32] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(32) }
        return result
    }()

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
    maxDateYears = 275760;

    IS_LITTLE_ENDIAN = Runtime.isLittleEndianPlatform()

    uriArray = fillInURINotComponent();
    uriSpecialChars = createAndFillInURISpecial();
    uriArrayComponent = fillInCommonURI();

    digitChars = [
         c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h',
         c'i', c'j', c'k', c'l', c'm', c'n', c'o', c'p', c'q', c'r', c's', c't', c'u', c'v', c'w', c'x', c'y', c'z'
    ]
    hexChars = [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'A', c'B', c'C', c'D', c'E', c'F'];
    percentEncoded = [
        "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
        "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
        "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
        "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
        "%20", "%21", "%22", "%23", "%24", "%25", "%26", "%27",
        "%28", "%29", "%2A", "%2B", "%2C", "%2D", "%2E", "%2F",
        "%30", "%31", "%32", "%33", "%34", "%35", "%36", "%37",
        "%38", "%39", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
        "%40", "%41", "%42", "%43", "%44", "%45", "%46", "%47",
        "%48", "%49", "%4A", "%4B", "%4C", "%4D", "%4E", "%4F",
        "%50", "%51", "%52", "%53", "%54", "%55", "%56", "%57",
        "%58", "%59", "%5A", "%5B", "%5C", "%5D", "%5E", "%5F",
        "%60", "%61", "%62", "%63", "%64", "%65", "%66", "%67",
        "%68", "%69", "%6A", "%6B", "%6C", "%6D", "%6E", "%6F",
        "%70", "%71", "%72", "%73", "%74", "%75", "%76", "%77",
        "%78", "%79", "%7A", "%7B", "%7C", "%7D", "%7E", "%7F",
        "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
        "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
        "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
        "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
        "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
        "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
        "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
        "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
        "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
        "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
        "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
        "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
        "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
        "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
        "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
        "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
    ];
}
