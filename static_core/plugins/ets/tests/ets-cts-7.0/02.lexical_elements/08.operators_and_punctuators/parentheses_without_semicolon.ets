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

{% for semicolon in ["", ";"] %}

/*---
desc: >-
    Some specific cases when statements are unambiguously split without semicolon.
    Punctuators are tokens that separate, complete, or otherwise organize program
    elements and parts. Punctuators include commas, semicolons, parentheses, square brackets, etc.
---*/

function main(): void {
  let testsuite = new arktest.ArkTestsuite("example")
  let b: int = 0

  testsuite.addTest("1", () => {
      b = 1
    }){{semicolon}}testsuite.run()
  arktest.assertEQ(b, 1, "expect 1")
}
{% endfor %}
