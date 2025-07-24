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

export declare class Foo
{
    bar: number;
    baz: string;
}

export declare let fooInstance: Foo;

export declare class NumberMap {
    [key: number]: number
}
export declare let numberMapInstance: NumberMap;
export declare function functionArg0(): number;
export declare function functionArg1(arg1: number): number;
export declare function functionArg2(arg1: number, arg2: number): number;

export declare class Qux {
    methodArg0(): number;
    methodArg1(arg1: number): number;
    methodArg2(arg1: number, arg2: number): number;
}

export declare let quxInstance: Qux;

export declare class TestNew0
{
    bar: number;
    constructor();
}

export declare class TestNew1 {
    bar: number;

    constructor(_bar: number);
}

export declare class TestNew2 {
    bar: number;

    constructor(lhs: number, rhs: number);
}

export declare class Foobar {
    receivedUndefined: boolean;
    constructor(arg: Object | undefined, arg2: Object);
}