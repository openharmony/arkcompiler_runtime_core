/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class AccSumCount {
  public sum: number;
  public count: number;
  public constructor(sum: number, count: number) {
    this.sum = sum;
    this.count = count;
  }
}

class AccProdSteps {
  public prod: number;
  public steps: number;
  public constructor(prod: number, steps: number) {
    this.prod = prod;
    this.steps = steps;
  }
}

function main(): int {
  const suite = new arktest.ArkTestsuite('Uint16ArrayReduce');

  // reduce – number, no initialValue, varying callback arity
  suite.addTest('reduce number/no init – full args', testReduceNumberNoinitFullargs);
  suite.addTest('reduce number/no init – 3 args', testReduceNumberNoinitThreeargs);
  suite.addTest('reduce number/no init – 2 args', testReduceNumberNoinitTwoargs);
  suite.addTest('reduce number/no init – 1 arg', testReduceNumberNoinitOnearg);
  suite.addTest('reduce number/no init – 0 args', testReduceNumberNoinitZeroargs);

  // reduce – number with initialValue
  suite.addTest('reduce number/with init – full args', testReduceNumberWithinitFullargs);

  // reduce – generic<U=...> with initialValue, varying arity
  suite.addTest('reduce generic<U=string>/with init – full args', testReduceGenericStringFullargs);
  suite.addTest('reduce generic<U=object>/with init – 3 args', testReduceGenericObjectThreeargs);
  suite.addTest('reduce generic<U=boolean>/with init – 2 args', testReduceGenericBooleanTwoargs);
  suite.addTest('reduce generic<U=number>/with init – 1 arg', testReduceGenericNumberOnearg);
  suite.addTest('reduce generic<U=number>/with init – 0 args', testReduceGenericNumberZeroargs);

  // reduce – index/array params + exception cases
  suite.addTest('reduce – index and array params', testReduceIndexAndArrayParams);
  suite.addTest('reduce – throws on empty without init', testReduceThrowsOnEmpty);

  // reduceRight – number, no initialValue, varying callback arity
  suite.addTest('reduceRight number/no init – full args', testReducerightNumberNoinitFullargs);
  suite.addTest('reduceRight number/no init – 3 args', testReducerightNumberNoinitThreeargs);
  suite.addTest('reduceRight number/no init – 2 args', testReducerightNumberNoinitTwoargs);
  suite.addTest('reduceRight number/no init – 1 arg', testReducerightNumberNoinitOnearg);
  suite.addTest('reduceRight number/no init – 0 args', testReducerightNumberNoinitZeroargs);

  // reduceRight – number with initialValue
  suite.addTest('reduceRight number/with init – full args', testReducerightNumberWithinitFullargs);

  // reduceRight – generic<U=...> with initialValue, varying arity
  suite.addTest('reduceRight generic<U=string>/with init – full args', testReducerightGenericStringFullargs);
  suite.addTest('reduceRight generic<U=object>/with init – 3 args', testReducerightGenericObjectThreeargs);
  suite.addTest('reduceRight generic<U=boolean>/with init – 2 args', testReducerightGenericBooleanTwoargs);
  suite.addTest('reduceRight generic<U=number>/with init – 1 arg', testReducerightGenericNumberOnearg);
  suite.addTest('reduceRight generic<U=number>/with init – 0 args', testReducerightGenericNumberZeroargs);

  // reduceRight – order + exception
  suite.addTest('reduceRight – order verification', testReducerightOrderVerification);
  suite.addTest('reduceRight – throws on empty without init', testReducerightThrowsOnEmpty);

  return suite.run();
}

/* reduce – number, no initialValue, varying callback arity */

function testReduceNumberNoinitFullargs(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const sum = arr.reduce((prev: number, cur: number, idx: int, a: Uint16Array): number => prev + cur);
  arktest.assertEQ(sum, 10);
}

function testReduceNumberNoinitThreeargs(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const sum = arr.reduce((prev: number, cur: number, idx: int): number => prev + cur);
  arktest.assertEQ(sum, 6);
}

function testReduceNumberNoinitTwoargs(): void {
  const arr = new Uint16Array([5, 2, 7]);
  const sum = arr.reduce((prev: number, cur: number): number => prev + cur);
  arktest.assertEQ(sum, 14);
}

function testReduceNumberNoinitOnearg(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const res = arr.reduce((prev: number): number => prev + 1);
  arktest.assertEQ(res, arr[0] + 3);
}

function testReduceNumberNoinitZeroargs(): void {
  const arr = new Uint16Array([9, 8, 7]);
  const res = arr.reduce((): number => 123);
  arktest.assertEQ(res, 123);
}

/* reduce – number with initialValue */

function testReduceNumberWithinitFullargs(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const sum = arr.reduce((prev: number, cur: number, idx: int, a: Uint16Array): number => prev + cur, 10);
  arktest.assertEQ(sum, 20);
}

/* reduce – generic<U=...> with initialValue, varying arity */

function testReduceGenericStringFullargs(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const joined = arr.reduce<string>(
    (prev: string, cur: number, idx: int, a: Uint16Array): string => prev + `-${cur}@${idx}`,
    'start'
  );
  arktest.assertEQ(joined, 'start-1@0-2@1-3@2');
}

function testReduceGenericObjectThreeargs(): void {
  const arr = new Uint16Array([2, 4, 6]);
  const acc = arr.reduce<AccSumCount>((prev: AccSumCount, cur: number, idx: int): AccSumCount => {
    prev.sum += cur;
    prev.count += 1;
    return prev;
  }, new AccSumCount(0, 0));
  arktest.assertEQ(acc.sum, 12);
  arktest.assertEQ(acc.count, 3);
}

function testReduceGenericBooleanTwoargs(): void {
  const arr = new Uint16Array([1, 3, 5, 7]);
  const allOdd = arr.reduce<boolean>((prev: boolean, cur: number): boolean => prev && (cur % 2 !== 0), true);
  arktest.assertTrue(allOdd);
}

function testReduceGenericNumberOnearg(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const calls = arr.reduce<number>((prev: number): number => prev + 1, 0);
  arktest.assertEQ(calls, 4);
}

function testReduceGenericNumberZeroargs(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const res = arr.reduce<number>(() => 7, 100);
  arktest.assertEQ(res, 7);
}

/* reduce – index and array param verification */

function testReduceIndexAndArrayParams(): void {
  const arr = new Uint16Array([10, 20, 30]);
  let seenSameArray = true;
  const visited: number[] = [];
  const sum = arr.reduce((prev: number, cur: number, idx: int, a: Uint16Array): number => {
    if (a !== arr) {
        seenSameArray = false
    }
    visited.push(idx);
    return prev + cur;
  });
  arktest.assertTrue(seenSameArray);
  arktest.assertEQ(visited.length, 2); // no init => indexes 1,2
  arktest.assertEQ(visited[0], 1);
  arktest.assertEQ(visited[1], 2);
  arktest.assertEQ(sum, 60);
}

/* reduce – exceptions */

function testReduceThrowsOnEmpty(): void {
  const empty = new Uint16Array(0);
  let threw = false;
  try {
    empty.reduce((p: number, c: number) => p + c);
  } catch (e) {
    threw = true;
  }
  arktest.assertTrue(threw);
}

/* reduceRight – number, no initialValue, varying callback arity */

function testReducerightNumberNoinitFullargs(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const diff = arr.reduceRight((prev: number, cur: number, idx: int, a: Uint16Array): number => prev - cur);
  arktest.assertEQ(diff, -2);
}

function testReducerightNumberNoinitThreeargs(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const sum = arr.reduceRight((prev: number, cur: number, idx: int): number => prev + cur);
  arktest.assertEQ(sum, 6);
}

function testReducerightNumberNoinitTwoargs(): void {
  const arr = new Uint16Array([5, 2, 7]);
  const sum = arr.reduceRight((prev: number, cur: number): number => prev + cur);
  arktest.assertEQ(sum, 14);
}

function testReducerightNumberNoinitOnearg(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const res = arr.reduceRight((prev: number): number => prev + 1);
  arktest.assertEQ(res, arr[3] + 3);
}

function testReducerightNumberNoinitZeroargs(): void {
  const arr = new Uint16Array([9, 8, 7]);
  const res = arr.reduceRight((): number => 50);
  arktest.assertEQ(res, 50);
}

/* reduceRight – number with initialValue */

function testReducerightNumberWithinitFullargs(): void {
  const arr = new Uint16Array([1, 2, 3, 4]);
  const res = arr.reduceRight((prev: number, cur: number, idx: int, a: Uint16Array): number => prev + cur, 10);
  arktest.assertEQ(res, 20);
}

/* reduceRight – generic<U=...> with initialValue, varying arity */

function testReducerightGenericStringFullargs(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const joined = arr.reduceRight<string>(
    (prev: string, cur: number, idx: int, a: Uint16Array): string => prev + `(${cur}<${idx})`,
    'R'
  );
  arktest.assertEQ(joined, 'R(3<2)(2<1)(1<0)');
}

function testReducerightGenericObjectThreeargs(): void {
  const arr = new Uint16Array([2, 4, 6]);
  const acc = arr.reduceRight<AccProdSteps>((prev: AccProdSteps, cur: number, idx: int): AccProdSteps => {
    prev.prod *= cur;
    prev.steps += 1;
    return prev;
  }, new AccProdSteps(1, 0));
  arktest.assertEQ(acc.prod, 48);
  arktest.assertEQ(acc.steps, 3);
}

function testReducerightGenericBooleanTwoargs(): void {
  const arr = new Uint16Array([2, 4, 6]);
  const anyEvenFromRight = arr.reduceRight<boolean>((prev: boolean, cur: number): boolean => prev || (cur % 2 === 0),
      false);
  arktest.assertTrue(anyEvenFromRight);
}

function testReducerightGenericNumberOnearg(): void {
  const arr = new Uint16Array([1, 2, 3, 4, 5]);
  const calls = arr.reduceRight<number>((prev: number): number => prev + 1, 0);
  arktest.assertEQ(calls, 5);
}

function testReducerightGenericNumberZeroargs(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const res = arr.reduceRight<number>(() => -1, 999);
  arktest.assertEQ(res, -1);
}

/* reduceRight – order verification */

function testReducerightOrderVerification(): void {
  const arr = new Uint16Array([1, 2, 3]);
  const left = arr.reduce((p: number, c: number) => p * 10 + c, 0);
  const right = arr.reduceRight((p: number, c: number) => p * 10 + c, 0);
  arktest.assertEQ(left, 123);
  arktest.assertEQ(right, 321);
}

/* reduceRight – exceptions */

function testReducerightThrowsOnEmpty(): void {
  const empty = new Uint16Array(0);
  let threw = false;
  try {
    empty.reduceRight((p: number, c: number) => p + c);
  } catch (e) {
    threw = true;
  }
  arktest.assertTrue(threw);
}
