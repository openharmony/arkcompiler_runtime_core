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

/*---
desc: More interfaces and complex methods(boolean type).
---*/

interface Reader {
  read(file: boolean): boolean;
}

interface Writer {
  write(content: boolean): boolean;
}

class FileManager implements Reader, Writer {
  read(file: boolean): boolean {
    return file;
  }

  write(content: boolean): boolean {
    return content;
  }
}

function main(): void {
  const fm = new FileManager();
  const result = fm.read(true)
  const result1 = fm.write(false);
  assertEQ(result, true);
  assertEQ(result1, false);
}