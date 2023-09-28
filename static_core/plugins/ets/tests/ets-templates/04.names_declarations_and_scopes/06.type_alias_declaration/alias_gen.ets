{% for t in cases %}
/*---
desc: >-
    Type aliases provide names for anonymous types (array, function or nullable types)
    or alternative names for existing types. Type aliases allow one to use meaningful
    and concise notation.
---*/
{{t.defs}}

function main(): int {
    {{t.use|indent}}
    if (!ok) {
        return 1;
    }
    return 0;
}
{% endfor %}
