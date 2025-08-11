/**
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

const F1: string = 'Hello';

class C1 {
  public C1F1: string = 'public';
  protected C1F2: string = 'protected';
  private C1F3: string = 'private';
  public static C1F4: string = 'static';
}

interface I1 {
  I1F1: number;
  readonly I1F2: string;
}

enum E1 {
  ONE
}

namespace NS1 {
  const NS1F1: string[] = ['namespace'];
}
