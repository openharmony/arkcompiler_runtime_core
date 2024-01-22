/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

function main(): void {
  const result0 = Array.of<number>(5, 4, 3);
  assert result0.length == 3;
  assert (result0.$_get(0)) == 5;
  assert (result0.$_get(1)) == 4;
  assert (result0.$_get(2)) == 3;

  const result1 = new Array<number>();
  assert result1.length == 0;

  testCopyWithin();
  testFill();
  testRevSort();
  testSliceSplice();
}

function testCopyWithin(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5, 6)
  const result4 = init.copyWithin(0, 2, -2);
  assert result4.length == 6;
  assert (result4.$_get(0)) == 3;
  assert (result4.$_get(1)) == 4;
  assert (result4.$_get(2)) == 3;
  assert (result4.$_get(3)) == 4;
  assert (result4.$_get(4)) == 5;
  assert (result4.$_get(5)) == 6;
  assert init.length == 6;
  assert (init.$_get(0)) == 3;
  assert (init.$_get(1)) == 4;
  assert (init.$_get(2)) == 3;
  assert (init.$_get(3)) == 4;
  assert (init.$_get(4)) == 5;
  assert (init.$_get(5)) == 6;
}

function testFill(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5)

  assert init.length == 5;
  assert (init.$_get(0)) == 1;
  assert (init.$_get(1)) == 2;
  assert (init.$_get(2)) == 3;
  assert (init.$_get(3)) == 4;
  assert (init.$_get(4)) == 5;

  const result7 = init.fill(2, 2.0, 4.0);
  assert result7.length == 5;
  assert (result7.$_get(0)) == 1;
  assert (result7.$_get(1)) == 2;
  assert (result7.$_get(2)) == 2;
  assert (result7.$_get(3)) == 2;
  assert (result7.$_get(4)) == 5;

  const result8 = init.fill(1, 3.0, Infinity);
  assert result8.length == 5;
  assert (result8.$_get(0)) == 1;
  assert (result8.$_get(1)) == 2;
  assert (result8.$_get(2)) == 2;
  assert (result8.$_get(3)) == 1;
  assert (result8.$_get(4)) == 1;

  const result9 = init.fill(0, -Infinity, Infinity);
  assert result9.length == 5;
  assert (result9.$_get(0)) == 0;
  assert (result9.$_get(1)) == 0;
  assert (result9.$_get(2)) == 0;
  assert (result9.$_get(3)) == 0;
  assert (result9.$_get(4)) == 0;

  const result10 = init.fill(30, -2.0, -1.0);
  assert result10.length == 5;
  assert (result10.$_get(0)) == 0;
  assert (result10.$_get(1)) == 0;
  assert (result10.$_get(2)) == 0;
  assert (result10.$_get(3)) == 30;
  assert (result10.$_get(4)) == 0;

  assert init.length == 5;
  assert (init.$_get(0)) == 0;
  assert (init.$_get(1)) == 0;
  assert (init.$_get(2)) == 0;
  assert (init.$_get(3)) == 30;
  assert (init.$_get(4)) == 0;
}

function testRevSort(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5, 11, 21, 111)

  assert init.length == 8;
  assert (init.$_get(0)) == 1;
  assert (init.$_get(1)) == 2;
  assert (init.$_get(2)) == 3;
  assert (init.$_get(3)) == 4;
  assert (init.$_get(4)) == 5;
  assert (init.$_get(5)) == 11;
  assert (init.$_get(6)) == 21;
  assert (init.$_get(7)) == 111;

  const result13 = init.reverse();
  assert result13.length == 8;
  assert (result13.$_get(0)) == 111;
  assert (result13.$_get(1)) == 21;
  assert (result13.$_get(2)) == 11;
  assert (result13.$_get(3)) == 5;
  assert (result13.$_get(4)) == 4;
  assert (result13.$_get(5)) == 3;
  assert (result13.$_get(6)) == 2;
  assert (result13.$_get(7)) == 1;

  assert init.length == 8;
  assert (init.$_get(0)) == 111;
  assert (init.$_get(1)) == 21;
  assert (init.$_get(2)) == 11;
  assert (init.$_get(3)) == 5;
  assert (init.$_get(4)) == 4;
  assert (init.$_get(5)) == 3;
  assert (init.$_get(6)) == 2;
  assert (init.$_get(7)) == 1;

  const result15 = init.toReversed();
  assert result15.length == 8;
  assert (result15.$_get(0)) == 1;
  assert (result15.$_get(1)) == 2;
  assert (result15.$_get(2)) == 3;
  assert (result15.$_get(3)) == 4;
  assert (result15.$_get(4)) == 5;
  assert (result15.$_get(5)) == 11;
  assert (result15.$_get(6)) == 21;
  assert (result15.$_get(7)) == 111;

  assert init.length == 8;
  assert (init.$_get(0)) == 111;
  assert (init.$_get(1)) == 21;
  assert (init.$_get(2)) == 11;
  assert (init.$_get(3)) == 5;
  assert (init.$_get(4)) == 4;
  assert (init.$_get(5)) == 3;
  assert (init.$_get(6)) == 2;
  assert (init.$_get(7)) == 1;

  const result17 = init.toSorted();
  assert result17.length == 8;
  assert (result17.$_get(0)) == 1;
  assert (result17.$_get(1)) == 11;
  assert (result17.$_get(2)) == 111;
  assert (result17.$_get(3)) == 2;
  assert (result17.$_get(4)) == 21;
  assert (result17.$_get(5)) == 3;
  assert (result17.$_get(6)) == 4;
  assert (result17.$_get(7)) == 5;

  assert init.length == 8;
  assert (init.$_get(0)) == 111;
  assert (init.$_get(1)) == 21;
  assert (init.$_get(2)) == 11;
  assert (init.$_get(3)) == 5;
  assert (init.$_get(4)) == 4;
  assert (init.$_get(5)) == 3;
  assert (init.$_get(6)) == 2;
  assert (init.$_get(7)) == 1;

  const result19 = init.sort();
  assert result19.length == 8;
  assert (result19.$_get(0)) == 1;
  assert (result19.$_get(1)) == 11;
  assert (result19.$_get(2)) == 111;
  assert (result19.$_get(3)) == 2;
  assert (result19.$_get(4)) == 21;
  assert (result19.$_get(5)) == 3;
  assert (result19.$_get(6)) == 4;
  assert (result19.$_get(7)) == 5;

  assert init.length == 8;
  assert (init.$_get(0)) == 1;
  assert (init.$_get(1)) == 11;
  assert (init.$_get(2)) == 111;
  assert (init.$_get(3)) == 2;
  assert (init.$_get(4)) == 21;
  assert (init.$_get(5)) == 3;
  assert (init.$_get(6)) == 4;
  assert (init.$_get(7)) == 5;
}

function testSliceSplice(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5, 11, 21, 111)

  const result21 = init.slice(1, 3);
  assert result21.length == 2;
  assert (result21.$_get(0)) == 2;
  assert (result21.$_get(1)) == 3;

  assert init.length == 8;
  assert (init.$_get(0)) == 1;
  assert (init.$_get(1)) == 2;
  assert (init.$_get(2)) == 3;
  assert (init.$_get(3)) == 4;
  assert (init.$_get(4)) == 5;
  assert (init.$_get(5)) == 11;
  assert (init.$_get(6)) == 21;
  assert (init.$_get(7)) == 111;

  const result23 = init.splice(2, 100);
  assert result23.length == 6;
  assert (result23.$_get(0)) == 3;
  assert (result23.$_get(1)) == 4;
  assert (result23.$_get(2)) == 5;
  assert (result23.$_get(3)) == 11;
  assert (result23.$_get(4)) == 21;
  assert (result23.$_get(5)) == 111;

  assert init.length == 2;
  assert (init.$_get(0)) == 1;
  assert (init.$_get(1)) == 2;
}
