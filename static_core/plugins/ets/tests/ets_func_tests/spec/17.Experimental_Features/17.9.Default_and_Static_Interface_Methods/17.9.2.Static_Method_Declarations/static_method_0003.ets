/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 desc: 17.9.1 Experimental Features
 tags: [negative]
 ---*/


interface I1  {
    static foo(): int {return 11;}
    static bar(): int {return 12;}
}

interface I2 extends I1 {
    static foo(): int {return 21;}
    static bar(): int {return 22;}
}


class C implements I1, I2 {
}

function main() : int {
    assert C.foo() == 11;
    assert C.bar() == 12;
    return 0;
}
