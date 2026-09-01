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

/*---
desc: "TEST-UNIT-11-0014 GAP-11-0014 GRC-11-0014 CI-11-00-008-02: String enum value implicitly converts to string"
---*/
enum Message: string {
  Hello = "hello",
  Goodbye = "goodbye"
}

function main(): void {
  let msg: Message = Message.Hello
  let str: string = msg  // Implicit conversion
  arktest.assertEQ(str, "hello")
}
