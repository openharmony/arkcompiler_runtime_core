/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Point class from documentation
tags: [compile-only]
---*/

class Point {
    public x: number
    public y: number
    public constructor(x : number, y : number) {
        this.x = x
        this.y = y
    }

    public distanceBetween(other: Point): number {
        return Math.sqrt (
            (this.x - other.x) * (this.x - other.x) +
            (this.y - other.y) * (this.y - other.y)
        )
    }

    static origin = new Point(0, 0)
}
