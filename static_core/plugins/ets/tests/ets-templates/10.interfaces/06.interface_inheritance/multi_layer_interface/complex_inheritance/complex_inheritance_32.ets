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
desc: Constant access test under complex inheritance, and access the read-only constant.
tags: [compile-only, negative]
---*/

interface G {
    readonly INTERFACR_CONST: string;
}

interface F extends G {

}

class GrandParent {
    static readonly GRANDPARENT_CONST = "GrandParent Constant";
    static readonly OVERRIDE_CONST = "GrandParent Override Constant";
}

class Parent extends GrandParent {
    static readonly PARENT_CONST = "Parent Constant";
    static readonly OVERRIDE_CONST = "Parent Override Constant";
}

class Child extends Parent implements F {
    static readonly CHILD_CONST = "Child Constant";
    static readonly OVERRIDE_CONST = "Child Override Constant";
    readonly INTERFACR_CONST: string = "GrandParent interface";
}

function main(): void {
    Child.CHILD_CONST = 'child';
    Child.OVERRIDE_CONST = 'override';

    Parent.CHILD_CONST = 'parent';
    Parent.OVERRIDE_CONST = 'override';

    GrandParent.GRANDPARENT_CONST = "grandParent";
    GrandParent.OVERRIDE_CONST = "override";
}