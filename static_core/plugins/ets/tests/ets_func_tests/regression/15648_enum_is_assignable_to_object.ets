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

 /*---
 desc: Issue 15648. Instance of Enum must be assignable in object
 tags: []
 ---*/

enum Color {Red, Green, Blue}

function checkInstanceOf(o: Object): boolean {
    let cr = o instanceof Color
    return cr
}

function main() {
    let c: Color = Color.Red;
    let o: Object = c;
    assertTrue(checkInstanceOf(o));

    let c2: Color = Color.Green;
    let o2: object = c2;
    assertTrue(checkInstanceOf(o2));

}
