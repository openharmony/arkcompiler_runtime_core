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
 desc: 03.8.10 Tuple Types
 name: func_tuple_1.ets
 tags: []
 ---*/

 type Func1 = (value: String) => string;
 type Func0 = () => number;

 type FuncTuple3 = [Func1, Func1, Func0]

 function main(): void {

    let t3: FuncTuple3 = [(value: String) => { return value; }, (value: String) => { return value; }, () => { return 42.0 }];

    assertEQ( t[0]("One"), "One")
    assertEQ( t[1]("Two"), "Two")
    assertEQ( t[2](), 42.0)
 }
