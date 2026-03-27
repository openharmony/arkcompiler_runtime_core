/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 function main(): int {
     const suite = new arktest.ArkTestsuite('Test RegExp');

     suite.addTest('Test 0', () => {
         let re = RegExp('\\u{0}', 'g')
         let test = '\u0000'
         arktest.assertTrue(re.test(test))
     })
     suite.addTest('Test 2', () => {
         let re = RegExp('\\u{00}', 'g')
         let test = '\u0000\u0000'
         arktest.assertTrue(re.test(test))
     })
     suite.addTest('Test 4', () => {
         let re = RegExp('\\u{0000}', 'g')
         let test = '\u0000\u0000\u0000\u0000'
         arktest.assertTrue(re.test(test))
     })
     suite.addTest('Test 8', () => {
         let re = RegExp('\\u{00000000}', 'g')
         let test = '\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000'
         arktest.assertTrue(re.test(test))
     })

     suite.addTest('Hello World Regexp Test', () => {
         let re = RegExp('\\u{48}\\u{65}\\u{6c}\\u{6c}\\u{6f}\\u{20}\\u{77}\\u{6f}\\u{72}\\u{6c}\\u{64}', 'g')
         let test = 'Hello world'
         arktest.assertTrue(re.test(test))
     })

     suite.addTest('Regexp Test', () => {
         let test = '\\u{10FFFF}'
         let re = RegExp(test, 'g')
         arktest.assertFalse(re.test(test))
         test = '\\u{110000}'
         arktest.expectThrow(() => { RegExp(test, 'g') }, (e) => e instanceof SyntaxError)
     })


     return suite.run();
 }
