{% for mod in modifiers %}
{% for member in members %}
/*---
desc: Access modifiers.
assert: Members and constructors may have either private, internal, internal protected, protected or public modifier to explicitly specify the desired accessibility.
params: "{{mod}} {{member}}"
tags: [compile-only]
---*/

class A {
  {{mod}} {{member}}
}

{% endfor %}
{% endfor %}
