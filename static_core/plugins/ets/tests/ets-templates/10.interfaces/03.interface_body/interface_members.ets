{% for im in interface_members %}
/*---
desc: Interface members.
assert: The body of an interface may declare members of the interface, that is, fields, methods, classes, interfaces, and enumerations.
tags: [compile-only]
---*/

interface A {
  {{im}}
}

{% endfor %}