{% for c in cases %}
/*---
desc: In a 'for-of' statement type 'expr' must be either array or string.
---*/

let res = ""
{{c.decl}}

function main(): int {
    res = ""
    {{c.use|indent}}
    return 1;
}
{% endfor %}
