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
    console.log("Simple string message");
    console.log(42);
    console.log(true);
    console.log(undefined);
    console.log(null);
    console.log()

    // Multiple arguments
    console.log("User:", "John", "Age:", 30);

    // Object
    console.log(new A("John", 30));

    // Template strings
    const name = "Alice";
    console.log(`Hello, ${name}!`);

    // Styled console output
    console.log(
        "%c Styled text",
        "color: blue;"
    );

    // Error objects
    console.log(new Error("Test error"));

    console.log(null, undefined, 11, "123", new Object())
    let arr = new Array<string | null | undefined>(3)
    console.log(arr)

    // Formatted strings
    console.log("%d")
    console.log("%d", 1)
    console.log("%d", 1, 2)

    console.log("%s %s", "37", 37)
    console.log("%s %s", "19.5", 19.5)
}
