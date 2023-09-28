{% for mod in modifiers %}

/*---
desc: Static method declaraion with '{{mod}}' keyword.
assert: It is compile-time error if a method declaration that contains the keyword static also contains any one of the keywords abstract, open, override, or synchronized.
params: {{mod}}
tags: [compile-only, negative]
---*/

class A {
  {{mod}} abstract run(): void;
}

{% endfor %}