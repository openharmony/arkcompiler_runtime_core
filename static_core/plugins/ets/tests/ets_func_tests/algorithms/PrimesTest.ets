/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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


function cmp(x: FixedArray<long>, y: FixedArray<long>): boolean {
    if (x.length != y.length) {
        return false;
    }
    for (let i: int = 0; i < x.length; i++) {
        if (x[i] != y[i]) {
            return false;
        }
    }
    return true;
}

function primeFactorsTrivial(n0: long): FixedArray<long> {
    let primes: FixedArray<long> = []
    let factor: long = 2
    let n = n0;
    while (n > 1 && factor < n0) {
        if (n % factor == 0) {
            primes = concat(primes, [factor]);
            n = (n / factor).toLong();
        } else {
            factor++;
        }
    }
    return primes;
}

function primeFactorsTrivialPlus(n0: long): FixedArray<long> {
    let primes: FixedArray<long> = [];
    let factor: long = 3;
    let n: long = n0;
    while (n % 2 == 0) {
        primes = concat(primes, [2 as long]);
        n = (n / 2).toLong();
    }
    while (factor * factor <= n) {
        if (n % factor == 0) {
            primes = concat(primes, [factor]);
            n = (n / factor).toLong();
        } else {
            factor += 2;
        }
    }
    if (n != 1 && n < n0) {
        primes = concat(primes, [n]);
    }
    return primes;
}

function sievePrimes(n: int): FixedArray<long> {
    let isPrime: FixedArray<boolean> = new FixedArray<boolean>(n + 1);
    let primes: FixedArray<long> = [];
    fill(isPrime, true);
    // 0 and 1 are not primes
    for (let i = 2; i < n + 1; i++) {
        if (isPrime[i]) {
            for (let j = 2; i * j < n + 1; j++) {
                isPrime[i * j] = false;
            }
        }
    }
    for (let i = 2; i < n + 1; i++) {
        if (isPrime[i]) {
            primes = concat(primes, [i.toLong()]);
        }
    }
    return primes;
}

function test(result: boolean, name: String): int {
    if (result) {
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.println('PASSED: ' + name);
        return 0;
    }
    // CC-OFFNXT(G.AOD.03) current test suite code style
    console.println('FAILED: ' + name);
    return 1;
}

function main(): int {
    let failures: int = 0;
    const empty: FixedArray<long> = [];
    failures += test(
        cmp(primeFactorsTrivial(1234567), [127 as long, 9721]),
        'Trivial factorisation of 1234567');
    failures += test(
        cmp(primeFactorsTrivial(7), empty),
        'Trivial factorisation of 7');
    failures += test(
        cmp(primeFactorsTrivialPlus(1234567), [127 as long, 9721]),
        'Improved factorisation of 1234567');
    failures += test(
        cmp(primeFactorsTrivialPlus(7), empty),
        'Improved factorisation of 7');
    failures += test(
        cmp(sievePrimes(15), [2 as long, 3, 5, 7, 11, 13]),
        'Prime numbers under 15');
    return failures;
}
