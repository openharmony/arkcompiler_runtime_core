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

export final class UnionTypeInfo extends TypeInfo {
    private types_: FixedArray<Class>

    private constructor(type: Class) {
        super(type)
    }

    /**
     * Gets all possible types that this union type represents
     *
     * @returns An immutable array of {@link Class} types that comprise this union
     */
    public getTypes(): FixedArray<Class> {
        return this.types_
    }
}

} // namespace reflect
