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

function xassert(v: boolean) { arktest.assertTrue(v) }

function expectError(action: () => void) {
    try { action() } catch (e) { return }
    throw new Error('Error expected, but nothing was thrown')
}

function expectErrorMessage(action: () => void, msg: String) {
    try { action() }
    catch (e) {
        if (e.message.includes(msg)) {
            return
        }
        throw new Error('Error message "' + e.message +
                        '" does not include expected substring "' + msg + '"')
    }
    throw new Error('Error expected, but nothing was thrown')
}

class X {
    public prop: String = 'ooooooo'
    foo() { return this.prop }
    bar(a: String) { return this.prop + a }
    public baz(a: String) { return a }
    public baz(a: number) { return a }

    private privateMethod() { return this.prop }
    fnWithTwoParams(a: Int, b: String) { return a }

    public qux(a: Date) { return a; }
    public xyz(a: String) { return a + 'X'; }
    public abc(a: number) { return a; }
    public inherit(p1: Y) { return "XY"; }
}

class Y extends X {
    public qux(a: String) { return a; }
    public override xyz(a: String) { return a + 'Y'; }
    public abc(a: String) { return a + 'Y'; }
    public inherit(p1: X) { return "YX"; }
}

class Z extends Y {
    public override abc(a: String) { return a + 'z';  }
}

class F {
    public prop: String = 'test'
}

function testldstname() {
    let x = new X();
    ((a: any) => a.prop = 'ttt')(x);
    xassert(((a: any) => a.prop)(x) == 'ttt');

    // expectError(() => { ((a: any) => a.prop = 1)(x) }); // exception is not thrown
    expectError(() => { ((a: any) => a.nope)(x) });
    expectError(() => { ((a: any) => a.nope = 1)(x) });
}

function testldstval() {
    let x = new X();
    ((a: any) => a['prop'] = 'uuu')(x);
    xassert(((a: any) => a["prop"])(x) == 'uuu');

    // expectError(() => { ((a: any) => a['prop'] = 1)(x) }); // exception is not thrown
    expectError(() => { ((a: any) => a['nope'])(x) });
    expectError(() => { ((a: any) => a['nope'] = 1)(x) });
}

function testldstidx() {
    let x: Any[] = [undefined, 21, 'ay'];
    ((a: any) => a[1] = 'aaa')(x);
    xassert(((a: any) => a[1])(x) == "aaa");

    // expectError(() => { ((a: any) => a[-1])(x) }); // nothing happens
    // expectError(() => { ((a: any) => a[-1] = 'aaa')(x) }); // nothing happens
    // expectError(() => { ((a: any) => a[1.5])(x) }); // nothing happens
}

function testcallthis() {
    let x = new X();
    let y = new Y();
    let z = new Z();
    ((a: any) => a['prop'] = 'ya')(x);
    xassert(((a: any) => a.foo())(x) == 'ya');
    xassert(((a: any) => a.bar('ee'))(x) == 'yaee');

    // Error: Method does not exist on type X
    expectError(() => { ((a: any) => a.missingMethod('ee'))(x) });
    // Error: Method is private in type X
    expectErrorMessage(() => { ((a: any) => a.privateMethod())(x) }, "not found");
    // Error: Missing required arguments for method
    expectError(() => { ((a: any) => a.fnWithTwoParams())(x) });

    // Error: Argument types do not match parameter types
    expectError(() => { ((a: any) => a.fnWithTwoParams('test', 100))(x) });

    // Verify boxed primitives are unboxed before call
    xassert(((a: any) => { return a.fnWithTwoParams(100, 'test') })(x) == 100);

    // Error: Second argument type is incompatible (Z instead of String type)
    expectError(() => {
        let f = new F();
        ((a: any) => a.fnWithTwoParams(100, f))(x);
    });

    // Error: Cannot resolve method overload (defined and overloaded in base class)
    expectError(() => { ((a: any) => a.baz('baz'))(y) });
    // Error: Cannot resolve method overload (defined and overloaded in base and derived classes)
    expectError(() => { ((a: any) => a.qux('qux'))(y) });

    // Verify overridden method without overloads resolves correctly
    xassert(((a: any) => a.xyz('xyz'))(y) == 'xyzY');

    // Error: Cannot resolve overridden method with overload in base class
    expectError(() => { ((a: any) => a.abc('abc'))(z) == 'abc'; });

    xassert(((a: any) => a.inherit(y))(x) == 'XY');
    expectError(() => { ((a: any) => a.inherit(x))(x) == 'XY'; });
    xassert(((a: any) => a.inherit(x))(y) == 'YX');
    xassert(((a: any) => a.inherit(y))(y) == 'YX');
}

function testcall() {
    xassert(((a: any) => a())(() => 'aaa') == 'aaa');
    xassert(((a: any) => a('bbb'))((v: string) => v) == 'bbb');
    xassert(((a: any) => a(123.0))((v: number) => v) == 123.0);
    xassert(((a: any) => a('foo', 'bar'))((v: String, w: String) => v + w) == 'foobar');

    expectError(() => { ((a: any) => a(1))((s: String) => s) });
}

function testcasts() {
    let x = new X()
    xassert(((a: any) => a as X)(x) == x)
    xassert(((a: any) => a instanceof X)(x) == true)
}

function test() {
    testldstval();
    testldstidx();
    testldstname();
    testcall();
    testcallthis();
    testcasts();
}
test()
