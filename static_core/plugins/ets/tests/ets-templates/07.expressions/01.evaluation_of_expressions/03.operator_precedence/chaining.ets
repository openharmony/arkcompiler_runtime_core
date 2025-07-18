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
desc: chaining operator with other operators
---*/

interface Foo {
    foo (x: int): int
}

class Bar implements Foo {
    intVal?: Int
    bar?: Bar
    row?: Bar[]
    foo(x: int): int {
        return x
    }
    self(): Bar {
        return this;
    }
}

class More extends Bar {}

function main(): int {
    let x: Bar = new Bar();

    // grouping; member access
    if (x?.foo(1) != 1) return 1;
    if ((new Bar())?.foo(2) != 2) return 2;
    if (x?.self()?.foo(3) != 3) return 3;
    if (x.self()?.foo(4) != 4) return 4;
    if (x?.self().foo(5) != 5) return 5;
    if (x?.intVal != undefined) return 6;
    if (x?.intVal?.toString() != undefined) return 7;
    if (x.self().self().bar?.bar?.foo(1) != undefined) return 8;
    if (x
        ?.self()
        ?.
        self()
        .bar
        ?.bar
        ?.foo(1) != undefined) return 9;

    // access - lower precedence
    if (x?.row?.[0] != undefined) return 10;
    if (x.row?.[0]?.foo(1) != undefined) return 11;

    // null coalescing
    if ((x?.bar??null) !== null) return 12;
    if ((x?.bar??x?.foo(1)) !== 1) return 13;

    // equality
    if ((new Bar()?.foo(1)==x?.foo(1)) !== true) return 14;
    if ((x?.foo(1)!=new Bar()?.foo(1)) !== false) return 15;
    if ((x ?. intVal == new Bar() ?. intVal) !== true) return 16;

    // cast
    let raised = false;
    try {
        let m = new More();
        if ((m ?. bar as Bar) instanceof Bar) return 17;
    } catch (e) {
        if (!(e instanceof ClassCastError)) return 18;
        raised = true;
    }
    if (!raised) return 19;

    // typeof
    if ((typeof x ?. bar) !== 'undefined') return 20;
    if ((typeof x ?. self()) !== 'object') return 21;

    // instanceof
    if ((x ?. intVal instanceof undefined) !== true) return 22;

    return 0;
}
