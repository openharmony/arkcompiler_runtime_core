{% for fd in function_decls %}
/*---
desc: >-
  A function declaration introduces a named function, specifying
  its name, signature and body. Types of all parameters must be explicitly defined.
---*/

{{fd.decl}}

function main(): void {
    {{fd.call|indent}}
}

{% endfor %}
