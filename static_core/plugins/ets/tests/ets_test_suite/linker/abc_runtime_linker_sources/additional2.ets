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

import { AdditionalClass1 } from "./additional1";
import { additionalFunction3 } from "./additional3";

export class AdditionalClass2 {
    public static sideEffect = (() => { return AdditionalClass2.test(); })();

    public static test(): number {
        // Use class which is not cached during initialization
        const map = new Map<AdditionalClass1, string>();
        map.set(new AdditionalClass1(), "AdditionalClass1");
        const size = additionalFunction3(map);
        assertEQ(size, 1);
        return 1;
    }
}
