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
import * as EnumTest from "@generated/enum_test";
import * as System from "std/core/System";

loadLibrary("ani_enum");

function test_nextEnum_with_color() {
    const color = EnumTest.Color.GREEN;
    const nextColor = EnumTest.nextEnum(color);
    assertEQ(nextColor, EnumTest.Color.BLUE)
}

function weekdayToString(day: EnumTest.Weekday): string {
    switch (day) {
        case EnumTest.Weekday.MONDAY:
            return "MONDAY";
        case EnumTest.Weekday.TUESDAY:
            return "TUESDAY";
        case EnumTest.Weekday.WEDNESDAY:
            return "WEDNESDAY";
        case EnumTest.Weekday.THURSDAY:
            return "THURSDAY";
        case EnumTest.Weekday.FRIDAY:
            return "FRIDAY";
        case EnumTest.Weekday.SATURDAY:
            return "SATURDAY";
        case EnumTest.Weekday.SUNDAY:
            return "SUNDAY";
        default:
            return "Unknown";
    }
}

function weekdayEnumInputTest() {
    let day = EnumTest.Weekday.WEDNESDAY;
    EnumTest.showEnumWeekday(day);
}

function weekdayEnumOuputTest() {
    let day = EnumTest.Weekday.WEDNESDAY;
    console.log(EnumTest.nextEnumWeekday(day))
    let weekdayName = weekdayToString(EnumTest.nextEnumWeekday(day));
    console.log(`nextEnumWeekday is : ${weekdayName}`);
    assertEQ(weekdayName, "THURSDAY");
}

function main() {
    console.log("##############start#############");
    const suite = new ArkTestsuite("EnumTest Tests");

    let color = EnumTest.Color.GREEN;
    EnumTest.showEnum(color);
    suite.addTest("test nextEnum with color", test_nextEnum_with_color);

    suite.addTest("weekday Enum Input Test", weekdayEnumInputTest)
    suite.addTest("weekday Enum Ouput Test", weekdayEnumOuputTest)

    System.exit(suite.run());
}
