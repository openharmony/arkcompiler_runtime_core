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

function xassert(a: Any, b: Any) { return assertEQ(a, b) }

function fakeLambdaInvoke(a: string, b: string | undefined): string {
    return a + (b === undefined ? 'u' : b);
}

class FakeLambdaObject extends Lambda2 implements Function1<Any, Any> {
    public constructor() { }

    public invoke1(a: Any): Any {
        return fakeLambdaInvoke(a as string, undefined)
    }

    public invokeR1(a: Any, ...r: FixedArray<Any>): Any {
        r = Runtime.restArgumentsUnderapplicationStub(r, 1)
        return fakeLambdaInvoke(a as string, r[0] as string | undefined)
    }

    public invoke2(a: Any, b: Any): Any {
        return fakeLambdaInvoke(a as string, b as string | undefined)
    }

    public $_invoke(a: string, b: string | undefined): string {
        return fakeLambdaInvoke(a, b)
    }
}

function testF1(f: Function1<Any, Any>) {
    xassert(f.invoke1('a'), 'au')
    xassert(f.invoke2('a', 'b'), 'ab')
    xassert(f.invoke3('a', 'b', 'c'), 'ab')
    xassert(f.invokeR1('a'), 'au')
    xassert(f.invokeR1('a', 'b'), 'ab')
    xassert(f.invokeR1('a', 'b', 'c'), 'ab')
    xassert(f.invokeR2('a', 'b'), 'ab')
    xassert(f.invokeR2('a', 'b', 'c'), 'ab')
    xassert(f.invokeR3('a', 'b', 'c'), 'ab')
    xassert(f.invokeR3('a', 'b', 'c', 'd'), 'ab')
}

function main() {
    testF1(new FakeLambdaObject())
}
