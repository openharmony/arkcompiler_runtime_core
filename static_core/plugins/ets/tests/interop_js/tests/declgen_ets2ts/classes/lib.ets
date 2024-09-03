/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

export interface I0 {
    I0Method(a: string): string { return a; };
}

export interface I1 {
    I1Method(a: double): double { return a; };
}

export class Base {
    a: double = 1;
    constructor(a: double) {this.a = a;}
}

export class Derived extends Base implements I0, I1 {
    I0Method(a: string): string { return a; };
    I1Method(a: double): double { return a; };
    b: double = 2;
    constructor(a: double, b: double) {super(a); this.b = b;}
}
