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

export async function getFulfilled1(): Promise<number> {
    return new Promise<number>((resolve: (value: number) => void): void => {
        setTimeout((): void => { resolve(1); }, 10);
    });
}

export async function getFulfilled2(): Promise<number> {
    return new Promise<number>((resolve: (value: number) => void): void => {
        setTimeout((): void => { resolve(2); }, 20);
    });
}

export async function getFulfilled3(): Promise<number> {
    return new Promise<number>((resolve: (value: number) => void): void => {
        setTimeout((): void => { resolve(3); }, 30);
    });
}

export function createAllEmpty(): Promise<Array<Object>> {
    return Promise.all<Object>([]);
}

export function createAllWithPromises(): Promise<Array<number>> {
    let p1: Promise<number> = new Promise<number>((resolve: (value: number) => void): void => {
        setTimeout((): void => { resolve(10); }, 20);
    });
    let p2: Promise<number> = new Promise<number>((resolve: (value: number) => void): void => {
        setTimeout((): void => { resolve(20); }, 10);
    });
    return Promise.all([p1, p2, Promise.resolve<number>(30)]);
}

export function createAllWithRejected(): Promise<Array<number>> {
    return Promise.all<number>([
        Promise.resolve<number>(1),
        Promise.reject<number>(new Error('error')),
        Promise.resolve<number>(3)
    ]);
}

export function createAllMixed(): Promise<Array<Object>> {
    return Promise.all<Object>(['a', Promise.resolve<Object>(123), 'c']);
}

export function createAllWithString(): Promise<Array<string>> {
    return Promise.all('ab');
}

export function createAnyEmpty(): Promise<Object> {
    return Promise.any<Object>([]);
}

export function createAnyAllRejected(): Promise<Object> {
    return Promise.any<Object>([
        Promise.reject<Object>(new Error('A')),
        Promise.reject<Object>(new Error('B')),
        Promise.reject<Object>(new Error('C'))
    ]).catch((e: Error): Array<string> => {
        let result = new Array<string>();
        let ae = e as AggregateError;
        for (let i = 0; i < ae.errors.length; i++) {
            result.push(ae.errors[i].message);
        }
        return result;
    });
}

export function createAnyMixed(): Promise<Object> {
    return Promise.any<Object>([
        Promise.reject<Object>(new Error('slow_fail')),
        100,
        new Promise<Object>((resolve: (value: Object) => void): void => {
            setTimeout((): void => { resolve('slow'); }, 100);
        })
    ]);
}

export function createAnyFirstWins(): Promise<string> {
    return Promise.any([
        new Promise<string>((resolve: (value: string) => void): void => {
            setTimeout((): void => { resolve('slow'); }, 100);
        }),
        new Promise<string>((resolve: (value: string) => void): void => {
            setTimeout((): void => { resolve('fast'); }, 50);
        }),
        Promise.reject(new Error('error'))
    ]);
}

export function createRaceEmpty(): Promise<Object> {
    return Promise.race<Object>([]);
}

export function createRacePlainValues(): Promise<number> {
    return Promise.race<number>([1, 2, 3]);
}

export function createRaceFulfilledWins(): Promise<string> {
    return Promise.race([
        new Promise<string>((resolve: (value: string) => void): void => {
            setTimeout((): void => { resolve('slow'); }, 100);
        }),
        new Promise<string>((resolve: (value: string) => void): void => {
            setTimeout((): void => { resolve('fast'); }, 50);
        })
    ]);
}

export function createRaceRejectedWins(): Promise<void> {
    const rejected = new Promise<void>((_, reject) => reject(new Error('fail')));
    const delayed = new Promise<void>((resolve) => {
        setTimeout(() => resolve(undefined), 50);
    });
    return Promise.race<void>([rejected, delayed]);
}

export function createRaceMixedImmediateWins(): Promise<Object> {
    return Promise.race([
        new Promise<Object>((resolve: (value: Object) => void): void => {
            setTimeout((): void => { resolve('slow'); }, 100);
        }),
        42
    ]);
}

export function createRaceRejectImmediate(): Promise<void> {
    return Promise.race<void>([
        Promise.reject<void>(new Error('immediate_fail')),
        Promise.resolve()
    ]);
}

export function createRaceWithString(): Promise<string> {
    return Promise.race('ab');
}

export let cbLambda: () => Promise<number> = async (): Promise<number> => {
    return new Promise<number>((resolve: (value: number) => void): void => {
        setTimeout((): void => { resolve(24); }, 10);
    });
};

export class CreateClass1 {
    async foo(): Promise<string> {
        await new Promise<void>((resolve): void => {
            setTimeout((): void => { resolve(undefined); }, 10);
        });
        return 'Hello from foo!';
    }
    static async foo1(): Promise<string> {
        await new Promise<void>((resolve): void => {
            setTimeout((): void => { resolve(undefined); }, 10);
        });
        return 'Hello!';
    }
}
