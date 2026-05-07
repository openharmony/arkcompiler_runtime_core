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

function moduleFunction(): int {
    return 11;
}

function overloaded(): int {
    return 1;
}

function overloaded(value: int): int {
    return value;
}

export let moduleValue: int = 17;

export class NotModule {
    public static staticFunction(): int {
        return 31;
    }

    public static value: int = 37;
}

namespace Ops {
    export function namespaceFunction(): int {
        return 23;
    }

    export function overloaded(): int {
        return 2;
    }

    export function overloaded(value: int): int {
        return value;
    }

    export let namespaceValue: int = 29;
}
