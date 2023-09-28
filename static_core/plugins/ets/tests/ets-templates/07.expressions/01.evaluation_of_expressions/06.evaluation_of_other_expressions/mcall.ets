/*---
desc: >-
    The choice of the method of o.m(...)call depends on the methods
    comprising the class or interface of the o type.
---*/

interface I {
    meth(s: String): String;
}

class A implements I {
  public override meth(s: String): String {
      return s + "A";
  }
}

class B extends A {
  public override meth(s: String): String {
      return super.meth(s) + "B";
  }
}

function main(): int {
    let a: A = new A();
    if (!a.meth("X").equals("XA")) return 1;

    a = new B();
    if (!a.meth("Y").equals("YAB")) return 1;

    let i: I = new A();
    if (!i.meth("Z").equals("ZA")) return 1;

    i = new B();
    if (!i.meth("W").equals("WAB")) return 1;

    return 0;
}
