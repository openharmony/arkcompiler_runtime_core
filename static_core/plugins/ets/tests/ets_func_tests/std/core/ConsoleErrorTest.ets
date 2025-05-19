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

class A {
    name: string
    age: int
    constructor(name: string, age: int)
    {
        this.name = name
        this.age = age
    }
}

function main() {
    // Basic values
    console.error("Simple string message");
    console.error(42);
    console.error(true);
    console.error(undefined);
    console.error(null);
    console.error()

    // Multiple arguments
    console.error("User:", "John", "Age:", 30);

    // Objects and arrays
    console.error(new A("John", 30));
    console.error([1, 2, 3, 4, 5]);

    // Template strings
    const name = "Alice";
    console.error(`Hello, ${name}!`);

    // Styled console output
    console.error(
        "%c Styled text",
        "color: blue;"
    );

    // Error objects
    console.error(new Error("Test error"));

    console.error(null, undefined, 11, "123", new Object())
    let arr = new Array<string | null | undefined>(3)
    console.error(arr)

    // Formatted strings
    console.error("%d")
    console.error("%d", 1)
    console.error("%d", 1, 2)

    console.error("%s %s", "37", 37)
    console.error("%s %s", "19.5", 19.5)
}
