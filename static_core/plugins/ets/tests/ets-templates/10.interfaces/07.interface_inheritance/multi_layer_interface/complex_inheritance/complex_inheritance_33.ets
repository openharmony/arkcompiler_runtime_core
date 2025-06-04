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
desc: Access static constants normally in complex inheritance relationships.
---*/

interface G {
    readonly INTERFACR_CONST: string;
}

interface F extends G {

}

class GrandParent implements F {
    static readonly GRANDPARENT_CONST = "GrandParent Constant";
    readonly INTERFACR_CONST: string = "GrandParent interface";
}

class Parent extends GrandParent {
    static readonly PARENT_CONST = "Parent Constant";
}

class Child extends Parent {
    static readonly CHILD_CONST = "Child Constant";
}

function main(): void {
    assertEQ(GrandParent.GRANDPARENT_CONST, "GrandParent Constant");
    assertEQ(new GrandParent().INTERFACR_CONST, "GrandParent interface");
    assertEQ(Parent.PARENT_CONST, "Parent Constant");
    assertEQ(Child.CHILD_CONST, "Child Constant");
}