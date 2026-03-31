/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Overloaded function with float and double parameters: overload resolution for
    floating-point literals. A literal without f suffix resolves to float when the
    value is exactly representable as float; otherwise it resolves to double.
    An f-suffixed literal always resolves to float.
---*/

class Resolver {
    static floatCalls: int = 0;
    static doubleCalls: int = 0;

    static accept(v: float): void {
        Resolver.floatCalls++;
    }

    static accept(v: double): void {
        Resolver.doubleCalls++;
    }
}

function main(): void {
    // Drop counters for JIT-repeats
    Resolver.floatCalls = 0;
    Resolver.doubleCalls = 0;

    // Literals without 'f' suffix whose values are exactly representable as float
    // → resolve to accept(float)
    Resolver.accept(2.0);    // exactly 2 = 1.0 * 2^1
    Resolver.accept(0.5);    // exactly 0.5 = 1.0 * 2^-1
    Resolver.accept(1.5);    // exactly 1.5 = 1.1 binary
    Resolver.accept(4.0);    // exactly 4 = 1.0 * 2^2
    Resolver.accept(0.25);   // exactly 0.25 = 1.0 * 2^-2
    arktest.assertEQ(Resolver.floatCalls, 5);
    arktest.assertEQ(Resolver.doubleCalls, 0);

    // Literals without 'f' suffix whose values are NOT exactly representable as float
    // (double provides more precision) → resolve to accept(double)
    Resolver.accept(0.1);    // 0.1 has infinite binary expansion; float and double differ
    Resolver.accept(0.0709); // not exactly representable in float
    Resolver.accept(1.5e15); // exceeds float's 24-bit significand precision
    arktest.assertEQ(Resolver.doubleCalls, 3);
    arktest.assertEQ(Resolver.floatCalls, 5);  // float count unchanged

    // Float literals with 'f' suffix — always resolve to accept(float)
    Resolver.accept(2.f);
    Resolver.accept(7090e-5f);
    Resolver.accept(2.E-20f);
    arktest.assertEQ(Resolver.floatCalls, 8);
    arktest.assertEQ(Resolver.doubleCalls, 3);  // double count unchanged

    // Explicit double variable — resolves to accept(double)
    let d1: double = 2.0;
    Resolver.accept(d1);
    arktest.assertEQ(Resolver.doubleCalls, 4);
    arktest.assertEQ(Resolver.floatCalls, 8);  // float count unchanged

    // Integer literal in float/double preferred-type context: exercises ETSAnalyzer.cpp:3631 branch 3
    // (preferredType != nullptr && CheckIfLiteralValueIsAppropriate path)
    let fltVar: float = 7090;
    arktest.assertEQ(fltVar, 7090.0f);
    let totalBefore = Resolver.doubleCalls + Resolver.floatCalls;
    Resolver.accept(7090);
    arktest.assertEQ(Resolver.doubleCalls + Resolver.floatCalls, totalBefore + 1);

    // Verify literal values using different syntactic forms for the same value
    let d: double = .0909;
    arktest.assertEQ(d, 9.09e-2);  // .0909 and 9.09e-2 are the same double value

    let f: float = 7090e-5f;
    arktest.assertEQ(f, 0.0709f);  // 7090e-5 and 0.0709 are the same float value
}
