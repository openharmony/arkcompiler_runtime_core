/*
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

class NeverError extends Error { }

function neverErr(): never {
    throw new NeverError();
}

interface ReturnObj {
    returnNever(): Object { return new Object() }
}

interface ReturnArray {
    returnNever(): FixedArray<int> { return [0, 1, 2] }
}

interface ReturnUnion {
    returnNever(): string | double { return "Hi" }
}

interface ReturnAny {
    returnNever(): Any { return null }
}

class Base implements ReturnObj, ReturnArray, ReturnUnion {
    static create(): Base { return new Base() }

    returnNever(): never { return neverErr() }
}

class T {}

function overrideMethods() {
    let baseClass: Base = Base.create();

    // success overriding of returnNever() method
    arktest.expectThrow(() => { baseClass.returnNever() }, (e) => e instanceof NeverError)
}

function main() {
    overrideMethods();
}
