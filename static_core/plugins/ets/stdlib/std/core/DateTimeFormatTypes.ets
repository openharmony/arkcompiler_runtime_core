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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;

export namespace Intl {

    export interface ResolvedDateTimeFormatOptions {
        calendar: string;
        dateStyle?: "full" | "long" | "medium" | "short";
        day?: string;
        dayPeriod?: "narrow" | "short" | "long";
        era?: string;
        formatMatcher?: "basic" | "best fit" | "best fit";
        fractionalSecondDigits?: number;
        hour12?: boolean;
        hour?: string;
        hourCycle?: "h11" | "h12" | "h23" | "h24";
        locale: string;
        minute?: string;
        month?: string;
        numberingSystem: string;
        second?: string;
        timeStyle?: "full" | "long" | "medium" | "short";
        timeZone: string;
        timeZoneName?: string;
        weekday?: string;
        year?: string;
    }

    export interface DateTimeFormatPart {
        type: DateTimeFormatPartTypes;
        value: string;
    }

    export interface DateTimeRangeFormatPart extends DateTimeFormatPart {
        source: DateTimeRangeFormatPartSource
    }

}
