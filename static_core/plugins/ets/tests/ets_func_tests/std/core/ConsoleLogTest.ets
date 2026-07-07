/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class A {
    public name: string
    public age: int
    constructor(name: string, age: int) {
        this.name = name
        this.age = age
    }
}

class B {
    public buf: ArrayBuffer
    constructor(b: ArrayBuffer) {
        this.buf = b
    }
}

function main() {
    // Basic values
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('Simple string message');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(42);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(true);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(undefined);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(null);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log()

    // Multiple arguments
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('User:', 'John', 'Age:', 30);

    // Object
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(new A('John', 30));

    // Template strings
    const name = 'Alice';
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(`Hello, ${name}!`);

    // Styled console output
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(
        '%c Styled text',
        'color: blue;'
    );

    // Error objects
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(new Error('Test error'));

    let obj: Object = {}
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(null, undefined, 11, '123', obj)
    let arr = Array.create<string | null | undefined>(3, undefined)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(arr)

    // Formatted strings
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('%d')
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('%d', 1)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('%d', 1, 2)

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('%s %s', '37', 37)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log('%s %s', '19.5', 19.5)
    
    // ArrayBuffer tests
    const buf1 = new ArrayBuffer(3);
    const view1 = new Uint8Array(buf1);
    view1[0] = 1;
    view1[1] = 2;
    view1[2] = 3;

    const buf2 = new ArrayBuffer(2);
    const view2 = new Uint8Array(buf2);
    view2[0] = 4;
    view2[1] = 5;
    
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(buf1); 
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(new ArrayBuffer(0)); 
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log([new B(buf1), new B(buf2)]);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(new B(buf1), new B(buf2));

    const arrayBuffer = new ArrayBuffer(10);
    const arrayBufferValue = reflect.Value.of(arrayBuffer);
    const result = arrayBufferValue.toPrint(0).toString();
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.log(result);
}