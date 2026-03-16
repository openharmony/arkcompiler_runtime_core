/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;

let STValue = etsVm.STValue;
let SType = etsVm.SType;

let testAll = etsVm.getFunction('Lhybrid_stack/ETSGLOBAL;', 'testAll');
let staFun1 = etsVm.getFunction('Lhybrid_stack/ETSGLOBAL;', 'staFun1');
let staFun2 = etsVm.getFunction('Lhybrid_stack/ETSGLOBAL;', 'staFun2');
let staFun0 = etsVm.getFunction('Lhybrid_stack/ETSGLOBAL;', 'staFun0');
let staLambdaFun1 = etsVm.getClass('Lhybrid_stack/ETSGLOBAL;').staLambdaFun1;
let testHybridTestLambda = etsVm.getClass('Lhybrid_stack/ETSGLOBAL;').testHybridTestLambda;
let staESFun1 = etsVm.getFunction('Lhybrid_stack/ETSGLOBAL;', 'staESFun1');

export function dyFun1(): void {
    staFun0();
}
export function dyFun2_1(): void {
    staFun1();
}
export function dyFun2_2(): void {
    dyFun2_1();
}
export function dyFun2(): void {
    dyFun2_2();
}
export function dyFun3_3(): void {
    staFun2();
}
export function dyFun3(): void {
    dyFun3_3();
}

export let dyLambdaFun1: () => void = () => {
    testHybridTestLambda();
}

export let dyLambdaFun2: () => void = () => {
    staLambdaFun1();
}

export function dyES1(lambFun: () => void): void {
    lambFun();
}

export function dyES2(): void {
    staESFun1();
}

export function dyES3(): void {
    dyES2();
}

let module = STValue.findClass('hybrid_stack.ETSGLOBAL');
export function dyST1(): void {
    let staFun = module.namespaceGetVariable('lambdaFun', SType.REFERENCE);
    module.namespaceInvokeFunction('staSTfun1', 'C{std.core.Function0}:', [staFun]);
}

function dyST2(): void {
    module.namespaceInvokeFunction('staSTfun2', ':', []);
}

function main(): void {
    testAll();
    dyST2()
}

main();
