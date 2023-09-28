{% for mod in modifiers %}

/*---
desc: Abstract method declaraion with '{{mod}}' keyword.
assert: It is compile-time error if a method declaration that contains the keyword abstract also contains any one of the keywords static, open, override, native, or synchronized.
params: {{mod}}
tags: [compile-only, negative]
---*/

abstract class A {
  {{mod}} abstract run(): void;
}

{% endfor %}