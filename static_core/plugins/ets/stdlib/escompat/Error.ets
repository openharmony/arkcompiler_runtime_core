/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

package escompat;

/**
 * Strores information about stacktrace and cause in case of an error.
 * Serves as a base class for all error classes.
 */
export class Error {
  public stack: String[];
  public cause: Object;
  public message: String;
  public name: String;

  public fileName: String;
  public lineNumber: int;
  public columnNumber: int;

  private provisionStackTrace(): void {
    this.stack = stackTraceLines();
  }

  /**
   * Constructs a new empty error instance
   */
  constructor () {
    this.message = "";
    this.cause = this;
    this.name = "Error";
    this.fileName = "";
    this.lineNumber = 0;
    this.columnNumber = 0;
    this.provisionStackTrace();
  }

  /**
   * Constructs a new error instance with provided message
   *
   * @param msg message of the error
   */
  constructor(msg: String) {
    this.message = msg;
    this.cause = this;
    this.name = "Error";
    this.provisionStackTrace()
  }

  /**
   * Constructs a new error instance with provided message and cause
   *
   * @param msg message of the error
   *
   * @param cause cause of the error
   */
  constructor(msg: String, cause: Object) {
    this.message = msg;
    this.cause = cause;
    this.name = "Error";
    this.provisionStackTrace();
  }

  /**
    * Converts this error to a string
    * Result includes error message and the stacktrace
    *
    * @returns result of the conversion
    */
  override toString(): String {
    let s: String = "";

    if (this.message != "") {
      s += this.name + ": " + this.message + "\n";
    }

    for (let i: int = (this.stack.length > 2 ? 2 : 0); i < this.stack.length; i++) {
      s += this.stack[i];
      if (i != this.stack.length - 1) {
        s += "\n";
      }
    }

    return s;
  }

  // TODO(nsizov): Uncomment and rewrite when proper getters and setters will be implemented

  /**
   * Returns the cause of this error
   *
   * @returns the cause
   */
  // public getCause(): Object | null {
  //   if (this.cause == this) {
  //     return null;
  //   } else {
  //     return this.cause;
  //   }
  // }

  // TODO(nsizov): Uncomment and rewrite when proper getters and setters will be implemented

  /**
   * Returns the message of this error
   *
   * @returns String - the message
   */
  // public getMessage(): String {
  //   return this.msg;
  // }
}
