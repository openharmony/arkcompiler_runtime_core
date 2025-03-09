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

package test
enum Colors {Red = 1, Green = 2, Blue = 3}

function CreateTuple(a: int, b: long): [int, long] {
    return [a, b]
}

class A {
    constructor() {
    }

    public int_method(a0: int, a1: int): int
    {
        return a0 + a1;
    }

    public int_override_method(a0: int, a1: int): int
    {
        return a0 * a0;
    }
}

final class B extends A {
    constructor() {
    }
    public override int_override_method(a0: int, a1: int): int
    {
        return a1 * a1;
    }
}

interface I {
    imethod(x: int): int
    imethod_tuple(tuple: [int, long]): int
}

class C implements I {
    constructor() {
    }

    public override imethod(x: int): int
    {
        return x
    }

    public imethod(x: string): int
    {
        return x.length as int
    }

    public override imethod_tuple(tuple: [int, long]): int
    {
        return tuple[0] + tuple[1] as int
    }

    public imethod_optional(z?: string): int
    {
        if (z == undefined) {
            return 5
        }
        return 4
    }

    public method_rest(...numbers: int[]): int
    {
        let res = 0
        for (let n of numbers) {
            res += n
        }
        return res
    }

    public method_union(x: int | string): int
    {
        if (x instanceof string) {
            return x.length as int
        }
        return x
    }
}

abstract class Base {
    public abstract abstract_method(x: int): int
    
    public method(x: int)
    {
        return 2 * x;
    }
}

class Derived extends Base {
    constructor() {
    }
    
    public override abstract_method(x: int): int
    {
        return x;
    }
}

class Overload {
    constructor() {
    }
    
    private method(x: int): int
    {
        return x;
    }
    
    private method(x: int, y: boolean): int
    {
        return 2 * x;
    }
    private method(x: int, y: double): int
    {
        return 3 * x;
    }
}

function CreatePartial(a: int): Partial<UnusualTypes> {
    if (a == 2) {
        return {}
    }
    return {property: a}
}

function CreateRequired(a: int): Required<UnusualTypes> {
    return {property: a, opt_property: a}
}

function CreateReadonly(a: int): Readonly<UnusualTypes> {
    return {property: a}
}

interface UnusualTypes {
    property: int
    opt_property?: int
}

class SpecialTypes {
    public partial_method(i: Partial<UnusualTypes>) : int {
        if (i.property == undefined) {
            return 5
        }
        return i.property as int
    }

    public required_method(i: Required<UnusualTypes>) : int {
        return i.opt_property as int
    }

    public readonly_method(i: Readonly<UnusualTypes>) : int {
        return i.property as int
    }

    public record_method(map: Record<string, int>) : int {
        return map["abab"] as int
    }

    public enum_method(color: Colors): int {
        return color
    }

    public string_literal_method(a: "aaa"): int {
        return a.length as int
    }

    public null_method(a: null): int {
        return 5
    }

    public bigint_method(a: bigint): int {
        if (a.toString() == "2") {
            return 2
        }
        return 1
    }
}

type oneArgLambda = (p: int) => int
type twoArgsLambda = (p1: int, p2:int)  => int
type optArgLambda = (p?: int) => int

//NOTE(ypigunova): include testcase, when rest param in lambdas will support issue #23524
// type restParamLambda = (...numbers: int[]) => int

function LambdaOneArg(): oneArgLambda {
    return (p: int) => { return p }
}

function LambdaTwoArgs(): twoArgsLambda {
    return (p1: int, p2: int) => { return p1 + p2 }
}

function LambdaOptArg(): optArgLambda {
    return (p?: int) => {
        if (p == undefined) {
            return 2
        }
        return 3
    }
}

//NOTE(ypigunova): include testcase, when rest param in lambdas will support. Issue #23524
// function LambdaRestParam(): restParamLambda {
//     return (...numbers: int[]) => {
//         let res = 0
//         for (let n of numbers) {
//             res += n
//         }
//         return res
//     }
// }

class LambdaTypes {
    public one_arg_method(a: oneArgLambda): int {
        return a(5)
    }

    public two_arg_method(a: twoArgsLambda): int {
        return a(1, 2)
    }

    public opt_arg_method(a: optArgLambda): int {
        return a()
    }

//NOTE(ypigunova): include testcase, when rest param will support. Issue #23524
    // public rest_param_method(a: restParamLambda): int {
    // }
}

class Generic0<Type> {
    public method(a: Type): int {
        return 5
    }
}

class Generic1<Type extends Object> extends Generic0<Type> {
    override method(a: Type): int {
        return 2
    }
}
