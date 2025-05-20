/*
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

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return 0
    }
    console.println("FAILED: " + name)
    return 1
}

class Base {}

class Derived extends Base {}

function testNoThrow(fn: () => boolean throws): boolean {
    try {
        return fn()
    } catch (ignored) {
        return false
    }
}

function testGetParameterError(lambdaType: LambdaType): boolean {
    try {
        lambdaType.getParameter(1000)
    } catch (ignored: TypeError) {
        // Expected error is catched
        return true
    } catch (other) {
    }
    return false
}

function testLambdaToString(): int {
    let lambda = () => {}; 
    let result = lambda.toLocaleString();
    assertEQ(result,"Cannot get source code of function");
    return 0
}

function testLambdaType(): int {
    let foo: () => Base = () : Base => { return new Base() }
    let foo2: (p: Base) => Derived = (p: Base): Derived => { return new Derived() }

    let baseType = Type.of(new Base()) as ClassType
    let derivedType = Type.of(new Derived()) as ClassType

    let fooType = Type.of(foo) as LambdaType
    let foo2Type = Type.of(foo2) as LambdaType

    return test(fooType.getParametersNum() == 0,                      "   0 Parameters") +
            test(fooType.getResultType().equals(baseType),            "   Base result type") +
            test(foo2Type.getParametersNum() == 1,                    "   1 Parameter") +
            test(foo2Type.getParameter(0).getType().equals(baseType), "       base type param") +
            test(testGetParameterError(foo2Type),                     "   non-existing parameter") +
            test(foo2Type.getResultType().equals(derivedType),        "   Derived result type") +
            test(testNoThrow((): boolean throws => {
                const foo2 = fooType.make()
                return Type.of(foo).equals(Type.of(foo2))
            }),  " Lambda instantiation")
}

function main(): int {
    let failures = 0
    failures += testLambdaType()
    failures += testLambdaToString()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
