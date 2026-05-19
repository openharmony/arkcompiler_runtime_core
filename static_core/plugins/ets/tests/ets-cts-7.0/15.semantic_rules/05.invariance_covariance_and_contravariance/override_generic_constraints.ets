/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Override-compatible signatures must have type parameter constraints contravariant to the base class
    (15.8.3 rule 6). Derived class can widen constraints.
---*/

class WidenBase {
    foo<T extends String>(p: T): T { return p }
}
class WidenSub extends WidenBase {
    override foo<T extends Object>(p: T): T { return p }
}

class CovGenBase<out T> {
    get(): T { return "base" as T }
}
class CovGenSub<out T> extends CovGenBase<T> {
    override get(): T { return "sub" as T }
}

interface UnionRetBase {
    ret<T extends String, U extends Object>(): T | U
}
class UnionRetImpl implements UnionRetBase {
    ret<T extends Object, U extends Object>(): T | U {
        return "ok" as T | U
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Override compatibility with generic constraints")

    suite.addTest("derived widens constraint: T extends String -> T extends Object", () => {
        let sub: WidenBase = new WidenSub()
        arktest.assertEQ(sub.foo<String>("hello"), "hello")
    })

    suite.addTest("override in covariant generic class hierarchy", () => {
        let b: CovGenBase<Object> = new CovGenSub<String>()
        arktest.assertEQ(b.get(), "sub")
    })

    suite.addTest("override with union return + contravariant constraints", () => {
        let b: UnionRetBase = new UnionRetImpl()
        let r: String | Object = b.ret<String, Object>()
        arktest.assertEQ(r.toString(), "ok")
    })

    suite.addTest("override dispatch through base reference works at runtime", () => {
        let base: WidenBase = new WidenSub()
        arktest.assertEQ(base.foo<String>("dispatched"), "dispatched")
        let derived: WidenSub = new WidenSub()
        arktest.assertEQ(derived.foo<Object>(new Object()) instanceof Object, true)
    })

    return suite.run()
}
