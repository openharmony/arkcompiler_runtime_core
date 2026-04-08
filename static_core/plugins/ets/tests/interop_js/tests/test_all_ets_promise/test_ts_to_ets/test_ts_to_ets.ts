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

export async function getPromiseNumber(): Promise<number> {
    return 42;
}

export async function getPromiseString(): Promise<string> {
    return 'hello';
}

export async function getPromiseBoolean(): Promise<boolean> {
    return true;
}

export async function getPromiseBigInt(): Promise<bigint> {
    return 123456789n;
}

export async function getPromiseNull(): Promise<null> {
    return null;
}

export async function getPromiseUndefined(): Promise<undefined> {
    return undefined;
}

export async function getPromiseNan(): Promise<number> {
    return NaN;
}

export async function getPromiseInfinity(): Promise<number> {
    return Infinity;
}

export async function getRejectPromise(): Promise<void> {
    return Promise.reject(new Error('error'));
}

export async function getRejected1(): Promise<void> {
    return Promise.reject(new Error('error1'));
}

export async function getRejected2(): Promise<void> {
    return Promise.reject(new Error('error2'));
}

export function createAllEmpty(): Promise<Array<Object>> {
    return Promise.all([]);
}

export function createAllWithPromises(): Promise<Array<number>> {
    const p1 = new Promise<number>((resolve) => setTimeout(() => resolve(10), 20));
    const p2 = new Promise<number>((resolve) => setTimeout(() => resolve(20), 10));
    return Promise.all([p1, p2, 30]);
}

export function createAllWithRejected(): Promise<Array<number>> {
    return Promise.all([
        Promise.resolve(1),
        Promise.reject(new Error('error')),
        Promise.resolve(3)
    ]);
}

export function createAllMixed(): Promise<Array<Object>> {
    return Promise.all(['a', Promise.resolve(123), 'c']);
}

export function createAllWithString(): Promise<Array<string>> {
    return Promise.all('ab');
}

export function createAnyEmpty(): Promise<void> {
    return Promise.any([]);
}

export async function getFulfilled1(): Promise<number> {
    return new Promise(resolve => setTimeout(() => resolve(1), 10));
}

export async function getFulfilled2(): Promise<number> {
    return new Promise(resolve => setTimeout(() => resolve(2), 20));
}

export async function getFulfilled3(): Promise<number> {
    return new Promise(resolve => setTimeout(() => resolve(3), 30));
}

export function createAnyAllRejected(): Promise<Array<string>> {
    return Promise.any([
        Promise.reject('A'),
        Promise.reject('B'),
        Promise.reject('C')
    ]).catch((e: AggregateError) => e.errors) as Promise<Array<string>>;
}

export function createAnyMixed(): Promise<number> {
    return Promise.any([
        Promise.reject('slow_fail'),
        100,
        new Promise(resolve => setTimeout(() => resolve('slow'), 100))
    ]) as Promise<number>;
}

export function createAnyFirstWins(): Promise<string> {
    return Promise.any([
        new Promise(resolve => setTimeout(() => resolve('slow'), 100)),
        new Promise(resolve => setTimeout(() => resolve('fast'), 50)),
        Promise.reject('error')
    ]) as Promise<string>;
}

export function createRaceEmpty(): Promise<void> {
    return Promise.race([]);
}

export function createRacePlainValues(): Promise<number> {
    return Promise.race([1, 2, 3]);
}

export function createRaceFulfilledWins(): Promise<string> {
    return Promise.race([
        new Promise(resolve => setTimeout(() => resolve('slow'), 100)),
        new Promise(resolve => setTimeout(() => resolve('fast'), 50))
    ]) as Promise<string>;
}

export function createRaceRejectedWins(): Promise<void> {
    return Promise.race([
        new Promise((_, reject) => setTimeout(() => reject(new Error('fail')), 50)),
        new Promise(resolve => setTimeout(() => resolve('success'), 100))
    ]) as unknown as Promise<void>;
}

export function createRaceMixedImmediateWins(): Promise<number> {
    return Promise.race([
        new Promise(resolve => setTimeout(() => resolve('slow'), 100)),
        42
    ]) as Promise<number>;
}

export function createRaceRejectImmediate(): Promise<void> {
    return Promise.race([
        Promise.reject(new Error('immediate_fail')),
        Promise.resolve('too_late')
    ]) as unknown as Promise<void>;
}

export function createRaceWithString(): Promise<string> {
    return Promise.race('ab');
}

export let cbLambda: () => Promise<number> = async () => {
    return new Promise(resolve => setTimeout(() => resolve(24)));
};

export class CreateClass1 {
    async foo(): Promise<string> {
        await new Promise(resolve => setTimeout(resolve, 10));
        return 'Hello from foo!';
    }
    static async foo1(): Promise<string> {
        await new Promise(resolve => setTimeout(resolve, 10));
        return 'Hello!';
    }
}
