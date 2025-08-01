/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
 * Strores information about stacktrace and cause in case of an exception.
 * Serves as a base class for all exception classes.
 */
export class Exception {
  private stackLines: FixedArray<String> = [];
  message: String;
  private cause: Object;
  private provisionStackTrace(): void {
    this.stackLines = stackTraceLines();
  }

  /**
   * Constructs a new empty exception instance
   */
  constructor () {
    this.message = "";
    this.cause = this;
    this.provisionStackTrace();
  }

  /**
   * Constructs a new exception instance with provided message
   *
   * @param msg message of the exception
   */
  constructor(msg: String) {
    this.message = msg;
    this.cause = this;
    this.provisionStackTrace()
  }

  /**
   * Constructs a new exception instance with provided message and cause
   *
   * @param msg message of the exception
   *
   * @param cause cause of the exception
   */
  constructor(msg: String, cause: Object) {
    this.message = msg;
    this.cause = cause;
    this.provisionStackTrace();
  }

  /**
    * Converts this excepiton to a string
    * Result includes exception message and the stacktrace
    *
    * @returns result of the conversion
    */
  override toString(): String {
    let s: String = "";

    if (this.message != "") {
      s += this.message + "\n";
    }

    for (let i: int = (this.stackLines.length > 2 ? 2 : 0); i < this.stackLines.length; i++) {
      s += this.stackLines[i];
      if (i != this.stackLines.length-1) {
        s += "\n";
      }
    }

    return s;
  }

  /**
   * Returns the cause of this excepiton
   *
   * @returns the cause
   */
  public getCause(): Object | null {
    if (this.cause == this) {
      return null;
    } else {
      return this.cause;
    }
  }
}
