{% for case in cases %}
/*---
desc: String literals consist of zero or more characters enclosed between single or double quotes.
tags: [negative, compile-only]
---*/

function main() {
    let s: string = {{case}}
    assert s instanceof Object
}
{% endfor %}
