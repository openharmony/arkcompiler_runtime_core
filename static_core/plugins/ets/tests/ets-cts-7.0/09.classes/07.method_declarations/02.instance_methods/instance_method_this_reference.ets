/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
    An instance method is always called with respect to an object that becomes
    the current object which the keyword this refers to during method execution.
---*/

class Point {
  public x: number
  public y: number
  public constructor(x: number, y: number) {
    this.x = x
    this.y = y
  }
  public distanceBetween(other: Point): number {
    return Math.sqrt(
      (this.x - other.x) * (this.x - other.x) +
      (this.y - other.y) * (this.y - other.y)
    )
  }
}

function main(): void {
  let p1: Point = new Point(0, 0);
  let p2: Point = new Point(3, 4);

  // When called on p1, 'this' refers to p1
  let dist1: number = p1.distanceBetween(p2);
  arktest.assertEQ(dist1, 5);

  // When called on p2, 'this' refers to p2
  let dist2: number = p2.distanceBetween(p1);
  arktest.assertEQ(dist2, 5);
}
