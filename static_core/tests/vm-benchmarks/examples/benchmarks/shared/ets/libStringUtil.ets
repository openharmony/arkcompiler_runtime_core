/**
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

export class StringUtil {

    x: int;
    y: int;

    constructor() {
        this.x = 25;
        this.y = 97;
    }

    randomString(size: int): string {
        let s = '';
        for (let i = 0; i < size; i++) {
            s += String.fromCharCode((this.x * Math.random()) + this.y);
        }
        return s;
    }

}
