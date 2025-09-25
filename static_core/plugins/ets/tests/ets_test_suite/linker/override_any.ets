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

class StubError extends Error { }

class NeverError extends Error { }

function stub() {
    throw new StubError();
}

function neverErr(): never {
    throw new NeverError();
}

class OverrideParamAny {
    paramAny(a: Object): void { stub() }
    paramAny(a: FixedArray<int>): void { stub() }
    paramAny(a: string | double): void { stub() }
    paramAny(a: never): void { stub() }
}

class Base extends OverrideParamAny {
    static create(): Base { return new Base() }

    paramAny(a: Any): void {}

    returnAny(): Any { stub(); return 1 }
}

class ReturnObj extends Base {
    static create(): Base { return new ReturnObj() }

    returnAny(): Object { return new Object() }
}

class ReturnArray extends Base {
    static create(): Base { return new ReturnArray() }

    returnAny(): FixedArray<int> { return [0, 1, 2] }
}

class ReturnUnion extends Base {
    static create(): Base { return new ReturnUnion() }

    returnAny(): string | double { return 'Hi' }
}

class ReturnNever extends Base {
    static create(): Base { return new ReturnNever() }

    returnAny(): never { return neverErr() }
}

class T {}

function overrideMethods() {
    let baseClass: Base = Base.create();
    let retObj: Base = ReturnObj.create();
    let retArr: Base = ReturnArray.create();
    let retUn: Base = ReturnUnion.create();
    let retNever: Base = ReturnNever.create();

    let param1: Object = new Object()
    let param2: FixedArray<int> = [0, 1, 2]
    let param3: string | double = ''
    let param4: T = new T()

    // success overriding of paramAny() method
    baseClass.paramAny(param1);
    baseClass.paramAny(param2);
    baseClass.paramAny(param3);
    baseClass.paramAny(param4);

    // success overriding of returnAny() method
    retObj.returnAny();
    retArr.returnAny();
    retUn.returnAny();
    arktest.expectThrow(() => { retNever.returnAny() }, (e) => e instanceof NeverError)
}

function main() {
    overrideMethods();
}
