/*---
desc: A compile-time error occurs if the qualified this expression occurs in a class or interface which is not class T or an inner class of class T.
tags: [compile-only, negative]
---*/

class T {}

class A {
  public foo(): T { 
    return T.this();
  }
}