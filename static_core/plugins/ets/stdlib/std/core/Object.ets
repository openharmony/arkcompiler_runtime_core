/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

/**
 * Common ancestor amongst all other classes
 */
export class Object {

  /**
   * Constructs a new blank Object
   */
  constructor () {};

  /**
   * Converts this object to a string
   *
   * @returns result of the conversion
   */
  public toString(): String {
    return Type.of(this).toString();
  }

  /**
   * Returns a hash code (integer representation) for this instance
   *
   * @returns representation of this instance
   */
  public hashCode(): int {
    return runtime.GetHashCode(this);
  }

  /**
   * Performs default reference equality comparison
   *
   * @param to object to be checked against
   *
   * @returns true if provided object and this instance have same references, false otherwise
   */
  public equals(to: Object|null): boolean {
    return runtime.equals(this, to);
  }
}

export type NullishType = Object | null
