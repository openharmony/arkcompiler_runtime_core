/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let failed = false;
let done = 0;
const TOTAL = 15;

function fail(msg: string): void {
    if (failed) {
        return;
    }
    failed = true;
    console.log('Test failed. ' + msg);
}

function doneOk(): void {
    if (failed) {
        return;
    }
    done++;
    if (done == TOTAL) {
        console.log('Test passed.');
    }
}

function expectNumberEq(actual: number, expected: number, msg: string): void {
    if (actual != expected) {
        fail(msg + ' Expected ' + expected + ' but got ' + actual + '.');
    }
}

function expectSameError(actual: Error, expected: Error, msg: string): void {
    if (actual != expected) {
        fail(msg + ' Expected the same Error instance.');
    }
}

function expectStringEq(actual: string, expected: string, msg: string): void {
    if (actual != expected) {
        fail(msg + ' Expected "' + expected + '" but got "' + actual + '".');
    }
}

class ThenDefaultHandlerBox {
    public x: number;
    constructor(x: number) {
        this.x = x;
    }
}

let pUndef: Promise<number> = Promise.resolve<number>(1).then(undefined);
let pEmpty: Promise<number> = Promise.resolve<number>(1).then();
if (pUndef == undefined || pEmpty == undefined) {
    console.log('unreachable');
}

Promise.resolve<number>(1).then(undefined).then<void, void>((v: number): void => {
    expectNumberEq(v, 1, 'then(undefined) should forward value.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in then(undefined) fulfilled path: ' + e);
});

Promise.resolve<number>(3).then().then<void, void>((v: number): void => {
    expectNumberEq(v, 3, 'then() should forward value.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in then() fulfilled path: ' + e);
});

Promise.resolve<number>(5).then().then(undefined).then().then<void, void>((v: number): void => {
    expectNumberEq(v, 5, 'Chained then()/then(undefined) should forward value.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in chained default handlers (fulfilled path): ' + e);
});

Promise.resolve<number>(NaN).then().then<void, void>((v: number): void => {
    if (v == v) {
        fail('then() should forward NaN without conversion.');
        return;
    }
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in then() NaN forwarding path: ' + e);
});

Promise.resolve<number>(Infinity).then(undefined).then<void, void>((v: number): void => {
    if (v != Infinity) {
        fail('then(undefined) should forward Infinity without conversion.');
        return;
    }
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in then(undefined) Infinity forwarding path: ' + e);
});

let obj = new ThenDefaultHandlerBox(1);
Promise.resolve<ThenDefaultHandlerBox>(obj).then().then(undefined).then<void, void>((v: ThenDefaultHandlerBox): void => {
    if (v != obj) {
        fail('Chained default handlers should preserve object identity.');
        return;
    }
    expectNumberEq(v.x, 1, 'Forwarded object should keep its data.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in object identity forwarding path: ' + e);
});

Promise.resolve<number>(10).then(undefined).then<number>((v: number): number => {
    return v + 1;
}).then<void, void>((v: number): void => {
    expectNumberEq(v, 11, 'then(undefined) should allow later mapping.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in mapping after then(undefined): ' + e);
});

Promise.resolve<number>(30).then().then<number>((v: number): number => {
    return v + 3;
}).then<void, void>((v: number): void => {
    expectNumberEq(v, 33, 'then() should allow later mapping.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in mapping after then(): ' + e);
});

let sUndef: Promise<string> = Promise.resolve<string>('abc').then(undefined);
let sEmpty: Promise<string> = Promise.resolve<string>('abc').then();
if (sUndef == undefined || sEmpty == undefined) {
    console.log('unreachable');
}

Promise.resolve<string>('abc').then(undefined).then<void, void>((v: string): void => {
    expectStringEq(v, 'abc', 'then(undefined) should forward string.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in then(undefined) string path: ' + e);
});

Promise.resolve<string>('ghi').then().then<void, void>((v: string): void => {
    expectStringEq(v, 'ghi', 'then() should forward string.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in then() string path: ' + e);
});

Promise.resolve<string>('a').then(undefined).then<string>((v: string): string => {
    return v + 'b';
}).then<void, void>((v: string): void => {
    expectStringEq(v, 'ab', 'Mapping after then(undefined) should work for string.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in string mapping after then(undefined): ' + e);
});

Promise.resolve<string>('e').then().then<string>((v: string): string => {
    return v + 'f';
}).then<void, void>((v: string): void => {
    expectStringEq(v, 'ef', 'Mapping after then() should work for string.');
    doneOk();
}).catch((e: Error): void => {
    fail('Caught error in string mapping after then(): ' + e);
});

let errUndef = new Error('err-undef');
Promise.reject<number>(errUndef).then(undefined).catch((e: Error): void => {
    expectSameError(e, errUndef, 'then(undefined) should forward rejection reason.');
    doneOk();
});

let errChain = new Error('err-chain');
Promise.reject<number>(errChain).then().then(undefined).then().catch((e: Error): void => {
    expectSameError(e, errChain, 'Chained then()/then(undefined) should forward rejection reason.');
    doneOk();
});

let errEmpty = new Error('err-empty');
Promise.reject<number>(errEmpty).then().catch((e: Error): void => {
    expectSameError(e, errEmpty, 'then() should forward rejection reason.');
    doneOk();
});
