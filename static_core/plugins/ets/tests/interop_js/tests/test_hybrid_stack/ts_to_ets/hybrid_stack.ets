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

import { dyFun1, dyFun2, dyFun3 } from 'testDecl'
import { dyLambdaFun1, dyLambdaFun2 } from 'testDecl'
import { dyST1 } from 'testDecl'

native function testHybridTest(): boolean;
native function testHybridTestLambda(): boolean;
native function testHybridTestESValue(): boolean;
native function testHybridTestSTValue(): boolean;

function staFun0(): void {
    arktest.assertTrue(testHybridTest());
}

function staFun1(): void {
    dyFun1();
}

function staFun2(): void {
    dyFun2();
}

function staFun3(): void {
    dyFun3();
}

function staFun3_3(): void {
    staFun3();
}

let staLambdaFun1: () => void = () => {
    dyLambdaFun1();
}

let staLambdaFun2: () => void = () => {
    dyLambdaFun2();
}

let module = ESValue.load('testDecl');
function staESFun1(): void {
    let dyES1 = module.getProperty('dyES1');
    dyES1.invoke(ESValue.wrap(() => {
        testHybridTestESValue();
    }));
}

function staESFun2(): void {
    let dyES3 = module.getProperty('dyES3');
    dyES3.invoke();
}

let lambdaFun: () => void = () => {
    testHybridTestSTValue();
}
export function staSTfun1(staFun: () => void): void {
    staFun();
}

export function staSTfun2(): void {
    dyST1();
}

function testAll(): void {
    staFun3_3();
    staLambdaFun2();
    staESFun2();
}