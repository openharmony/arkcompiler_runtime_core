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

let err: Error = new Error('This is an error');
let rangeErr: RangeError = new RangeError();
let referenceErr: ReferenceError = new ReferenceError();
let syntaxErr: SyntaxError = new SyntaxError();
let uriErr: URIError = new URIError();
let typeErr: TypeError = new TypeError();
let linkerClassNotFoundError: LinkerClassNotFoundError = new LinkerClassNotFoundError();

function testThrowError(): void { throw err; }
function testThrowRangeError(): void { throw rangeErr; }
function testThrowReferenceError(): void { throw referenceErr; }
function testThrowSyntaxError(): void { throw syntaxErr; }
function testThrowURIError(): void { throw uriErr; }
function testThrowTypeError(): void { throw typeErr; }
function testThrowLinkerClassNotFoundError(): void { throw new LinkerClassNotFoundError(); }

function testError(message: string): Error {
    return new Error(message);
}
function testRangeError(message: string): RangeError {
    return new RangeError(message);
}
function testReferenceError(message: string): ReferenceError {
    return new ReferenceError(message);
}
function testSyntaxError(message: string): SyntaxError {
    return new SyntaxError(message);
}
function testURIError(message: string): URIError {
    return new URIError(message);
}
function testTypeError(message: string): TypeError {
    return new TypeError(message);
}

class ExtendError extends Error {
    constructor(message: string) {
        super(message);
        this.name = 'ExtendError';
    }
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

let extendError: ExtendError = new ExtendError('extendError');
let extendRangeError: ExtendRangeError = new ExtendRangeError('extendRangeError');
let extendReferenceError: ExtendReferenceError = new ExtendReferenceError('extendReferenceError');
let extendSyntaxError: ExtendSyntaxError = new ExtendSyntaxError('extendSyntaxError');
let extendURIError: ExtendURIError = new ExtendURIError('extendURIError');
let extendTypeError: ExtendTypeError = new ExtendTypeError('extendTypeError');

function testExtendThrowRangeError(message: string): void {
    throw new ExtendRangeError(message);
}
function testExtendThrowReferenceError(message: string): void {
    throw new ExtendReferenceError(message);
}
function testExtendThrowSyntaxError(message: string): void {
    throw new ExtendSyntaxError(message);
}
function testExtendThrowURIError(message: string): void {
    throw new ExtendURIError(message);
}
function testExtendThrowTypeError(message: string): void {
    throw new ExtendTypeError(message);
}
