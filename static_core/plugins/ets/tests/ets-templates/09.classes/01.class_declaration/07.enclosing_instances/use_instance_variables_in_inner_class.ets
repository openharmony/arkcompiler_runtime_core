{% for member in class_members %}

/*---
desc: Use variables that not declare in an inner class, and don't have readonly modifier.
assert: Any local variable, formal parameter, or exception and panic parameter used but not declared in an inner class must be declared readonly, or a compile-time error occurs where the use is attempted.
tags: [compile-only, negative]
---*/

class ZeroDivisor extends Exception { }

class A {
  {{member.decl}};

  public inner class B {
    public foo(): void {
      {{member.call}};
    }

    public foo(y: int): void {
      let b: int = y;
    }
  }
}

{% endfor %}
