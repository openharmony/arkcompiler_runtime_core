/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

/**
 * @Import { StringUtil } from '../../shared/libStringUtil.ts'
 * @Import { dummy } from '../moreshared/libDummy.ts'
 */

/**
 * @State
 * @Tags shared
 */
class B1 {

  /**
   * @Param 128
   */
    stringSize: number;
  /**
   * @Param 100
   */
    loopCount: number;
    startString: string = '';
    concatString: string = 'Ab';

  /**
   * @Setup
   */
    setup(): void {
        let util = new StringUtil();
        this.startString = util.randomString(this.stringSize);
        this.concatString = dummy();
    }

  /**
   * @Benchmark
   * @Tags alsostrings, simple
   */
    public test(): string {
        let result: string = this.startString;
        for (let i = 0; i < this.loopCount; i++) {
            result = result.concat(this.concatString);
        }
        return result.substring(this.loopCount / 2, this.loopCount);
    }

}
