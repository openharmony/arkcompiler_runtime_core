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

import {
    InterfaceClass,
    createInterfaceClassFromTs,
    instanceInterfaceClass,
} from 'main_js';

function checkInterfaceClassMethod(): boolean {
    const TSClass = new InterfaceClass();

    return TSClass.compare(TSClass.get) as boolean;
}

function checkCreateInterfaceClassFromTs(): boolean {
    const TSClass = createInterfaceClassFromTs();

    return TSClass.compare(TSClass.get) as boolean;
}


function checkInstanceInterfaceClass(): boolean {
    const TSClass = instanceInterfaceClass;

    return TSClass.compare(TSClass.get) as boolean;
}
