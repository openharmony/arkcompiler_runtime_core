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
import * as huks from "../author_generated/huks";

loadLibrary("ani_huks");

function main() {
    console.log("hello main huks");
    let numbers1: byte[] = [1, 2, 3, 4, 5];
    let arrbuf1: ArrayBuffer = new ArrayBuffer(numbers1.length);
    for (let i = 0; i < numbers1.length; i++) {
        arrbuf1.set(i, numbers1[i]);
    }
    let numbers2: byte[] = [3, 4, 5, 6, 7];
    let arrbuf2: ArrayBuffer = new ArrayBuffer(numbers2.length);
    for (let i = 0; i < numbers2.length; i++) {
        arrbuf2.set(i, numbers2[i]);
    }
    const huksParam1: huks.HuksParam = {
        tag: huks.HuksTag.HUKS_TAG_KEY_SIZE,
        value: huks.HuksKeySize.HUKS_RSA_KEY_SIZE_512 as number
    };
    const huksParam2:
        huks.HuksParam = {tag: huks.HuksTag.HUKS_TAG_KEY, value: arrbuf1};
    const huksOptions: huks.HuksOptions = {
        properties: [huksParam1, huksParam2],
        inData: arrbuf2
    };

    console.log(huksOptions)

    let huksResult = huks.generateKey("alias", huksOptions);

    console.log(huksResult)
}
