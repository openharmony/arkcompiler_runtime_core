/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    let suite = new arktest.ArkTestsuite("Chaining operator precedence")

    suite.addTest("optional method call", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x?.foo(1), 1)
    })

    suite.addTest("optional on new expression", () => {
        arktest.assertEQ((new Bar())?.foo(2), 2)
    })

    suite.addTest("chained optional methods", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x?.self()?.foo(3), 3)
    })

    suite.addTest("non-optional then optional", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x.self()?.foo(4), 4)
    })

    suite.addTest("optional then non-optional", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x?.self().foo(5), 5)
    })

    suite.addTest("optional property", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x?.intVal, undefined)
    })

    suite.addTest("optional property chain", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x?.intVal?.toString(), undefined)
    })

    suite.addTest("deep chain", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x.self().self().bar?.bar?.foo(1), undefined)
    })

    suite.addTest("multiline chain", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x
            ?.self()
            ?.
            self()
            .bar
            ?.bar
            ?.foo(1), undefined)
    })

    suite.addTest("optional index", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x?.row?.[0], undefined)
    })

    suite.addTest("optional index method", () => {
        let x: Bar = new Bar();
        arktest.assertEQ(x.row?.[0]?.foo(1), undefined)
    })

    suite.addTest("null coalescing with null", () => {
        let x: Bar = new Bar();

        arktest.assertEQ((x?.bar??null), null)
    })

    suite.addTest("null coalescing with fallback", () => {
        let x: Bar = new Bar();

        arktest.assertEQ((x?.bar??x?.foo(1)), 1)
    })

    suite.addTest("equality ==", () => {
        let x: Bar = new Bar();

        arktest.assertTrue((new Bar()?.foo(1)==x?.foo(1)))
    })

    suite.addTest("equality !=", () => {
        let x: Bar = new Bar();

        arktest.assertFalse((x?.foo(1)!=new Bar()?.foo(1)))
    })

    suite.addTest("equality == undefined", () => {
        let x: Bar = new Bar();

        arktest.assertTrue((x ?. intVal == new Bar() ?. intVal))
    })

    suite.addTest("cast", () => {
        let x: Bar = new Bar();

        let raised = false;
        try {
            let m = new More();
            arktest.assertFalse((m ?. bar as Bar) instanceof Bar)
        } catch (e) {
            arktest.assertTrue(e instanceof ClassCastError)
            raised = true;
        }
        arktest.assertTrue(raised)
    })

    suite.addTest("typeof undefined", () => {
        let x: Bar = new Bar();

        arktest.assertEQ((typeof x ?. bar), 'undefined')
    })

    suite.addTest("typeof object", () => {
        let x: Bar = new Bar();

        arktest.assertEQ((typeof x ?. self()), 'object')
    })

    suite.addTest("instanceof", () => {
        let x: Bar = new Bar();

        arktest.assertTrue((x ?. intVal instanceof undefined))
    })

    return suite.run()
}
