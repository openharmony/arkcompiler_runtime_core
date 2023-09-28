{% for mod in modifiers %}

/*---
desc: Native method declaraion with '{{mod}}' keyword.
assert: It is compile-time error if a method declaration that contains the keyword native also contains any one of the keywords abstract, open, override, native, or synchronized.
tags: [compile-only, negative]
---*/

class A {
  {{mod}} native run(): void;
}

{% endfor %}