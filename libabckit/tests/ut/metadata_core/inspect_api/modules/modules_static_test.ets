/**
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
import { Ns3 } from './import_test.ets';
import { testA } from './import_test.ets';

const moduleField: string = 'Module Field';

interface InterfaceTest {
    value: number;
    name: string;
}

let add = (a: number, b: number) => {
    return a + b;
}

class Test {
    static TestA: testA = new testA()
    a: number = 10
    b: Array<number> = [1, 2, 3]
    d: string = "abc"

    func() {
        console.log("this is class Test");
    }
}

namespace Ns2 {
interface InterfaceTest1 {}

enum EnumTest {
    Type1,
    Type2,
}
}