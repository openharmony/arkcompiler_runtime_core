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

/*---
desc: >-
    11 Enumeration
name: 11.enumeration
tags: []
---*/

enum ConstExpr {
    A = 5 * 2,
    B = 77 / 7,
    C = 28 % 15,
    D = 15 + (2 * (5 + 4) - 3) / 5,
}

function main(): void {
    assert ConstExpr.A.getValue() == 10
    assert ConstExpr.B.getValue() == 11
    assert ConstExpr.C.getValue() == 13
    assert ConstExpr.D.getValue() == 18
}
