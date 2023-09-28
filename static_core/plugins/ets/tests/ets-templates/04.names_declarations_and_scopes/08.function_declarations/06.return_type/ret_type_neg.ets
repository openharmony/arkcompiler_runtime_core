{% for fd in function_decls %}
/*---
desc: >-
  An omitted function or method return type can be inferred from the function or the method body.
tags: [compile-only, negative]
---*/

{{fd.decl}}

function main() {
    {{fd.call|indent}}
}
{% endfor %}
