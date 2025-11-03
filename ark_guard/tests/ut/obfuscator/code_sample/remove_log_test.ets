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
class ClassB {
    foo() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log(1);
    }
}

function foo2() {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log();
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('hello');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('hello', 1);
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('hello', 1, 2, 3, 4, 'hello', 'hello');
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log(1);
}

namespace Ns1 {
    function bar(){
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('hello');
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('hello', 1);
    }
}

class ClassA {
    public method1(a: int, b: string, c: Array<int>) {
        let ret = `clsA: ${a}|${b}|${c.length}`;
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log(ret);
        return ret;
    }
}
function foo1(a: int, b: ClassA, c: string, d: Array<int>) {
    let ret = `foo1: ${b.method1(a, c, d)}`;
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log(ret);
}
let cls1 = new ClassA();
foo1(2, cls1, 'abc', [1, 2, 3]);