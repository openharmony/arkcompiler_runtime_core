/*---
desc: Declare field with name used in superclass.
assert: If a class declares a field with a certain name, then the declaration of that field is said to hide any and all accessible declarations of field with the samename in superclasses and superinterfaces if the class.
---*/

class A {
  public x: int;
}

class B extends A{
  public x: boolean;
}

function main(): void {
    let instance: B = new B();

    assert instance.x == false;
}
