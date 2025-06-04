/**
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

class Operations {
    static voidMethod(a0: int, a1: int): void {
        Operations.count += a0;
        Operations.count += a1;
    }

    static getCount(): int {
        return Operations.count;
    }

    static count: int = 0;
}

class A {
    static funcA(a0: int, a1: int): void {
        A.count += a0;
        A.count += a1;
    }

    static funcA(a0: double, a1: double): double {
        return a0 + a1 as double;
    }

    static funcB(a0: int, a1: int): void {
        return A.funcA(a0, a1);
    }

    static getCount(): int {
        return A.count;
    }

    static count: int = 0;
};
class B extends A{
    static funcB(a0: int, a1: int): void {
        B.count += a1;
        B.count -= a0;
    }

    static getCount(): int {
        return B.count;
    }

    static count: int = 0;
};