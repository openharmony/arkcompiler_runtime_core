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

package std.core;

export namespace reflect {

/**
 * Represents static field of class
 *
 */
export final class StaticField extends Field {
    private constructor() { throw new Error("StaticField constructor called") }

    public equals(other: StaticField): boolean {
        throw new Error("Not implemeted yet")
    }

    public getValue(): Any {
        throw new Error("Not implemeted yet")
    }

    public setValue(v: Any) {
        throw new Error("Not implemeted yet")
    }
}

} // namespace reflect
