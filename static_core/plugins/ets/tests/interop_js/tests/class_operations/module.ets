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

export declare function evaluateNumber(): void;

export declare function evaluateNumber(v0: int, v1: int): int;

export declare function evaluateNumber(v0: string, v1: string): string;

export declare function emptyFunction(): void;

export declare function evaluateObject(obj: ExampleClass): ExampleClass;

export declare function evaluateArray(arr: int[], size: int): int[];

export declare class ExampleClass {
    v0: JSValue;
    v1: JSValue;

    constructor(v0: JSValue, v1: JSValue);

    static evaluateNumber(v2: int, v3: int): int;

    static evaluateNumber(v2: string, v3: string): string;

    static evaluateArray(arr: int[], size: int): int[];

    instanceEvaluateNumber(): JSValue;

    evaluateObject(obj: ExampleClass): ExampleClass;

    getV0(): int[];

    getV1(): int[];

    static emptyMethod(): void;
};

export declare class ClassWithEmptyConstructor {
    v0: JSValue;
    v1: JSValue;

    constructor();

    verifyProperties(v0_expected: JSValue, v1_expected: JSValue): JSValue;

    GetV0(): int[];

    GetV1(): int[];
}

export declare class Kitten {
    constructor(id: int, name: string);
    id: int;
    name: string;
}

export declare class MyNamespace {
    static createKitten(id: int, name: string): Kitten;
}