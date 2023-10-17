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

export declare function EvaluateNumber(): void;

export declare function EvaluateNumber(v0: int, v1: int): int;

export declare function EvaluateNumber(v0: string, v1: string): string;

export declare function EmptyFunction(): void;

export declare function EvaluateObject(obj: ExampleClass): ExampleClass;

export declare function EvaluateArray(arr: int[], size: int): int[];

export declare class ExampleClass {
    v0: JSValue;
    v1: JSValue;

    constructor(v0: JSValue, v1: JSValue);

    static EvaluateNumber(v2: int, v3: int): int;

    static EvaluateNumber(v2: string, v3: string): string;

    static EvaluateArray(arr: int[], size: int): int[];

    InstanceEvaluateNumber(): JSValue;

    EvaluateObject(obj: ExampleClass): ExampleClass;

    GetV0(): int[];

    GetV1(): int[];

    static EmptyMethod(): void;
};

export declare class ClassWithEmptyConstructor {
    v0: JSValue;
    v1: JSValue;

    constructor();

    varify_properties(v0_expected: JSValue, v1_expected: JSValue): JSValue;

    GetV0(): int[];

    GetV1(): int[];
}

// this is TS namespace
// export namespace MyNamespace {
//     class Kitten {
//         constructor(id: number, name: string) {
//             this.id = id;
//             this.name = name;
//         }

//         id: number;
//         name: string;
//     }

//     export function create_kitten(id: number, name: string) {
//         return new Kitten(id, name);
//     }
// }


// TODO: support namespaces in front-end

export declare class Kitten {
    constructor(id: int, name: string);
    id: int;
    name: string;
}

export declare class MyNamespace {
    static create_kitten(id: int, name: string): Kitten;
}