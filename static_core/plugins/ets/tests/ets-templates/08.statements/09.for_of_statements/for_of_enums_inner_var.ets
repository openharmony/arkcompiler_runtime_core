/*
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
{%- for c in cases %}

/*---
desc: >-
    For-of variable is local, specified inside the loop with inferred type.
    {{c.desc}}
---*/

class EnumIterable<T> implements Iterable<T> {
    data: T[]
    constructor(p: T[]) {
        this.data = p
    }
    $_iterator() {
        return new EnumIterator<T>(this)
    }
}

class EnumIterator<T> implements Iterator<T> {
    index = 0
    base: EnumIterable<T>
    constructor (base: EnumIterable<T>) {
        this.base = base
    }
    next(): IteratorResult<T> {
        return new IteratorResult<T>(
            this.index >= this.base.data.length,
            this.index >= this.base.data.length ? undefined : this.base.data[this.index++]
        )
    }
}

enum Enum1 { Zero, Five = 5, Six }
enum Enum2 { Seven = 7, Ten = 10 }
{{c.decl}}

function main() {
    let res = 0
    {{c.main|indent}}
}
{% endfor %}
