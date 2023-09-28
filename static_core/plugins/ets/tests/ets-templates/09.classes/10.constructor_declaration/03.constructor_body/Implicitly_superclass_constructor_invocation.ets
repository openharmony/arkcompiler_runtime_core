/*---
desc: Check that superclass construct implicitly invoced.
assert: If a constructor body does not begin with an explicit constructor call and the constructor being declared is not part of the primordial class Object, then the constructor body implicitly begins with a superclass constructor call "super();", a call of the constructor of its direct superclass that takes no arguments.
---*/

class A {
  public x: int;

  public constructor() {
    this.x = 5;
  }
}

class B extends A {
  public constructor() {

  }
}

function main(): void {
  let instance: B = new B();
  assert instance.x == 5;
}
