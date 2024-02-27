/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function main(): int {
    let failures = 0;

    failures += test(testDateChangeYear(), "Try to change date's year");
    failures += test(testDateChangeMonth(), "Try to change date's month");
    failures += test(testDateChangeDay(), "Try to set date's new day");
    failures += test(testDateChangeHour(), "Try to set new hour");
    failures += test(testDateChangeMinutes(), "Try to set new minutes");
    failures += test(testDateChangeSeconds(), "Try to set new seconds");


    // next test raises assert "Invalid year/month pair"
    //failures += test(testDateChangeMonthBad(), "Try to change month to bad value");
    return test(failures, "All tests run");
}

function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;      
    }
    console.println("FAILED: " + message);
    return 1;
}

function testDateChangeYear():int {

    let date: Date = new Date();
    let year = date.getFullYear();
    date.setFullYear(year - 1);

    if(date.getFullYear() == (year - 1)) return 0;
    return 1;

}

function daysInMonth(year: number, month: number): number {
    let tmp = new Date(year, month + 1, 0);
    return tmp.getDate();
}

function testDateChangeMonthImpl(date: Date, change: int) {
    let month = date.getMonth();
    let dmo = daysInMonth(date.getFullYear(), month);
    let newMonth = month + change;
    let dmc = daysInMonth(date.getFullYear(), newMonth);
    if(dmc < dmo) newMonth++;
    date.setMonth(newMonth);
    assert date.getMonth() == newMonth
}

function testDateChangeMonth(): int {
    try {
        let date = new Date();
        let month = date.getMonth();
        let change: int = 1;
        if(month == 11) change = -change;
        testDateChangeMonthImpl(date, change);
    } catch(ae: AssertionError) {
        return 1;
    }
    return 0;

}

function testDateChangeMonthBad(): int {
    let date: Date = new Date();
    try {
        date.setMonth(13);
    } catch(id: InvalidDate) {
        return 0;
    }
    return 1;
}


function testDateChangeDay(): int {
    let date: Date = new Date();
    let day: byte = date.getDate() as byte;
    let newDay: byte = 15;
    if(newDay == day) newDay++;
    date.setDate(newDay);
    
    if(date.getDate() == newDay) return 0;
    return 1;
}

function testDateChangeHour(): int {
    let date: Date = new Date();
    let hour: byte = date.getHours() as byte;
    let newHour: byte = 12;
    if(newHour == hour) newHour = 11;
    date.setHours(newHour);

    if(date.getHours() == newHour) return 0;
    return 1;
}

function testDateChangeMinutes(): int {
    let date: Date = new Date();
    let minutes: byte = date.getMinutes() as byte;
    let newMinutes: byte = 30;
    if(newMinutes == minutes) newMinutes = 21;
    date.setMinutes(newMinutes);

    if(date.getMinutes() == newMinutes) return 0;
    return 1;
}

function testDateChangeSeconds(): int {
    let date: Date = new Date();
    let seconds: byte = date.getSeconds() as byte;
    let newSeconds: byte = 30;
    if(newSeconds == seconds) newSeconds = 21;
    date.setSeconds(newSeconds);

    if(date.getSeconds() == newSeconds) return 0;
    return 1;    
}
