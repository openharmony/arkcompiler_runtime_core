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
desc: Class implements multiple interface methods.
---*/

interface Reader {
  read(file: string): string;
}

interface Writer {
  write(content: string, file: string): string;
}

interface Deleter {
  delete(file: string): boolean;
}

class FileManager implements Reader, Writer, Deleter {
  read(file: string) {
  return `Reading content from ${file}`;
  }

  write(content: string, file: string) {
  return`Writing ${content} to ${file}`;
  }
  
  delete(file: string) {
  return true;
  }
}

function main(): void {
  const fm = new FileManager();
  const result = fm.read('test.txt');
  const result1 = fm.write('Hello, world!', 'test.txt');
  const result2 = fm.delete('Our memories');
  assertEQ(result, "Reading content from test.txt");
  assertEQ(result1, "Writing Hello, world! to test.txt");
  assertEQ(result2, true);
}