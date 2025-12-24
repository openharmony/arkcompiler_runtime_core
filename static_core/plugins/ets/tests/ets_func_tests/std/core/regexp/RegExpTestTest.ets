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

function test(): void {
    arktest.assertEQ(RegExp("abacaba").test("abacaba"), true)
    arktest.assertEQ(RegExp("abacaba").test("abacab_"), false)
    arktest.assertEQ(RegExp("(a.){4}").test("abacaba"), false)
    arktest.assertEQ(RegExp("(a.?){4}").test("abacaba"), true)
    arktest.assertEQ(RegExp("").test(""), true)
    arktest.assertEQ(RegExp("\u{55555}", "u").test("\u{55555}"), true)
    arktest.assertEQ(RegExp("\u{55555}", "u").test("abacaba"), false)
    arktest.assertEQ(RegExp("\u{12345}{3}", "u").test("\u{12345}\udf45\udf45"), false)
    arktest.assertEQ(RegExp("\u{12345}{3}").test("\u{12345}\udf45\udf45"), true)
    arktest.assertEQ(RegExp("\\u0410").test("\\u0410"), false)
    arktest.assertEQ(RegExp("\\u0410").test("\u0410"), true)
    arktest.assertEQ(RegExp("\u0410").test("\\u0410"), false)
    arktest.assertEQ(RegExp("\u0410").test("\u0410"), true)
    arktest.assertEQ(RegExp("\\u0410").test("bb\\u0410"), false)
    arktest.assertEQ(RegExp("\\u0410").test("bb\u0410"), true)
    arktest.assertEQ(RegExp("\u0410").test("bb\\u0410"), false)
    arktest.assertEQ(RegExp("\u0410").test("bb\u0410"), true)
    arktest.assertEQ(RegExp("\\u0410").test("bb\\u0410bb"), false)
    arktest.assertEQ(RegExp("\\u0410").test("bb\u0410bb"), true)
    arktest.assertEQ(RegExp("\u0410").test("bb\\u0410bb"), false)
    arktest.assertEQ(RegExp("\u0410").test("bb\u0410bb"), true)
    arktest.assertEQ(RegExp("\\u0410").test("\\u0410bb"), false)
    arktest.assertEQ(RegExp("\\u0410").test("\u0410bb"), true)
    arktest.assertEQ(RegExp("\u0410").test("\\u0410bb"), false)
    arktest.assertEQ(RegExp("\u0410").test("\u0410bb"), true)

    arktest.assertEQ(RegExp("\\1(a)").test("aa"), true)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExp.test")

    suite.addTest("RegExp.test", test)

    return suite.run()
}
