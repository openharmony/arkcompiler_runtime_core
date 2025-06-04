/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function SumInt(value1: int, value2: int): int {
    return value1 + value2;
}

function TaskpoolFuncTestSumInt() {
    let result = await taskpool.execute(SumInt, 10, 20);
    assertEQ(result, 30);
}

function TaskpoolFuncTestAnd() {
    let And = (value1: boolean, value2: boolean): boolean => {
        if (value1 & value2) {
            return true;
        }
        return false;
    }
    let result = await taskpool.execute(And, true, true);
    assertEQ(result, true);
}

function TaskpoolFuncTestStrCat() {
    let StrCat = (value1: string, value2: string): string => {
        return value1 + value2;
    }
    let result = await taskpool.execute(StrCat, "abc", "def");
    assertEQ(result, "abcdef");
}

function TaskpoolFuncTestStrCat2() {
    let StrCat = (value1: string, value2: string): string => {
        return value1 + value2;
    }
    let result = await taskpool.execute(StrCat, "abc", "def");
    result = await taskpool.execute(StrCat, "abc", "def");
    assertEQ(result, "abcdef");
}

function TaskpoolFuncTestStrCat3() {
    let StrCat = (value1: string, value2: string): string => {
        return value1 + value2;
    }
    let Sum = (value1: string, value2: string): string => {
        return value1 + StrCat(value2, "hello");
    }
    let result = await taskpool.execute(Sum, "abc", "def");
    assertEQ(result, "abcdefhello");
}

function TaskpoolFuncTestArraySum() {
    let Sum = (value1: int[], value2: int[]): int[] => {
        let result: int[] = [0, 0];
        result[0] = value1[0] + value2[0];
        result[1] = value1[1] + value2[1];
        return result;
    }
    let result = (await taskpool.execute(Sum, [1, 2], [3, 4])) as int[];
    assertEQ(result[0], 4);
    assertEQ(result[1], 6);
}

class A {
    constructor(a: int, b: int) { this.a = a; this.b = b; }
    constructor() { this(0, 0); }
    a: int;
    b: int;
}

function TaskpoolFuncTestClassFieldsSum() {
    let Sum = (value1: A, value2: A): A => {
        let result = new A();
        result.a = value1.a + value2.a;
        result.b = value1.b + value2.b;
        return result;
    }
    let result = (await taskpool.execute(Sum, new A(1, 2), new A(3, 4))) as A;
    assertEQ(result.a, 4);
    assertEQ(result.b, 6);
}

function TaskpoolFuncTestSumInt2() {
    let result1 = await taskpool.execute(SumInt, 10, 20);
    let result2 = await taskpool.execute(SumInt, 30, 40);
    assertEQ(result1, 30);
    assertEQ(result2, 70);
}

function TaskpoolFuncTestSumInt2SameArguments() {
    let result1 = await taskpool.execute(SumInt, 10, 20);
    let result2 = await taskpool.execute(SumInt, 10, 20);
    assertEQ(result1, 30);
    assertEQ(result2, 30);
}

function TaskpoolFuncTestSumAndMulti() {
    let MultiInt = (value1: int, value2: int): int => {
        return value1 * value2;
    }
    let result1 = await taskpool.execute(SumInt, 10, 20);
    let result2 = await taskpool.execute(MultiInt, 10, 20);
    let result3 = await taskpool.execute(SumInt, 10, 30);
    let result4 = await taskpool.execute(MultiInt, 20, 20);
    assertEQ(result1, 30);
    assertEQ(result2, 200);
    assertEQ(result3, 40);
    assertEQ(result4, 400);
}

function main(): int {
    TaskpoolFuncTestSumInt();
    TaskpoolFuncTestAnd();
    TaskpoolFuncTestStrCat();
    TaskpoolFuncTestStrCat2();
    TaskpoolFuncTestStrCat3();
    TaskpoolFuncTestArraySum();
    TaskpoolFuncTestClassFieldsSum();
    TaskpoolFuncTestSumInt2();
    TaskpoolFuncTestSumInt2SameArguments();
    TaskpoolFuncTestSumAndMulti();
    return 0;
}
