{% for im in interface_members %}
/*---
desc: Type declaration inside interface.
assert: Interfaces may contain member type declarations.
---*/

interface A {
  {{im.decl}}
}

{{im.call}}
{% endfor %}