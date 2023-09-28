/*---
desc: Default class constructor.
assert: If a class contains no constructor declarations, then a default constructor is implicitly declared. The semantics of such constructor is to provide default values to class fields which have default values.
---*/

class A {
  public x: int;
  public b: B;
}

class B {

}

function main(): void {
  let instance: A = new A();

  assert instance.x == 0;
  assert instance.b == null;
}
