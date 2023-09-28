{% for mod in modifiers %}

/*---
desc: Final method declaraion with '{{mod}}' keyword.
assert: A compile-time error occurs if a method declaration contains keyword abstract or static along with the keyword final.
params: {{mod}}
tags: [compile-only, negative]
---*/

class A {
  {{mod}} final run(): void;
}

{% endfor %}