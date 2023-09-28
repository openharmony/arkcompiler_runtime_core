/*---
desc: Static field declaration.
assert: Instance is created for every instance variable declared in that class or any of its superclasses.
---*/

class A {
  public static x: int;
}

class B extends A {
  public static y: int;

  public static getX(): int {
    return A.x;
  }

  public static setX(value: int): void {
    A.x = value;
  }
}

function main(): void {
  B.y = 10;
  B.setX(5);

  assert A.x == 5;

  assert B.getX() == 5;
  assert B.y == 10;

  B.y++;
  A.x++;

  assert A.x == 6;

  assert B.getX() == 6;
  assert B.y == 11;
}
