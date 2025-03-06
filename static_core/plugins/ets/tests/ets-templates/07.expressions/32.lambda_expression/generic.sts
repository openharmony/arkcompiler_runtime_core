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
desc: Generic declaration
---*/

let f = <T1, T2>(a: T1, b: T2): [T1, T2] => [a, b]


let x = f(true, 1234.0)
assertTrue(x[0] == true && x[1] == 1234.0, "boolean/float failed")

let y = f(1, "test")
assertTrue(y[0] == 1 && y[1] == "test", "int/string failed")
