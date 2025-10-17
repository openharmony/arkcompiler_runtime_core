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

/*---
desc: Test cases for various usage scenarios of asynchronous method.
      Using promise.allsettled in asynchronous methods.
---*/     

class MethodWithAllSettled {
    async methodWithAllSettled(): Promise<string>{
        const results = await Promise.allSettled([
            Promise.resolve("Resolved!"),
            Promise.reject(new Error("Rejected!")),
        ]);
        return JSON.stringify(results);
    }
}

async function startTest() {
    const example = new MethodWithAllSettled();
    const result = await example.methodWithAllSettled();

    const jsonObj = JSON.parseJsonElement(result)
    arktest.assertEQ(jsonObj.jsonType, jsonx.JsonType.JsonArray, "Root jsonElement type is not instanceof JsonArray");

    const rootObj = jsonObj.asArray()
    arktest.assertTrue(Array.isArray(rootObj), "Root json-value is not instanceof Array");
    arktest.assertEQ(rootObj.length, 2)


    const el1 = rootObj[0]
    const el2 = rootObj[1]

    arktest.assertEQ(el1.getString("status"), 'fulfilled');
    arktest.assertEQ(el2.getString("status"), 'rejected');
    arktest.assertEQ(el1.getString("value"), 'Resolved!');
    const el2_reason = el2.getElement("reason")
    arktest.assertEQ(el2_reason.jsonType, jsonx.JsonType.JsonObject, "jsonType of 'reason' is not instanceof JsonObject");
    arktest.assertEQ(el2_reason.getString("message_"), "Rejected!");
    arktest.assertEQ(el2_reason.getString("name_"), "Error");

}

function main(): void {
    waitForCompletion(startTest);
}