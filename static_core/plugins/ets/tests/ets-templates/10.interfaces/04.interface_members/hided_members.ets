{% for im in interface_members %}
/*---
desc: Hide inheritance members.
assert: The interface inherits, from the interfaces it extends, all members of those interfaces, except for fields, classes, interfaces, and enumerations that it hides.
tags: [compile-only]
---*/

interface A {
  {{im}}
}

interface B extends A {
  {{im}}
}

{% endfor %}