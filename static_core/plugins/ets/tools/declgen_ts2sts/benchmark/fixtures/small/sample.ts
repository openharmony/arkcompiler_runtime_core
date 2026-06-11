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

// Sample small fixture used by benchmark/scenarios/declgen.bench.ts.
// Keep this file small and self-contained.

export interface Point {
  x: number;
  y: number;
}

export class Shape<T extends Point> {
  constructor(public readonly origin: T) {}

  translate(dx: number, dy: number): Shape<T> {
    return new Shape({ ...this.origin, x: this.origin.x + dx, y: this.origin.y + dy });
  }
}

export function distance(a: Point, b: Point): number {
  const dx = a.x - b.x;
  const dy = a.y - b.y;
  return Math.sqrt(dx * dx + dy * dy);
}
