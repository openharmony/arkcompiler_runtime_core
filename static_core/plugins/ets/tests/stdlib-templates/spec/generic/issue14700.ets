/*
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

{% for item in btypes %}
/*---
desc: {function: ArrayBackedConstructorWith{{.item.type}}}
name: issue14700.ets
tags: []

---*/


class Buffer {
}

class TestArrayBuffer extends Buffer {
    constructor(size:  number) {
    }
}

export final class ArrayBackedConstructor {
    public constructor(buff: Array<{{.item.type}}>) {
        this();
    }

    public static readonly BYTES_PER_ELEMENT: number = 1;

    public constructor() {
        this(new TestArrayBuffer(0), 0, 0);
    }

    public constructor(buf: Buffer, offset: number, length: number) {
    }
}


function main(): void {
    let xx: ArrayBackedConstructor  = new ArrayBackedConstructor(new Array<{{.item.type}}>());
    assertNE(xx, null);
}

{%- endfor %}
