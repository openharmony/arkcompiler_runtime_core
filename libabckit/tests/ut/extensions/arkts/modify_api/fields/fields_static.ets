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

const moduleField1: string = 'ModuleFieldOne';
const moduleField2: string = 'ModuleFieldTwo';
const moduleField3: number = 5.5;

namespace NS1 {
    export const nS1F1: string[] = ['NamespaceFieldOne'];
}

class C1 {
    public c1F1: string = 'Anno1';
    public c1F2: number = 999;
}

class C2 {
    public c2F1: string = 'Anno1';
}

class C3 {
    public c3F1: number = 999;
}

interface I1 {
    i1F1: string;
}

interface I2 {
    i2F1: number;
}

class C4 implements I1 {
    public i1F1: string = 'ABC';
}

class C5 implements I2 {
    public i2F1: number = 3;
}

enum E1 {
    ONE = 'AAAA',
    TWO = 'BBBB'
}

function enumFieldSetName() {
    console.log(E1.ONE)
}

function interfaceFieldSetType() {
    let inst: C5 = new C5();
    console.log(inst)
}

function interfaceFieldSetName() {
    let inst: C4 = new C4();
    console.log(inst)
}

function classFieldSetName() {
    let class1: C1 = new C1();
    console.log(class1);
}

function moduleFieldSetName() {
    console.log(moduleField1);
}

function moduleFieldSetValue() {
    console.log(moduleField1);
}

function namespaceFieldSetName() {
    console.log(NS1.nS1F1);
}

function printLogStr(a: string) {
    console.log(a);
}

function printLogNum(a: number) {
    console.log(a);
}

function moduleFieldSetType() {
    printLogNum(moduleField3)
}

function classSetType() {
    let class3: C3 = new C3();
    console.log(class3)
}

function classSetValueString() {
    let class2: C2 = new C2();
    console.log(class2.c2F1)
}
