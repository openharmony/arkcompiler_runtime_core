{% for fd in function_decls %}
/*---
desc: >-
  A function declaration introduces a named function, specifying
  its name, signature and body. Types of all parameters must be explicitly defined.
tags: [compile-only, negative]
---*/

{{fd.decl}}

function main() {
    {{fd.call}}
}

{% endfor %}
