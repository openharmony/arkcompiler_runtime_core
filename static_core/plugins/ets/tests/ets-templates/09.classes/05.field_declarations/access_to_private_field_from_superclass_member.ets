/*---
desc: A private field from of a superclass access, if both classes are members of same class.
assert: A private field of a superclass might be accessible to a subclass - for example, if both classes are members of the same class.
---*/

class A {
  inner class C {
    private static x: int;
  }

  inner class B extends C {
    public getX(): int {
      return C.x;
    }
  }

  public getValue(): int {
    return new B.getX();
  }
}



function main(): void {
    let instance: A = new A();

    assert instance.getValue() == 0;
}
