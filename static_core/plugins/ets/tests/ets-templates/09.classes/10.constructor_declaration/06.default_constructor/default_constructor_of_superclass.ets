/*---
desc: Default class constructor invoke default construtor of superclass.
assert: If the class being declared is the primordial class Object, then the default constructor has an empty body. Otherwise, the default constructor simply invokes the superclass constructor with no arguments.
---*/

class A {
  public x: int;
  public b: B;
}

class B extends A{

}

function main(): void {
  let instance: B = new B();

  assert instance.x == 0;
  assert instance.b == null;
}
