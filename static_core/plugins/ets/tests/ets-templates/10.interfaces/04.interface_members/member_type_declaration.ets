{% for im in interface_members %}
/*---
desc: Type declaration inside interface.
assert: Interfaces may contain member type declarations.
tags: [compile-only]
---*/

interface A {
  {{im}}
}
{% endfor %}