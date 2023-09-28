/*---
desc: Multiple member inheritance.
assert: There might be several paths by which the same member type declaration is inherited from an interface. In such a situation, the member type is considered to be inherited only once.
tags: [compile-only]
---*/

interface A {
  class C {
    public static bar(): int {
      return 10;
    }
  }
}

class B {
  class C {
    public static bar(): int {
      return 5;
    }
  }
}

class D extends B implements A {
}
