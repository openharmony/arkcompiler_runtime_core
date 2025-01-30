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

function xgc(): void {
    JSRuntime.xgc();
}

class Result {
    constructor(p: Promise<Object>) {
        this.p = p;
    }

    p: Promise<Object>;
}

let root: Object|null = null;

function ignoreArg(p: Promise<Object>): void {
}

function rememberCycle(p: Promise<Object>): Result {
    let res = new Result(p);
    root = res;
    return res;
}

function forgetCycle(p: Promise<Object>): Result {
    let res = new Result(p);
    return res;
}

function sts2js(p: Promise<Object>): void {
    root = p;
}

function js2sts(): Object {
    return new Object();
}

function js2stsDeep(): Object {
    return new Result(Promise.resolve<Object>(new Object()));
}
