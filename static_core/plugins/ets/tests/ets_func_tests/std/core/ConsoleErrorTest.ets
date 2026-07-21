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

function main() {
    // Basic values
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('Simple string message');
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(42);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(true);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(undefined);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(null);
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error()

    // Multiple arguments
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('User:', 'John', 'Age:', 30);

    // Objects and arrays
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(new A('John', 30));

    // Template strings
    const name = 'Alice';
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(`Hello, ${name}!`);

    // Styled console output
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(
        '%c Styled text',
        'color: blue;'
    );

    // Error objects
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(new Error('Test error'));

    let obj: Object = {}
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(null, undefined, 11, '123', obj)
    let arr = Array.create<string | null | undefined>(3, undefined)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error(arr)

    // Formatted strings
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('%d')
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('%d', 1)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('%d', 1, 2)

    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('%s %s', '37', 37)
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.error('%s %s', '19.5', 19.5)
}
