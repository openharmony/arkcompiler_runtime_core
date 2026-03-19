/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Different return type in mA and mC.
    Return types may vary among methods that override each other if the return types are reference types.
    The notion of return-type-substitutability supports covariant returns,
    that is the specialization of the return type to a subtype.
---*/

class A {
    public m(): B {
        return new B;
    }
}

class B {
}

final class D extends B {
}

final class C extends A {
    override m(): D {
        return new D();
    }
}

function main() {
    let b: B = new C().m()
    let a: B = new A().m()
    arktest.assertTrue( b instanceof D );
    arktest.assertFalse( a instanceof D );
}