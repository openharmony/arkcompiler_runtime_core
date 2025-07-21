/**
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


export function Identity<T>(x: T): T {
    return x;
}

export function ForEach<T extends Object>(a: FixedArray<T>, cb: (e: T, idx: int) => void) {
    for (let i = 0; i < a.length; i++) {
        cb(a[i], i);
    }
}

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
    constructor(c: T, d: U) {}
}

export class C1<T> {}
export function foo1<T>(x: Partial<C1<T>>): void {}

export type Callback<T, C = void, V = void> = (data: T, data2: C) => V;
export class C2 {
    a: Callback<string, int>
    constructor() {
        this.a = (data: string, data2: int) => {
        };
    }
}

// NOTE(ivagin): enable when supported by interop
// export class DerivedGeneric<T, U, V> extends BaseGeneric<T, U> implements IGeneric0<U>, IGeneric1<IGeneric0<V>> {
//    I0Method(a: U): U { return a; };
//    I1Method(a: IGeneric0<V>): IGeneric0<V> { return a; };
//    e: T;
//    constructor(c: T, d: U) {super(c, d)}
// }
