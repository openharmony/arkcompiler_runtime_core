/*
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

/*---
desc: >-
    Casting conversion from a type parameter attempts to convert an expression
    of the type parameter to any reference type (see Reference Types) which
    is to be specified as a target type
tags: [compile-only]
---*/


class X<S, T> {
    method (p: T) {
        p as Object // OK
        p as Object[] // OK
        p as [Object, Object] // OK
        p as () => void // OK
        p as T // OK
        p as S // OK
        p as Number // OK use this cast instead of the cast to value type
    }
}
