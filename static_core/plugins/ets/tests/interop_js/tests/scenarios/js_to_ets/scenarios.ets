/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
import * as js from "pure_js"
import {
    ClassWithMethodJs, ClassWithGetterSetter, InterfaceWithUnionImpl, UnionTestClassJs,
    ClassWithStaticMethod, standaloneFunctionJs, newInterfaceWithMethod, lambdaFunction,
    genericFunction, ClassToExtend, functionArgTypeAny, functionArgTypeUnknown, functionArgTypeUndefined,
    functionArgTypeTuple, functionReturnTypeAny, functionReturnTypeUnknown, functionReturnTypeUndefined,
    functionArgTypeCallable, functionDefaultParameterFunction, functionDefaultIntParameterFunction,
    functionDefaultStringParameterFunction, functionDefaultFloatParameterFunction, genericTypeParameter,
    functionRestParameter, functionSpreadParameter, functionOverload, genericTypeReturnValue, ClassReturnThis, functionReturnOmittedValue,
    functionArgStringLiteralType, functionIntersectionTypePrimitive, functionCallableReturnValue,
    ClassWithInFieldDeclaration, functionReturnsCompositeType
} from "pure_js"

import {
    optionalCall,
    singleRequired,
} from "decl_js"


function testOptionals() {
    // #22991
    // let x = optionalCall(123, 321, 5)
    // assertTrue(x == 123 + 321 + 5);
    // x = optionalCall(23, 21)
    // assertTrue(x == 23 + 21 + 1);
    // x = optionalCall(100)
    // assertTrue(x == 100 + 130 + 1);
    // x = optionalCall()
    // assertTrue(x == 123 + 130 + 1);

    // #22991
    // x = singleRequired(123, 321)
    // assertTrue(x == 123 + 321 + 123);
    // x = singleRequired(100)
    // assertTrue(x == 100 + 123 + 123);

    return true;
}

function TestStandaloneFunctionCall(): boolean {
    return js.standaloneFunctionJs() as int == 1;
}

function TestClassMethodCall(): boolean {
    let classWithMethodJs = new ClassWithMethodJs();
    return classWithMethodJs.methodInClassJs() as int == 1;
}

function TestInterfaceMethodCall(): boolean {
    let interfaceMethod: () => int = js.newInterfaceWithMethod().methodInInterface as () => int;
    return interfaceMethod() == 1;
}

function TestClassGetter(): boolean {
    return (new ClassWithGetterSetter()).value as int == 1;
}

function TestClassSetter(): boolean {
    let classWithGetterSetter: ClassWithGetterSetter = new ClassWithGetterSetter();
    classWithGetterSetter.value = 2;
    return classWithGetterSetter.value as int == 2;
}

function TestLambdaFunctionCall(): boolean {
    return js.lambdaFunction() as int == 1;
}

function TestGenericFunctionCall(): boolean {
    return js.genericFunction(1) as int == 1;
}

class ClassToExtend1 {
    value(): int {
        return 1;
    }
}

// NOTE(oignatenko) uncomment below after interop is implemented in this direction
// class Child extends ClassToExtend {}
function TestExtendClass(): boolean {
    // return (new ClassToExtend()).value() as int == 1;
    return false;
}

function TestFunctionArgTypeAny(): boolean {
    let arg: JSValue = 1;
    return js.functionArgTypeAny(arg) as int == 1;
}

function TestFunctionArgTypeUnknown(): boolean {
    let arg: JSValue = 1;
    return js.functionArgTypeUnknown(arg) as int == 1;
}

function TestFunctionArgTypeUndefined(): boolean {
    let arg: JSValue = 1;
    return js.functionArgTypeUndefined(arg) as int == 1;
}

function TestFunctionArgTypeTuple(): boolean {
    let arg: [int, String] = [1, "one"];
    return js.functionArgTypeTuple(arg) as int == 1;
}

function TestFunctionReturnTypeAny(): boolean {
    return js.functionReturnTypeAny() as int == 1;
}

function TestFunctionReturnTypeUnknown(): boolean {
    return js.functionReturnTypeUnknown() as int == 1;
}

function TestFunctionReturnTypeUndefined(): boolean {
    return js.functionReturnTypeUndefined() as int == 1;
}

function callable(): int {
    return 1;
}

function TestFunctionArgTypeCallable(): boolean {
    return false;
    // NOTE #1646 extract this to separate ets file
    // NOTE #15891 uncomment below after interop is implemented in this direction
    // return functionArgTypeCallable(callable) as int == 1;
}

function TestDefaultValueIntDefineForParameter(): boolean {
    return js.functionDefaultIntParameterFunction() as int == 1;
}

function TestDefaultValueDefineForParameter(): boolean {
    return js.functionDefaultParameterFunction() as int == 1;
}

function TestGenericTypeAsParameter(): boolean {
    return js.genericTypeParameter(1) as String == "1";
}

function TestGenericTypeAsReturnValue(): boolean {
    return js.genericTypeReturnValue(1) as int == 1;
}

function TestFunctionArgTypeOptionalPrimitiveExplicit(): boolean {
    return js.functionArgTypeOptionalPrimitive(1) as int == 1;
}

function TestFunctionArgTypeOptionalPrimitiveDefault(): boolean {
    return js.functionArgTypeOptionalPrimitive() as int == 1;
}

function TestFunctionArgTypePrimitive(): boolean {
    let arg: int = 1;
    return js.functionArgTypePrimitive(arg) as int == 1;
}

function TestFunctionReturnTypePrimitive(): boolean {
    let res: boolean = js.functionReturnTypePrimitive() as boolean;
    return typeof res === "boolean";
}

function TestRestParams() {
    // NOTE: casts due to bug in frontend
    assertEQ( js.jsSumRestParams(1, 2, 3) as Int, (1 + 2 + 3) );
    assertEQ( js.jsMultiplyArgBySumRestParams(1, 2, 3, 4) as Int, (1) * (2 + 3 + 4));
    assertEQ( js.jsMultiplySumArgsBySumRestParams(1, 2, 3, 4, 5) as Int, (1 + 2) * (3 + 4 + 5));
    assertEQ( js.jsConcatStringsRestParams() as String, "");
    assertEQ( js.jsConcatStringsRestParams("a", "b", "c") as String, "abc");
    return true;
}

function TestFunctionArgTypeUnion(): boolean {
    let i: (number | string) = 1;
    let s: (number | string) = "1";
    if (js.functionArgTypeUnion(i) as int != 0) {
        return false;
    }
    if (js.functionArgTypeUnion(s) as int != 1) {
        return false;
    }
    return true;
}

function TestFunctionReturnTypeUnion(): boolean {
    let res: (number | string) = js.functionReturnTypeUnion(0) as (number | string);
    if (typeof res != "number" || res as number != 1) {
        return false;
    }
    res = js.functionReturnTypeUnion(1) as (number | string);
    if (typeof res != "string" || res as string != "1") {
        return false;
    }
    return true;
}

function TestClassMethodArgTypeUnion(): boolean {
    let i: (number | string) = 1;
    let s: (number | string) = "1";
    let obj = new UnionTestClassJs();
    if (obj.methodArgTypeUnion(i) as int != 0) {
        return false;
    }
    if (obj.methodArgTypeUnion(s) as int != 1) {
        return false;
    }
    return true;
}

function TestClassMethodReturnTypeUnion(): boolean {
    let obj = new UnionTestClassJs();
    let res: (number | string) = obj.methodReturnTypeUnion(0) as (number | string);
    if (typeof res != "number" || res as number != 1) {
        return false;
    }
    res = obj.methodReturnTypeUnion(1) as (number | string);
    if (typeof res != "string" || res as string != "1") {
        return false;
    }
    return true;
}

function TestInterfaceMethodCallReturnTypeUnion(): boolean {
    let obj = js.newInterfaceWithUnion();
    let res: (number | string) = obj.methodInInterface(0) as (number | string);
    if (typeof res != "number" || res as number != 1) {
        return false;
    }
    res = obj.methodInInterface(1) as (number | string);
    if (typeof res != "string" || res as string != "1") {
        return false;
    }
    return true;
}

function TestStaticMethodCall(): boolean {
    return js.ClassWithStaticMethod.staticMethod(1) as int == 1;
}

function TestStaticMethodCallReturnTypeUnion(): boolean {
    let res: (number | string) = js.ClassWithStaticMethod.staticMethodReturnTypeUnion(0) as (number | string);
    if (typeof res != "number" || res as number != 1) {
        return false;
    }
    res = js.ClassWithStaticMethod.staticMethodReturnTypeUnion(1) as (number | string);
    if (typeof res != "string" || res as string != "1") {
        return false;
    }
    return true;
}

function testReturnIsThis(): boolean {
    let obj = new ClassReturnThis();
    return obj.returnThis() as ClassReturnThis == obj;
}

function testReturnIsOmitted(): boolean {
    return functionReturnOmittedValue() as Object | undefined == undefined;
}

function testFunctionRestParameter(): boolean {
    return functionRestParameter(1, 2, 3) as int == 1;
}

function Test_function_spread_parameter(): boolean {
    // NOTE(nikitayegorov, DZ) uncomment 2 lines below after rest\spread is fixed (#17339)
    // const arr = [1, 1];
    // return functionSpreadParameter(...arr) as int == 2;
    return false;
}

function testFunctionOverload(): boolean {
    return functionOverload() as int == 1;
}

function testFunctionCallableReturnValue(): boolean {
    const fn = functionCallableReturnValue;
    return fn()(1) as int == 2;
}

function testFunctionArgStringLiteralType(): boolean {
    let arg: JSValue = 1;
    return functionArgStringLiteralType(arg) as int == 1;
}

function testFunctionIntersectionTypePrimitive(): boolean {
    let arg: JSValue = 1;
    return functionIntersectionTypePrimitive(arg) as int == 1;
}

function testClassInPlaceFieldDeclarations(): boolean {
    let obj = new ClassWithInFieldDeclaration(1);
    return obj.value as int == 1;
}

function testFunctionReturnsCompositeTypeInt(): boolean {
    let isOk = true;
    for (let i = 0; i < 4; i++){
        const expectedValue = functionReturnsCompositeType(i) as (string | number | double | string[]);
        switch (i) {
            case 0:
                isOk = isOk && expectedValue as string == '1';
                break;
            case 1:
                isOk = isOk &&  expectedValue as double == 1.0;
                break;
            case 2:
                isOk = isOk &&  expectedValue as number == 1;
                break;
            default:
                isOk &= typeof expectedValue == 'object';
                break;
        }
    }
    return isOk;
}
