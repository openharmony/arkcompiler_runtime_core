/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

type AnyType = {};

const callableProto = {
    _call(...args): AnyType[] {
        return args;
    }
};

type ReturnType = (...args: AnyType[]) => AnyType;

function createCallable(): ReturnType {
    const fn = function (...args): ReturnType {
        return fn._call(...args);
    };

    Object.setPrototypeOf(fn, callableProto);

    return new Proxy(fn, {
        apply: (target, thisArg, args): AnyType[] => target._call(...args)
    });
}

export const callableInstance = createCallable();
