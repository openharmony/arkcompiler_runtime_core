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
desc: After interface inheritance, the class implements multiple interfaces.
---*/

interface Shape {
  getArea(): number;
}

interface Colorful {
  getColor(): string;
}

interface ColorfulShape extends Shape, Colorful {}

class Circle implements ColorfulShape {
  private radius: number;
  private color: string;

  constructor(radius: number, color: string) {
    this.radius = radius;
    this.color = color;
  }

  getArea() {
    return Math.PI * this.radius * this.radius;
  }

  getColor() {
    return this.color;
  }
}

function main(): void {
  const circle = new Circle(5, 'red');
  const result = circle.getArea();
  const result1 = circle.getColor();
  assertEQ(result, 78.53981633974483);
  assertEQ(result1, "red");
}