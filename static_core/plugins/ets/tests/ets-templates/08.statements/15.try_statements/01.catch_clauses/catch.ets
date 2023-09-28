{% for t in cases %}
/*---
desc: A catch clause contains a block that corresponds to the exception parameter and handles exceptions.
---*/

{{t.defs}}

function main(): int {
    {{t.use|indent}}
}
{% endfor %}
