/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function main(): int {
    let dtfTestsuite = new arktest.ArkTestsuite("dateTimeFormatTest")
    dtfTestsuite.addTest("dateTimeFormatTest1", dateTimeFormatTest1)
    dtfTestsuite.addTest("dateTimeFormatTest2", dateTimeFormatTest2)
    dtfTestsuite.addTest("dateTimeFormatTest3", dateTimeFormatTest3)

    dtfTestsuite.addTest("dateToLocaleString1", dateToLocaleString1)
    dtfTestsuite.addTest("dateToLocaleString2", dateToLocaleString2)
    dtfTestsuite.addTest("dateToLocaleString3", dateToLocaleString3)

    return dtfTestsuite.run()
}

function dateTimeFormatTest1() {
    let newDate = new Date((24 * 60 * 60 * 1000).toLong() * (52 * 365 + 9 * 30 + 23));
    let options: Intl.DateTimeFormatOptions =
    {
        year: "numeric", month: "long", day: "numeric",
        hour: "2-digit", minute: "2-digit",
        timeZoneName: "short", timeZone: "UTC"
    };
    let intlObj = new Intl.DateTimeFormat(["pt-BR", "pt-PT"], options).format;
    arktest.assertEQ("8 de outubro de 2022 00:00 UTC", intlObj(newDate))
}

function dateTimeFormatTest2() {
    let newDate = new Date((24 * 60 * 60 * 1000).toLong() * (52 * 365 + 9 * 30 + 23));
    let swissLocales = ["de-CH", "fr-CH", "it-CH", "rm-CH"];
    let options: Intl.DateTimeFormatOptions =
    {
        weekday: "short",
        hour: "numeric", minute: "numeric",
        timeZone: "UTC", timeZoneName: "short"
    };
    let intlObj = new Intl.DateTimeFormat(swissLocales, options).format;
    arktest.assertEQ("Sa, 00:00 UTC", intlObj(newDate))
}

function dateTimeFormatTest3() {
    let newDate = new Date((24 * 60 * 60 * 1000).toLong() * (52 * 365 + 9 * 30 + 23));
    let options: Intl.DateTimeFormatOptions = {
        year: "numeric", month: "long", day: "numeric",
        hour: "2-digit", minute: "2-digit", timeZoneName: "short", timeZone: "UTC"
    };
    let intlObj = new Intl.DateTimeFormat("zh-CN", options);
    arktest.assertEQ('2022年10月8日 UTC 00:00', intlObj.format(newDate));
    arktest.assertEQ('UTC', intlObj.resolvedOptions().timeZone)
    arktest.assertEQ('short', intlObj.resolvedOptions().timeZoneName)

}

function dateToLocaleString1() {
    let date = new Date(Date.UTC(2022, 9, 20, 18, 0, 0));
    let value = date.toLocaleString('en-US', {timeZone: 'UTC'});
    let zerotime = "10/20/2022, 6:00:00 PM";
    arktest.assertEQ(value, zerotime)
}

function dateToLocaleString2() {
    let date = new Date(Date.UTC(2022, 9, 20, 18, 0, 0));
    let value = date.toLocaleString('ko-KR', {timeZone: 'UTC'});
    let zerotime = "2022. 10. 20. 오후 6:00:00";
    arktest.assertEQ(value, zerotime)
}

function dateToLocaleString3() {
    let date = new Date(Date.UTC(2022, 9, 20, 18, 0, 0));
    let value = date.toLocaleTimeString('it-IT', { hour12: false, timeZone: 'UTC', timeZoneName: 'short' });
    let zerotime = "18:00:00 UTC";
    arktest.assertEQ(value, zerotime)
}
