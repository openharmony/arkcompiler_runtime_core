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

{% for target in targetList %}

/*---
desc: Single export compilation test
tags: [compile-only]
---*/

export {{target}};

const constInt: int = 42;
const constString: string = 'test string';
const constChar: char = 176;

let varNumber: number = 1.42;
let varString: string = 'Test String 1';
let varBool: boolean = false;
let varArray: Array<number> = new Array<number>(10);
let varBigInt: BigInt = new BigInt(42);

const enum EnumColor { Red, Green, Blue }
let varEnum: EnumColor = EnumColor.Green;


function FuncSum(x: number, y: number): number {
    return x + y;
}

interface SimpleInterface {
    field: string;
    method(): string;
}

class SimpleClass implements SimpleInterface {
    field: string = "field value";
    method(): string {
        return "method result";
    }
}

abstract class SimpleAbstractClass implements SimpleInterface {
    field: string = "field abstract value";
    method (): string {
        return "method abstract result";
    }
}

let varSimpleClass: SimpleInterface = new SimpleClass;

{% endfor %}
