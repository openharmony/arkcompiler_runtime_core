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

let callbackEtsFunctionJs: JSValue = 0;

function testCallBackEtsFunctionLambda(): number {
    return callbackEtsFunctionJs((x: number) => x + 1);
}

function testCallBackEtsFunctionLambdaCapture(): number {
    let bias = 1;
    return callbackEtsFunctionJs((x: number) => x + bias);
}

function plusOne(x: number): number {
    return x + 1;
}

function testCallBackEtsFunctionOuter(): number {
    return callbackEtsFunctionJs(plusOne);
}

function sumFunc(...items: number[]) : number {
    let sum: number = 0;
    for (let i: int = 0; i < items.length; i++) {
        sum += items[i];
    }
    return sum;
}

class Sum implements Function {
    unsafeCall(...items: FixedArray<Any>): Any {
        let sum: number = 0;
        for (let i: int = 0; i < items.length; i++) {
            sum += (items[i] as number);
        }
        return sum;
    }
    get name(): string {
        return 'Sum';
    }
}

let sum: Sum = new Sum();

let sumFunction: Function = (...items: number[]) => {
    let sum: number = 0;
    for (let i: int = 0; i < items.length; i++) {
        sum += items[i];
    }
    return sum;
};
