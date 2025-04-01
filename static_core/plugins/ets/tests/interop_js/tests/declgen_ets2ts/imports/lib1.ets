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

import { Lib2Class, Lib2Class2 /*as Lib2Class2Alias*/, IntAlias } from './lib2';

import { DynClass } from "lib3"

export class C {
    constructor() {
        this.o1 = new Lib2Class();
        this.o2 = new Lib2Class2();
    }
    o1: Lib2Class;
    o2: Lib2Class2;
    v1: IntAlias = 1;
};

export class DynObjWrapper {
    dynObj_: DynClass = new DynClass();
};
