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

/*---
desc: Optional arguments for method with 'this' return type
tags: []
---*/

class NumberMethodParamClass {
    foo(value: number): this {
        return this;
    }
}


class OptionalNumberMethodParamClass {
    foo(value?: number): this {
        return this;
    }

    bar(value?: number): this {
        return this;
    }

    baz(name: string, value?: number): this {
        return this;
    }

    boo(): OptionalNumberMethodParamClass {
        return this.bar();
    }
}

class OverrideNumberMethodParamClass extends OptionalNumberMethodParamClass {
    override foo(value?: number): this {
        return this;
    }
}

function main(): void {
    let nm = new NumberMethodParamClass();
    assertEQ(nm, nm.foo(42).foo(1).foo(-5))

    let onm = new OptionalNumberMethodParamClass();
    assertEQ(onm, onm.foo())
    assertEQ(onm, onm.foo(42).bar().foo(1).bar(-5).baz("name", 42).boo())

    let oom = new OverrideNumberMethodParamClass();
    assertEQ(oom, oom.foo(42).bar().foo(1).bar(-5).baz("name", 42).boo())
}
