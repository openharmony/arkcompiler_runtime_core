{% for v in values %}
/*---
desc: check bitwise AND operation for char operands
---*/

function main(): void {
    const a: char = {{v.left}}
    const b: char = {{v.right}}
    assert (a & b) == {{v.result}}
}

{% endfor %}
