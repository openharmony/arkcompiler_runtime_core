/**
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

let uri: String = "https://ru.wikipedia.org/wiki/Заглавная_страница";
let line = "https://ru.wikipedia.org/wiki/tail";

function checker(lineString: String): boolean {
    let must_be_compressed_line_string = uri.slice(0, 30);  // the specified range contains compressible chars only
    let sb: StringBuilder = new StringBuilder(must_be_compressed_line_string);
    if (sb.append("tail").toString() == lineString) {
        return true;
    }
    return false;
}

function main(): int {
    for (let i = 0; i < 1000; i++) {
        arktest.assertEQ(checker(line), true);
    }
    return 0;
}
