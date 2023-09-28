/*---
desc: Super method call. 
---*/

class Rectangle {
  protected height: int = 0;
  protected width: int = 0;

  public constructor (h: int, w: int) {
    this.height = h;
    this.width = w;
  }

  public draw(): boolean {
    return true;
  }
}

class FilledRectangle extends Rectangle {
  color: String = "";

  constructor (h: int, w: int, c: String) {
    super(h, w);
    this.color = c;
  }

  override draw(): boolean {
    return super.draw();
  }
}

function main(): void {
  let instance: FilledRectangle = new FilledRectangle(5, 5, "color");

  assert instance.draw() == true;
}
