{% for mod in modifiers %}

/*---
desc: Hiding method with different access method.
assert: The access modifier of an overriding or hiding method must provide at least as much access as the overridden or hidden method, as follows.  If the overridden or hidden method is public, then the overriding or hiding method must be public; otherwise, a compile-time error occurs. If the overridden or hidden method is protected, then the overriding or hiding method must be protected or public; otherwise, a compile-time error occurs. If the overridden or hidden method has package access, then the overriding or hiding method must not be private; otherwise, a compile-time error occurs.
params: {{mod.m1}} and {{mod.m2}}
tags: [compile-only, negative]
---*/

class A {
  {{mod.m1}} m(): int {
    return 1;
  }
}

final class C extends A {
  {{mod.m2}} m(): int {
    return 5;
  }
}

{% endfor %}
