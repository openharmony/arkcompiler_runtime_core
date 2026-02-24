/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import * as all from "std/math";

/**
 * @State
 * @Bugs 0000
 * @Tags common
 */
export class ArraySort {
  /**
   * @Param 100
   */
  size: int;

  /**
   * Specifics of the array to be sorted.
   * @Param "sorted", "sorted_reversed"
   */
  distribution: String = "sorted";

  ints: FixedArray<int> = [];
  intsInitial: FixedArray<int> = [];

  /**
   * Prepare array depending on the distribution variant.
   * @Setup
   */
  public prepareArray(): void {
    this.ints = new FixedArray<int>(this.size);
    this.intsInitial = new FixedArray<int>(this.size);
    let size = this.size
    switch (this.distribution) {
      case "sorted":
        for (let i = 0; i < size; i++) {
          this.intsInitial[i] = i;
        }
        break;
      case "sorted_reversed":
        for (let i = 0; i < size; i++) {
          this.intsInitial[i] = size - i;
        }
        break;
      default:
        break;
    }
  }

  /**
   * @Benchmark
   * @Bugs sortbug
   * @Tags sorttag
   */
  public sort(): void {
    copyTo(this.intsInitial, this.ints, 0, 0, this.size);
    sort(this.ints);
  }

  /**
   * @Benchmark -wi 0 -mi 1
   */
  public baseline(): void {
    copyTo(this.intsInitial, this.ints, 0, 0, this.size);
  }
}
