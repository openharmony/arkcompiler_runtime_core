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

/*---
desc: Accessing common members with inheritance.
---*/

enum NumberEnum { One, Two };
enum StringEnum { One = "One", Two = "Two" };

type Union1 = number | NumberEnum;
type Union2 = string | StringEnum;

function main(): void {
  let numUnion: Union1 = NumberEnum.One;
  let strUnion: Union2 = StringEnum.Two;
  assertTrue(numUnion == 0);
}