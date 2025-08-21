/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/callable_signature/ts_to_sts/main');
let callableInstance = module.getProperty('callableInstance');

const etsInt: int = 1;

function checkCallableSignatureWithArg(): boolean {
    try {
        if (callableInstance.isUndefined() || callableInstance.isNull() || !callableInstance.isFunction()) {
            return false;
        }

        const res = callableInstance.invoke(ESValue.wrapInt(etsInt));

        if (res.isUndefined() || res.isNull()) {
            return false;
        }

        const unwrapped = res.unwrap();
        if (!(unwrapped instanceof Array)) {
            return false;
        }

        const firstElement = res.getProperty(0);
        if (!firstElement.isNumber()) {
            return false;
        }

        return firstElement.toNumber() == etsInt;

    } catch (e) {
        return false;
    }
}

function checkCallableSignatureWithoutArg(): boolean {
    try {
        if (callableInstance.isUndefined() || callableInstance.isNull() || !callableInstance.isFunction()) {
            return false;
        }

        const res = callableInstance.invoke();

        if (res.isUndefined() || res.isNull()) {
            return false;
        }

        const unwrapped = res.unwrap();
        return unwrapped instanceof Array;

    } catch (e) {
        return false;
    }
}