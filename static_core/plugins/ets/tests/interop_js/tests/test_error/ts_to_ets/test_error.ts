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

export function throwRangeError(): void {
    throw new RangeError();
}

export function throwRangeErrorWithMessage(message: string): void {
    throw new RangeError(message);
}

export function throwReferenceError(): void {
    throw new ReferenceError();
}

export function throwReferenceErrorWithMessage(message: string): void {
    throw new ReferenceError(message);
}

export function throwSyntaxError(): void {
    throw new SyntaxError();
}

export function throwSyntaxErrorWithMessage(message: string): void {
    throw new SyntaxError(message);
}

export function throwURIError(): void {
    throw new URIError();
}

export function throwURIErrorWithMessage(message: string): void {
    throw new URIError(message);
}

export function throwTypeError(): void {
    throw new TypeError();
}

export function throwTypeErrorWithMessage(message: string): void {
    throw new TypeError(message);
}

export function throwEvalError(): void {
    throw new EvalError();
}

export function throwEvalErrorWithMessage(message: string): void {
    throw new EvalError(message);
}

export function throwAggregateError(): void {
    throw new AggregateError([new Error('Error 1'), new TypeError('Error 2')], 'Multiple errors occurred');
}

export function throwAggregateErrorWithMessage(message: string): void {
    throw new AggregateError([new Error('First error'), new RangeError('Second error')], message);
}

class ExtendRangeError extends RangeError {
    constructor(message: string) {
        super(message);
        this.name = 'ExtendRangeError';
    }
}
class ExtendReferenceError extends ReferenceError {
    constructor(message: string) {
        super(message);
        this.name = 'ExtendReferenceError';
    }
}
class ExtendSyntaxError extends SyntaxError {
    constructor(message: string) {
        super(message);
        this.name = 'ExtendSyntaxError';
    }
}
class ExtendURIError extends URIError {
    constructor(message: string) {
        super(message);
        this.name = 'ExtendURIError';
    }
}
class ExtendTypeError extends TypeError {
    constructor(message: string) {
        super(message);
        this.name = 'ExtendTypeError';
    }
}

export function testExtendThrowRangeError(message: string): void {
    throw new ExtendRangeError(message);
}
export function testExtendThrowReferenceError(message: string): void {
    throw new ExtendReferenceError(message);
}
export function testExtendThrowSyntaxError(message: string): void {
    throw new ExtendSyntaxError(message);
}
export function testExtendThrowURIError(message: string): void {
    throw new ExtendURIError(message);
}
export function testExtendThrowTypeError(message: string): void {
    throw new ExtendTypeError(message);
}

export function throwNum(): void {
    throw 0;
}

export function throwStr(): void {
    throw 'hello';
}

export function throwObj(): void {
    throw {a: 1, b: 2};
}

class SubError extends Error {
    extraField = 123;
    foo(): number {
        return 456;
    }
}

export function throwErrorSubClass(): void {
    throw new SubError();
}

export function throwError1(): void {
    let err = new Error();
    err.message = {data: 123};
    throw err;
}

export function throwError2(): void {
    throw new Error('null pointer error');
}

export function throwError3(): void {
    try {
        throw new Error('original error');
    } catch (originalError) {
        throw new Error('current error', { cause: originalError });
    }
}