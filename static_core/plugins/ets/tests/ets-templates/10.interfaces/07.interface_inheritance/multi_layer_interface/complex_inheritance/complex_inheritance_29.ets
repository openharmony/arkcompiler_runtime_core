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
desc: Under complex inheritance conditions, verify the access of static methods.
---*/

interface G {
    overrideMethod(): string {
        return "Interface override method";
    }
}

class GrandParent implements G {
    static grandParentStaticMethod(): string {
        return "GrandParent static method";
    }

    static overrideMethod(): string {
        return "GrandParent override method";
    }

    overrideMethod(): string {
        return "Interface override method";
    }
}

class Parent extends GrandParent {
    static parentStaticMethod(): string {
        return "Parent static method";
    }

    static overrideMethod(): string {
        return "Parent override method";
    }
}

class Child extends Parent {
    static childStaticMethod(): string {
        return "Child static method";
    }

    static overrideMethod(): string {
        return "Child override method";
    }
}

function main(): void {
    assertEQ(GrandParent.grandParentStaticMethod(), "GrandParent static method");
    assertEQ(GrandParent.overrideMethod(), "GrandParent override method");

    assertEQ(Parent.parentStaticMethod(), "Parent static method");
    assertEQ(Parent.overrideMethod(), "Parent override method");
    assertEQ(Parent.grandParentStaticMethod(), "GrandParent static method");

    assertEQ(Child.childStaticMethod(), "Child static method");
    assertEQ(Child.overrideMethod(), "Child override method");
    assertEQ(Child.parentStaticMethod(), "Parent static method");
    assertEQ(Child.grandParentStaticMethod(), "GrandParent static method");

    assertEQ(Child.overrideMethod(), "Child override method");
    assertEQ(Parent.overrideMethod(), "Parent override method");
    assertEQ(GrandParent.overrideMethod(), "GrandParent override method");
}