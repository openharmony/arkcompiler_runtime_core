/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
desc: 05.Generics
name: issue14752.ets
tags: [compile-only, negative]
---*/

interface I<T> {
    get<T>(): I<T>
}

class B<T> implements I<T> {
    private b: B<T> = new B<T>()

    override get<T>(): I<T> { return this.b as I<T> }

    foo<T>(): I<T> {
        return true ? this.b : this.get<T>();
    }
}
