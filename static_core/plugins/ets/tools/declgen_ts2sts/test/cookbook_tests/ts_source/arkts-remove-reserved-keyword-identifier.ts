/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

export declare const Byte: number;
export declare const Double: number, Object: any, ok: boolean;

export interface A {
    Number: number;
    char: string;
    Int: number;
    foo: string;
    bar(): void;
    BigInt(): bigint;
}

export class B {
    Number: number = 1;
    char: string = "c";
    Int: number = 3;
    foo: string = "foo";
    bar(): void {}
    BigInt(): bigint { return 123n; }
}

export enum C {
    Number = 1,
    Char = 2,
    String = 3,
    XXX = 4
}
