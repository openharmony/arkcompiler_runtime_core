{% for mod in modifiers %}

/*---
desc: Override method declaration with keyword {{mod}}.
assert: It is compile-time error if a method declaration that contains the keyword override also contains any one of the keywords abstract, static, open, native, or synchronized.
params: {{mod}}
tags: [compile-only, negative]
---*/

class A {
  foo(): int {
    return 1;
  }
}

final class B extends A {
  override {{mod}} foo(): int {
    retrun 5;
  }
}

{% endfor %}
