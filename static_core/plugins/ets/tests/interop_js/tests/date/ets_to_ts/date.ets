/**
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

package date.test;

const DATASTR = '2025-03-01T01:02:03.000Z';
const TIMESTAMP = 1609459200000;

let etsDate: Date;
let strDate: Date;
let stampDate: Date;
{
    etsDate = new Date(2025, 2, 1, 1, 2, 3, 4);
    strDate = new Date(DATASTR);
    stampDate = new Date(TIMESTAMP);
}
