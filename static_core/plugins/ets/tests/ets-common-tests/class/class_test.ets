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

const globalClassName = "class_test.ETSGLOBAL"

namespace Outer {
    export namespace Inner {
        export class C {}

        export function getNamespace(): Class {
            return Class.current()
        }
    }

    export function getNamespace(): Class {
        return Class.current()
    }
}

class Test {
    public static testMethodsFromClass(): void {
        const currentClass = Class.current()
        arktest.assertNE(currentClass, undefined)
        arktest.assertEQ(currentClass.getName(), "class_test.Test")

        const callerClass = Class.ofCaller()
        arktest.assertNE(callerClass, undefined)
        arktest.assertEQ(callerClass!.getName(), globalClassName)
    }

    public static testClassIsNamespace(): void {
        const innerNs = Outer.Inner.getNamespace()
        arktest.assertNE(innerNs, undefined)
        arktest.assertTrue(innerNs!.isNamespace())
        arktest.assertEQ(innerNs!.getName(), "class_test.Outer.Inner")

        const outerNs = Outer.getNamespace()
        arktest.assertNE(outerNs, undefined)
        arktest.assertTrue(outerNs!.isNamespace())
        arktest.assertEQ(outerNs!.getName(), "class_test.Outer")
    }
}

function main(): void {
    // For the test purposes these checks must be located in the entrypoint function
    const currentClass = Class.current()
    arktest.assertNE(currentClass, undefined)
    arktest.assertEQ(currentClass.getName(), globalClassName)

    // `main` is entrypoint, hence it does not have managed callers
    const callerClass = Class.ofCaller()
    arktest.assertEQ(callerClass, undefined)

    Test.testMethodsFromClass()
    Test.testClassIsNamespace()
}
