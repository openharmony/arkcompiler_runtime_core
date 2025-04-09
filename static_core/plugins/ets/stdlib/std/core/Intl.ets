/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    export type BCP47LanguageTag = string
    export type DisplayNamesFallback = "code" | "none"
    export type DisplayNamesLanguageDisplay = "dialect" | "standard"
    export type DisplayNamesType = "language" | "region" | "script" | "calendar" | "dateTimeField" | "currency"
    export type ES2018NumberFormatPartType = "literal" | "nan" | "infinity" | "percent" | "integer" | "group" | "decimal" | "fraction" | "plusSign" | "minusSign" | "percentSign" | "currency" | "code" | "symbol" | "name"
    export type ES2020NumberFormatPartType = "compact" | "exponentInteger" | "exponentMinusSign" | "exponentSeparator" | "unit" | "unknown"
    export type LDMLPluralRule = "zero" | "one" | "two" | "few" | "many" | "other"
    export type ListFormatLocaleMatcher = "lookup" | "best fit"
    export type ListFormatStyle = "long" | "short" | "narrow"
    export type ListFormatType = "conjunction" | "disjunction" | "unit"
    export type LocaleCollationCaseFirst = "upper" | "lower" | "false"
    export type LocaleHourCycleKey = "h12" | "h23" | "h11" | "h24"
    export type NumberFormatPartTypes = ES2018NumberFormatPartType | ES2020NumberFormatPartType;
    export type PluralRuleType = "cardinal" | "ordinal"
    export type RelativeTimeFormatLocaleMatcher = "lookup" | "best fit"
    export type RelativeTimeFormatNumeric = "always" | "auto"
    export type RelativeTimeFormatStyle = "long" | "short" | "narrow";
    export type RelativeTimeFormatUnit = "year" | "years" | "quarter" | "quarters" | "month" | "months" | "week" | "weeks" | "day" | "days" | "hour" | "hours" | "minute" | "minutes"| "second" | "seconds"
    export type RelativeTimeFormatUnitSingular =  "year" | "quarter" | "month" | "week" | "day" | "hour" | "minute" | "second"
    export type UnicodeBCP47LocaleIdentifier = string;
    export type DateTimeFormatPartTypes = "day" | "dayPeriod" | "era" | "fractionalSecond" | "hour" | "literal" | "minute" | "month" | "second" | "timeZoneName" | "unknown" | "weekday" | "year";
    export type DateTimeRangeFormatPartSource = "startRange" | "endRange" | "shared"

}
