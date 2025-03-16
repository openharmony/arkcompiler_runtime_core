/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

let counter = 0;

function stack_eater(): void {
    stack_eater();
}

function coro_stack_overflow(): Object | null {
    stack_eater();
    return null;
}

function coro_error(): Object {
    throw new Error();
}

function test_stack_overflow(): boolean {
    try {
        await launch coro_stack_overflow();
        console.println("No exceptions thrown by coro_stack_overflow() but should be!")
        return false;
    } catch (e) {
        if (!(e instanceof StackOverflowError)) {
            console.println("Expected StackOverflowError but another exception has been thrown!");
            return false;
        }
        return true;
    }
}

function test_error(): boolean {
    try {
        await launch coro_error();
        console.println("No exceptions thrown by coro_error() but should be!")
        return false;
    } catch (e) {
        if (!(e instanceof Error)) {
            console.println("Expected Error but another exception has been thrown!");
            return false;
        }
        return true;
    }
}

export function main(): int {
    if (!test_stack_overflow()) {
        return 1;
    }
    if (!test_error()) {
        return 1;
    }
    return 0;
}
