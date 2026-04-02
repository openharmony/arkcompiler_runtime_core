/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: >-
    The method with the same name has the different parameters parameter type
    but same return types
---*/

interface Parent1 {
    combine(a: string, b: number): string;
}

interface Parent2 {
    combine(b: number, a: string): string;
}

// Parent1 and Parent2 combine methods with different parameter orders
interface Child extends Parent1, Parent2 {}

class ChildClass implements Child {
    //one method to override both interface methods
    combine(p: Object, s: Object): string {
        if (p instanceof string && s instanceof number)
            return "str1"
        else if (p instanceof number && s instanceof string )
            return "str2"
        
        return 'undefined'
    }
}

function main() {
    let c: ChildClass = new ChildClass()
    arktest.assertEQ(c.combine("str", 1.0),"str1")
    arktest.assertEQ(c.combine(2.1, "str"),"str2")
}
