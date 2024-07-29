#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from pytest import fixture, mark, TempPathFactory
from typing_extensions import LiteralString

from arkdb.compiler import CompileError, Compiler, Options, StringCodeCompiler
from arkdb.mirrors.base import arkts_str
from arkdb.runnable_module import ScriptFile
from arkdb.walker import BreakpointWalkerType

BEFORE_EVAL_MESSAGE: LiteralString = "Before eval : "
EVAL_MESSAGE: LiteralString = "From eval : "
AFTER_EVAL_MESSAGE: LiteralString = "After eval : "

pytestmark = mark.evaluate


async def test_evaluate_no_args(
    breakpoint_walker: BreakpointWalkerType,
):
    code = f"""
function main() {{
    console.log('{BEFORE_EVAL_MESSAGE}');
    undefined;  // #BP
    console.log('{AFTER_EVAL_MESSAGE}');
    return 0;   // #BP
}}
"""
    expression = f"console.log('{EVAL_MESSAGE}');"

    async with breakpoint_walker(code) as walker:
        eval_point = await anext(walker)
        assert walker.capture.stdout[-1] == arkts_str(BEFORE_EVAL_MESSAGE)

        result = await eval_point.evaluate(expression, [walker.script_file.panda_file])
        assert len(result) == 2
        # Must execute without exceptions.
        assert result[1] is None
        # Must be void result.
        assert result[0].type_ == "undefined"
        assert walker.capture.stdout[-1] == arkts_str(EVAL_MESSAGE)

        # Run after evaluation point - execution must be finished correctly.
        await anext(walker)
        assert walker.capture.stdout[-1] == arkts_str(AFTER_EVAL_MESSAGE)


async def test_evaluate_local_primitive(
    breakpoint_walker: BreakpointWalkerType,
):
    code = f"""
function main() {{
    let localInt: int = 1;
    console.log('{BEFORE_EVAL_MESSAGE}' + localInt);
    undefined;  // #BP
    console.log('{AFTER_EVAL_MESSAGE}' + localInt);
    return 0;   // #BP
}}
"""
    expression = f"console.log('{EVAL_MESSAGE}' + localInt); localInt += 2;"

    async with breakpoint_walker(code) as walker:
        eval_point = await anext(walker)
        expected_number = 1
        assert walker.capture.stdout[-1] == arkts_str(f"{BEFORE_EVAL_MESSAGE}{expected_number}")

        result = await eval_point.evaluate(expression, [walker.script_file.panda_file])
        assert len(result) == 2
        # Must execute without exceptions.
        assert result[1] is None
        # Last statement consists of primitive value, which will be returned.
        assert result[0].type_ == "number"
        assert walker.capture.stdout[-1] == arkts_str(f"{EVAL_MESSAGE}{expected_number}")
        expected_number += 2

        # Run after evaluation point - execution must be finished correctly.
        await anext(walker)
        assert walker.capture.stdout[-1] == arkts_str(f"{AFTER_EVAL_MESSAGE}{expected_number}")


async def test_evaluate_local_string(
    breakpoint_walker: BreakpointWalkerType,
):
    code = f"""
function main() {{
    let localString: String = 'Hello';
    console.log('{BEFORE_EVAL_MESSAGE}' + localString);
    undefined;  // #BP
    console.log('{AFTER_EVAL_MESSAGE}' + localString);
    return 0;   // #BP
}}
"""
    expression = f"console.log('{EVAL_MESSAGE}' + localString); localString += \", it's me\";"

    async with breakpoint_walker(code) as walker:
        eval_point = await anext(walker)
        expected_string = "Hello"
        assert walker.capture.stdout[-1] == arkts_str(f"{BEFORE_EVAL_MESSAGE}{expected_string}")

        result = await eval_point.evaluate(expression, [walker.script_file.panda_file])
        assert len(result) == 2
        # Must execute without exceptions.
        assert result[1] is None
        # Must be void result.
        assert result[0].type_ == "undefined"
        assert walker.capture.stdout[-1] == arkts_str(f"{EVAL_MESSAGE}{expected_string}")
        expected_string += ", it's me"

        # Run after evaluation point - execution must be finished correctly.
        await anext(walker)
        assert walker.capture.stdout[-1] == arkts_str(f"{AFTER_EVAL_MESSAGE}{expected_string}")


@fixture(scope="session")
def session_code_compiler(
    tmp_path_factory: TempPathFactory,
    ark_compiler_default_options: Options,
) -> StringCodeCompiler:
    """
    Return :class:`StringCodeCompiler` instance that can compile ``sts``-file.
    """
    return StringCodeCompiler(
        tmp_path=tmp_path_factory.mktemp("./expressions", numbered=True),
        ark_compiler=Compiler(ark_compiler_default_options),
    )


@fixture(scope="session")
def expression_evaluation_base(
    session_code_compiler: StringCodeCompiler,
) -> ScriptFile:
    base_code = """\
let globalInt: int = 2;
class BaseClass {
    public baseMethod() { console.log("Hello"); }
}
class UserClass extends BaseClass {
    public static staticInt: int = 1;
    public n: number = 1.0;

    public static staticMethod() { console.log(UserClass.staticInt); }
    public method() { console.log(this.n); }
}
function globalFoo(n: number) { console.log(n); }
function main() {
    let localInt: int = 1;
    console.log(localInt);
    {
        let intInScope: int = 1;
        console.log(intInScope);
    }
    UserClass.staticMethod();
    console.log(UserClass.staticInt);
    let userClass = new UserClass();
    userClass.baseMethod();
    userClass.method();
    console.log(userClass.n);
    globalFoo(1.5);
    undefined;  // #BP
}
"""
    # Will compile once for all test cases.
    return session_code_compiler.compile(base_code, name="failed_expressions_base_file")


@mark.parametrize(
    "expression",
    [
        "console.log(localNumber); localNumber += 1.5;",  # No local variable with this name
        "localInt += ' world';",  # Incorrect types
        "console.log(globalNumber); globalNumber += 1.5;",  # No global variable with this name
        "globalInt += ' world';",  # Incorrect types
        "console.log(intInScope); intInScope += 2;",  # Variable cannot be accessed in current scope
        "globalBar(1.5);",  # No function with this name
        "globalFoo('1.5');",  # No function matching the signature
        "const klass = new Klass(); console.log(klass.n);",  # No class with this name
        "const o = new UserClass('a'); console.log(o.n);",  # No constructor matching the signature
        "console.log(UserClass.staticNumber);",  # No class static variable with this name
        "console.log(userClass.staticInt);",  # Cannot access static variable through instance
        "userClass.m();",  # No method with this name
        "userClass.method(1);",  # No method matching the signature
        "userClass.method += 1;",  # Cannot use method as value
        "UserClass.staticM();",  # No static method with this name
        "UserClass.staticMethod(1);",  # No static method matching the signature
        "userClass.staticMethod();",  # Cannot call static method through instance
        "UserClass.staticMethod += 1;",  # Cannot use static method as value
        "const o = new BaseClass(); o.method();",  # No such method in the base class
    ],
)
@mark.xfail(reason="Must not be compiled in evaluation mode", raises=CompileError)
async def test_compilation_error(
    breakpoint_walker: BreakpointWalkerType,
    expression_evaluation_base: ScriptFile,
    expression: str,
):
    """
    This test checks that all incorrect expressions fail compilation.
    """
    async with breakpoint_walker(expression_evaluation_base) as walker:
        eval_point = await anext(walker)
        await eval_point.evaluate(expression, [walker.script_file.panda_file], allow_compiler_failure=True)
