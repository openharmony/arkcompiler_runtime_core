{% for v in values %}
/*---
desc: check bitwise OR of two short integers
---*/

function main(): void {
    const a: short = {{v.left}}
    const b: short = {{v.right}}
    assert (a | b) == {{v.result}}
}

{% endfor %}
