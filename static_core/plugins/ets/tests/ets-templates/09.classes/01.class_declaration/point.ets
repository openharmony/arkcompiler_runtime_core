/*---
desc: Point class from documentation
tags: [compile-only]
---*/

class Point {
  public x: double, y: double;
  public length(): double {
    return (this.x * this.x + this.y * this.y);
  }

  constructor(x: int, y: int) {
    this.x = x;
    this.y = y;
  }

  static origin = new Point(0, 0);
}
