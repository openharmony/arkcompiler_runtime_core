{% for fd in function_decls %}
/*---
desc: >-
    The ArkTS language allows specifying a function that can be called in
    different ways by writing overload signatures, i.e., by writing several
    function headers that have the same name but different signatures
    followed by one implementation function.
---*/

{{fd.decl}}

function main() {
    {{fd.call|indent}}
}
{% endfor %}
