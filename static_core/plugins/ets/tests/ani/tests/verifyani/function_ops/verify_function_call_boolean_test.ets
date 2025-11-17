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

class Parent {
    public booleanMethod(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string): boolean {
        return true;
    }
}

function booleanFunction(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string): boolean {
    return false;
}

function intFunction(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string): int {
    return 24;
}

function voidParamFunction(): boolean {
    return true;
}

native function bar(m: long): void;
native function foo(): void;
native function baz(): void;
