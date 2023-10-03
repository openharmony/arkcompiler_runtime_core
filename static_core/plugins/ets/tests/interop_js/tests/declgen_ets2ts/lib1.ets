/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

import { Lib2Class, Lib2Class2 /*as Lib2Class2Alias*/, DoubleAlias } from './lib2';

import { DynClass } from "/plugins/ets/tests/interop_js/tests/declgen_ets2ts/lib3.js"

// ================ Functions ================

export function SumDouble(a: double, b: double): double {
    return a + b;
}

export function SumString(a: string, b: string): string {
    return a + b;
}

export function Identity<T>(x: T): T {
    return x;
}

export function ForEach<T extends Object>(a: T[], cb: (e: T, idx: int) => void) {
    for (let i = 0; i < a.length; i++) {
        cb(a[i], i);
    }
}

export function ReplaceFirst(arr: Simple[], e: Simple): Simple[] {
    arr[0] = e;
    return arr;
}

// ================ Fields ================

export class Fields {
    readonly readonlyBoolean_: boolean = true;
    public static staticFloat_: float = 0.0;
    public double_: double = 2.2;
    public doubleAlias_: DoubleAlias = 3.3;
    int_: int = 4;
    string_: String = "Panda string!!";
    nullableString_: string | null = "nullable string";
    object_: Object = new Object();
    rangeError_: RangeError = new RangeError();
    uint8Array_: Uint8Array = new Uint8Array();
    uint16Array_: Uint16Array = new Uint16Array();
    uint32Array_: Uint32Array = new Uint32Array();
};
Fields.staticFloat_ = 1.1;

// ================ Methods ================

export class Methods {
    IsTrue(a: boolean): boolean {
        return a;
    }

    SumString(a: string, b: string): string {
        return a + b;
    }

    static StaticSumDouble(a: double, b: double): double {
        return a + b;
    }

    SumIntArray(a: int[]): int {
        let res: int = 0;
        for (let i = 0; i < a.length; i++) {
            res += a[i];
        }
        return res;
    }

    OptionalString(a: string | null): string | null {
        return a;
    }

    // TODO(ivagin): enable when supported by es2panda #12808
    //SumIntVariadic(a: int, ...b: int[]): int {
    //    for (let i = 0; i < b.length; i++) {
    //        a += b[i];
    //    }
    //    return a;
    //}
};

// ================ Classes/Interfaces ================

export enum ETSEnum { Val0, Val1, Val2 };

export interface I0 {
    I0Method(a: string): string { return a; };
}

export interface I1 {
    I1Method(a: double): double { return a; };
}

export class Base {
    a: double = 1;
}

export class Derived extends Base implements I0, I1 {
    I0Method(a: string): string { return a; };
    I1Method(a: double): double { return a; };
    b: double = 2;
}

export interface Interface0 {
    private a(): void {};
    private b(): void {};
    static d(): void {};
}

class Simple {
    constructor(x: double) {
        this.val = x;
    }
    val: double = 322;
};

export class External {
    constructor() {
        this.o1 = new Simple(322);
        this.o2 = new Lib2Class();
        this.o3 = new Lib2Class2();
    }
    o1: Simple;
    o2: Lib2Class;
    o3: Lib2Class2;
};

// ================ Generics ================

export class GenericClass<T extends Object> {
    identity(x: T): T {
        return x;
    }
}

export interface IGeneric0<T> {
    I0Method(a: T): T { return a; };
}

export interface IGeneric1<T> {
    I1Method(a: T): T { return a; };
}

export class BaseGeneric<T, U> {
    c: T;
    d: U;
}

// TODO(ivagin): Enable when type arguments are supported by declgen/es2panda #12808
//export class DerivedGeneric<T, U, V> extends BaseGeneric<T, U> implements IGeneric0<U>, IGeneric1<IGeneric0<V>> {
//    I0Method(a: U): U { return a; };
//    I1Method(a: IGeneric0<V>): IGeneric0<V> { return a; };
//    e: T;
//}

// ================ Dynamic objects ================

export class DynObjWrapper {
    dynObj_: DynClass = new DynClass();
};

// ================ Promise/async ================

export class PromiseWrapper {
    promise_: Promise<String> = new Promise<String>((resolve: (value: String) => void): void => {
        resolve("Panda promise");
    });
};

// TODO(ivagin): enable when supported by ets2ts declgen
// async function AsyncIdentity(param: int): Promise<int> {
//     return param;
// }
