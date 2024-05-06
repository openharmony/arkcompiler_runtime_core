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

package scenarios_test

const STRING_VALUE = '1';
const INT_VALUE = 1;
const INT_VALUE2 = 2;
const INT_VALUE3 = 3;
const FLOAT_VALUE = 1.0;

function standalone_function_ets(): int {
    return 1;
}

class ClassWithMethodEts {
    public methodInClass(): int {
        return 1;
    }
};

interface InterfaceWithMethodEts {
    methodInInterface(): int;
}

class ImplInterfaceWithMethodEts implements InterfaceWithMethodEts {
    public override methodInInterface(): int {
        return 1;
    }
}

// NOTE (oignatenko) figure a way (and figure if there is a need) to make a negative test for static methods
// of interface since these do not belong to subset.
function newInterfaceWithMethodEts(): InterfaceWithMethodEts {
    return new ImplInterfaceWithMethodEts();
}

class ClassWithGetterSetterEts {
    private _value: int = 1;

    get value() {
        return this._value; // NOTE figure why this._value fails to run here
    }

    set value(theValue: int) {
        this._value = theValue;
    }
}

let lambda_function_ets: () => int = () => {
  return 1;
}

function get_lambda_function_ets(): () => int {
    return lambda_function_ets;
}

function generic_function_ets<T>(arg: T): T {
    return arg;
}

class ClassToExtendEts {
    value(): int {
        return 1;
    }
}

function function_arg_type_any_ets(arg: JSValue): JSValue {
    return arg;
}

function function_arg_type_unknown_ets(arg: JSValue): JSValue {
    return arg;
}

function function_arg_type_undefined_ets(arg: JSValue): JSValue {
    return arg;
}

function function_arg_type_tuple_ets(arg: [number, string]): number {
    return arg[0];
}

function function_arg_type_callable_ets(function_to_call: () => JSValue): JSValue {
    return function_to_call();
}

function generic_type_parameter_ets<T>(arg: T): String {
    return arg!.toString();
}

function generic_type_return_value_ets<T>(arg: int, value: T): T {
   let arr: T[] = [value, value, value];
   return arr[arg];
}

class ClassWithDefaultParameterMethods {
    public int_parameter_method(arg: int = 1): int {
        return arg;
    }
    
    public static int_parameter_static_method(arg: int = 1): int {
        return arg;
    }
    
    public undefined_parameter_method(arg: JSValue|undefined = undefined): JSValue|undefined {
        return arg;
    }
    
    public static undefined_parameter_static_method(arg: JSValue|undefined = undefined): JSValue|undefined {
        return arg;
    }

    public overloaded_method(arg: int) {
        // do nothing
    }

    public overloaded_method(arg: string) {
        // do nothing
    }

    public overloaded_static_method(arg: int) {
        // do nothing
    }

    public overloaded_static_method(arg: string) {
        // do nothing
    }
}

class DerivedClassWithDefaultParameterMethods extends ClassWithDefaultParameterMethods {
    public int_parameter_method(arg: int): int {
        return -arg;
    }
}

function overloaded_function(arg: int) {
    // do nothing
}

function overloaded_function(arg: string) {
    // do nothing
}

function default_int_parameter_function(arg: int = 1): int {
    return arg;
}

function default_undefined_parameter_function(arg: JSValue|undefined = undefined): JSValue|undefined {
    return arg;
}

function function_arg_type_optional_primitive(arg?: Number): Number | undefined {
    return arg;
}

function function_arg_type_primitive(arg: number): number {
    return arg;
}

function function_return_type_primitive(): number {
    return 1;
}

function GCJSRuntimeCleanup(): void {
    try {
        // trigger FinalizationRegistry cleanup
        let full_gc_id = GC.startGC(GC.FULL_CAUSE);
        GC.waitForFinishGC(full_gc_id);
    } catch (e) {
        assert false : "Unexpected exception during GC";
    }
}
