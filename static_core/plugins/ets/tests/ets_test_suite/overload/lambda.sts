/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

let void_called: boolean = false;
let int_called = false;

function reset(): void {
    void_called = false;
    int_called = false;
}

function foo(callback: () => void): void {
    void_called = true;
}

function foo(callback: (x: int) => void): void {
    int_called = true;
}

function main(): void {
    reset();
    foo((): void => {
    });
    assertTrue(void_called);
    assertFalse(int_called);

    reset();
    foo((x: int): void => {
    });
    assertFalse(void_called);
    assertTrue(int_called);
}
