{% for v in values %}
/*---
desc: check bitwise AND operation for unsigned int operands
---*/

function main(): void {
    const a: uint = {{v.left}}
    const b: uint = {{v.right}}
    assert (a & b) == {{v.result}}
}

{% endfor %}
