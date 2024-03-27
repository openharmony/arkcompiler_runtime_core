/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
import { standaloneFunctionJs, ClassWithMethodJs, newInterfaceWithMethod, ClassWithGetterSetter, lambdaFunction,
    genericFunction, ClassToExtend, functionArgTypeAny, functionArgTypeUnknown, functionArgTypeUndefined,
    functionArgTypeTuple, functionReturnTypeAny, functionReturnTypeUnknown, functionReturnTypeUndefined,
    functionArgTypeCallable, functionDefaultParameterFunction, functionDefaultIntParameterFunction,
    functionDefaultStringParameterFunction, functionDefaultFloatParameterFunction, genericTypeParameter,
    genericTypeReturnValue, functionArgTypeOptionalPrimitive, functionArgTypePrimitive,
    functionReturnTypePrimitive } from "pure_js"

import {
    optional_call,
    single_required,
} from "decl_js"

function testOptionals() {
    let x = optional_call(123, 321, 5)
    assert(x == 123 + 321 + 5);
    x = optional_call(100)
    assert(x == 100 + 130 + 1);
    x = optional_call()
    assert(x == 123 + 130 + 1);

    x = single_required(123, 321)
    assert(x == 123 + 321 + 123);
    x = single_required(100)
    assert(x == 100 + 123 + 123);
    return true;
}

function Test_standalone_function_call(): boolean {
    return standaloneFunctionJs() as int == 1;
}

function Test_class_method_call(): boolean {
    let classWithMethodJs = new ClassWithMethodJs();
    return classWithMethodJs.methodInClassJs() as int == 1;
}

function Test_interface_method_call(): boolean {
    let interface_method: () => int = newInterfaceWithMethod().methodInInterface as () => int;
    return interface_method() == 1;
}

function Test_class_getter(): boolean {
    return (new ClassWithGetterSetter()).value as int == 1;
}

function Test_class_setter(): boolean {
    let classWithGetterSetter: ClassWithGetterSetter = new ClassWithGetterSetter();
    classWithGetterSetter.value = 2;
    return classWithGetterSetter.value as int == 2;
}

function Test_lambda_function_call(): boolean {
    return lambdaFunction() as int == 1;
}

function Test_generic_function_call(): boolean {
    return genericFunction(1) as int == 1;
}

class ClassToExtend1 {
    value(): int {
        return 1;
    }
}

// NOTE(oignatenko) uncomment below after interop is implemented in this direction
// class Child extends ClassToExtend {}
function Test_extend_class(): boolean {
    // return (new ClassToExtend()).value() as int == 1;
    return false;
}

function Test_function_arg_type_any(): boolean {
    let arg: JSValue = 1;
    return functionArgTypeAny(arg) as int == 1;
}

function Test_function_arg_type_unknown(): boolean {
    let arg: JSValue = 1;
    return functionArgTypeUnknown(arg) as int == 1;
}

function Test_function_arg_type_undefined(): boolean {
    let arg: JSValue = 1;
    return functionArgTypeUndefined(arg) as int == 1;
}

function Test_function_arg_type_tuple(): boolean {
    let arg: [int, String] = [1, "one"];
    return functionArgTypeTuple(arg) as int == 1;
}

function Test_function_return_type_any(): boolean {
    return functionReturnTypeAny() as int == 1;
}

function Test_function_return_type_unknown(): boolean {
    return functionReturnTypeUnknown() as int == 1;
}

function Test_function_return_type_undefined(): boolean {
    return functionReturnTypeUndefined() as int == 1;
}

function callable(): int {
    return 1;
}

function Test_function_arg_type_callable(): boolean {
    return false;
    // NOTE #1646 extract this to separate ets file
    // NOTE #15891 uncomment below after interop is implemented in this direction
    // return functionArgTypeCallable(callable) as int == 1;
}

function Test_default_value_int_define_for_parameter(): boolean {
    return functionDefaultIntParameterFunction() as int == 1;
}

function Test_default_value_define_for_parameter(): boolean {
    return functionDefaultParameterFunction() as int == 1;
}

function Test_generic_type_as_parameter(): boolean {
    return genericTypeParameter(1) as String == "1";
}

function Test_generic_type_as_return_value(): boolean {
    return genericTypeReturnValue(1) as int == 1;
}

function Test_function_arg_type_optional_primitive_explicit(): boolean {
    return functionArgTypeOptionalPrimitive(1) as int == 1;
}

function Test_function_arg_type_optional_primitive_default(): boolean {
    return functionArgTypeOptionalPrimitive() as int == 1;
}

function Test_function_arg_type_primitive(): boolean {
    let arg: int = 1;
    return functionArgTypePrimitive(arg) as int == 1;
}

function Test_function_return_type_primitive(): boolean {
    let res: boolean = functionReturnTypePrimitive() as boolean;
    return typeof res === "boolean";
}
