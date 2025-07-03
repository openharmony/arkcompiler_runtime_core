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

package ets_functions;

type refType = Object | ESValue;
native function saveHybridGref(ref: refType):void;

export function etsSaveNativeGrefChar(): void {
    let value : char = c'A'
    let ref = new Char(value);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefByte(): void {
    let value : byte = 0x12;
    let ref = new Byte(value);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefShort(): void {
    let value : short = 300;
    let ref = new Short(value);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefInt(): void {
    let ref = new Int(123456);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefDouble(): void {
    let ref = new Double(100.111);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefFloat(): void {
    let ref = new Float(1.5);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefString(): void {
    let ref = "hello world";
    saveHybridGref(ref);
}

export function etsSaveNativeGrefNumberArray(): void {
    let ref: Array<number> = [1, 2, 3, 4, 5];
    saveHybridGref(ref);
}

class ArkObject {
    name: string;
    version: number;

    constructor(name: string, version: number) {
        this.name = name;
        this.version = version;
    }
}

export function etsSaveNativeGrefObject(): void {
    let ref:ArkObject = new ArkObject("ArkTS", 1);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefMap(): void {
    let ref = new Map<string, number>();
    ref.set("a", 10);
    ref.set("b", 20);
    saveHybridGref(ref);
}

export function etsSaveNativeGrefSet(): void {
    let ref = new Set<string>();
    ref.add("x");
    ref.add("y");
    saveHybridGref(ref);
}

export function etsSaveNativeGrefObjectFromTS(fn: (x: number) => number): void {
    saveHybridGref(fn);
}
