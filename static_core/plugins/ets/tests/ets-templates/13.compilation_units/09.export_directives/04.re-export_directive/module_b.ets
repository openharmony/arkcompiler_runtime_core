/*
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

/*---
tags: [not-a-test, compile-only]
---*/

export function bar(): int {
    return 100;
}

export class foo {
    foo_bar() : int {
        return 101;
    }
    static foo_bar_s() : int {
        return 102;
    }
}

export interface foo_Inter {
    fld1 : int;
    fld2 : string;
}

export enum Color {
    Red = 10,
    Blue = 20
}

export namespace NS {
    export enum Color {
        Red = 5,
        Blue = 10
    }
}

const numExported : int = 111;

export numExported;

@Retention("SOURCE")
export @interface MyAnno {}

type fooCls = foo;
export fooCls;