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

import * as dep from "@dep/x"

export namespace test1 {
    export class C1 implements dep.test1.I4 { }

    export function run(): void {
        arktest.expectThrow(() => { (new C1() as dep.test1.I1).m(new C1()); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test2 {
    export class C1 implements dep.test2.I2, dep.test2.I3 { }

    export function run(): void {
        const b = new C1();
        arktest.expectThrow(() => { (b as dep.test2.I2).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { (b as dep.test2.I3).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test3 {
    export class C2 extends dep.test3.C1 implements dep.test3.I1 { }

    export function run(): void {
        arktest.expectThrow(() => { (new C2() as dep.test3.C1).m(); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test4 {
    export class C2 extends dep.test4.C1 implements dep.test4.I1 { }

    export function run(): void {
        arktest.expectThrow(() => { (new C2() as dep.test4.I1).m(new dep.test4.A1()); },
            (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test5 {
    export class C2 extends dep.test5.C1 { }

    export function run(): void {
        const c2 = new C2();
        arktest.expectThrow(() => { (c2 as dep.test5.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
        arktest.expectThrow(() => { c2.m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test6 {
    export function run(): void {
        const c3 = new dep.test6.C3();
        arktest.expectThrow(() => { (c3 as dep.test6.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

export namespace test7 {
    export function run(): void {
        const c2 = new dep.test7.C2();
        arktest.expectThrow(() => { (c2 as dep.test7.I1).m(); }, (e) => e instanceof LinkerAbstractMethodError);
    }
}

test1.run();
test2.run();
test3.run();
test4.run();
test5.run();
test6.run();
test7.run();
