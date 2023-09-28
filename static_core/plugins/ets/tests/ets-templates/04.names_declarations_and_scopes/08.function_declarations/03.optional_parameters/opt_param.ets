{% for c in cases %}
/*---
desc: Two forms of optional parameters.
---*/

{{c.decl}}

function main() {
    {{c.call|indent}}
}
{% endfor %}
