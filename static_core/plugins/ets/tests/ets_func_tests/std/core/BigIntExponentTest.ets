/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*
 * Tests for the fixed BigInt(string) exponent parsing behavior.
 *
 * Background: the constructor's exponent substring was parsed by BigInt.parseInt, which
 * accumulates `value = value * 10 + digit` into a signed `int` with no overflow check. For an
 * adversarial input like '0e2147483647' (mantissa '0' passes the earlier 'rest is zeros' guard),
 * the exponent silently overflowed and/or drove the unbounded `for (i < shift) digits.push(0)`
 * allocation loop until the process was OOM-killed (exit 137). The fix added BigInt.parseExponent,
 * which accumulates into a `long` and rejects any exponent whose resulting magnitude would exceed
 * baseSys - 1 bits -- the same limit operatorLeftShift enforces -- with a catchable
 * 'BigInt: too many digits in integer' error. These tests verify the fix rejects adversarial
 * inputs and does not over-reject legitimate ones.
 */

// ----------------------------------------------------------------------------
// Primary: adversarial exponent is rejected with a catchable error (no OOM-kill)
// ----------------------------------------------------------------------------

function testExponentOverflowThrows(): void {
    // The mantissa MUST be '0' to reach the buggy path: a non-zero mantissa (e.g. '1e2147483647')
    // is rejected earlier by the 'rest is zeros for negative shift' guard, so the exponent parser
    // is the only line of defense for this input.
    arktest.expectThrow(
        (): void => { let _ = new BigInt('0e2147483647'); },
        (e: Error): boolean | string => {
            return e.message.indexOf('too many digits in integer') >= 0
                ? true
                : 'error message should mention "too many digits in integer", got: ' + e.message;
        });
}

// ----------------------------------------------------------------------------
// Threshold boundary: legitimate exponents parse, over-threshold rejects, negative-exponent
// guard intact (the fix did not over-reject or break the existing validation path)
// ----------------------------------------------------------------------------

function testExponentThresholdBoundary(): void {
    // (a) A valid exponent produces the correct value: 1e6 == 1_000_000.
    let oneE6 = new BigInt('1e6');
    arktest.assertTrue(oneE6.equals(new BigInt('1000000')),
        'new BigInt("1e6") == 1000000');

    // (b) A larger valid exponent still fits: 1e10 == 10_000_000_000.
    let oneE10 = new BigInt('1e10');
    arktest.assertTrue(oneE10.equals(new BigInt('10000000000')),
        'new BigInt("1e10") == 10000000000');

    // (c) Mantissa '0' with an exponent just under the threshold: 0e6 == 0 (not rejected; the
    //     threshold only fires for unrepresentable magnitudes, and 0 stays 0 regardless of shift).
    let zeroE6 = new BigInt('0e6');
    arktest.assertTrue(zeroE6.equals(new BigInt(0)),
        'new BigInt("0e6") == 0');

    // (d) The threshold must reject a second adversarial exponent, not just the original one.
    //     '0e9999999999' (10 digits) far exceeds baseSys-1 bits, so it must throw the same error.
    arktest.expectThrow(
        (): void => { let _ = new BigInt('0e9999999999'); },
        (e: Error): boolean | string => {
            return e.message.indexOf('too many digits in integer') >= 0
                ? true
                : 'second adversarial exponent uses the same error message, got: ' + e.message;
        });

    // (e) Negative exponent on a non-zero mantissa is 'not an integer' -- the EXISTING guard at
    //     BigInt.ets handles it, NOT the new parseExponent threshold. Verifying this confirms the
    //     fix did not alter that path.
    arktest.expectThrow(
        (): void => { let _ = new BigInt('1e-3'); },
        (e: Error): boolean | string => {
            return e.message.indexOf('not an integer') >= 0
                ? true
                : 'negative-exponent non-integer uses the existing guard, got: ' + e.message;
        });
}

// ----------------------------------------------------------------------------

function main(): int {
    const suite = new arktest.ArkTestsuite('BigInt exponent parsing tests');
    suite.addTest('adversarial exponent overflow throws', testExponentOverflowThrows);
    suite.addTest('exponent threshold boundary', testExponentThresholdBoundary);
    return suite.run();
}
