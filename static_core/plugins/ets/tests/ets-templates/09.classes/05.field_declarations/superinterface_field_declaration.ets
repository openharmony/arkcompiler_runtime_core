/*---
desc: Declare field with name used in superinterface.
assert: If a class declares a field with a certain name, then the declaration of that field is said to hide any and all accessible declarations of field with the same name in superclasses and superinterfaces if the class.
---*/

interface A {
  x(): boolean {
    return true;
  }
}

class B implements A {
  public x: boolean;

  public getX(): boolean {
    return x();
  }
}

function main(): void {
    let instance: B = new B();

    assert instance.getX() == false;
}

