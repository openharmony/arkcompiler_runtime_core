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

/**
 * This test checks functionallity that we can await for standalone promise
 * that will be resolved in then callback of another promise
 * that will be resolved in another coroutine
 */

function foo() {}

let resolvePromiseFn: ((value: String) => void) | null = null;

function main() {
    let p1 = new Promise<String>((resolve: (value: String) => void): void => {
        resolvePromiseFn = resolve;
    });

    let p2 = launch foo();

    let p3 = p1.then<Number>((value: String): Number => {
        assertEQ(value, "Panda");
        return 42;
    });

    p2.then<void>((): void => {
        resolvePromiseFn!("Panda");
    });

    let value = await p3;
    assertEQ(value, 42);
}
