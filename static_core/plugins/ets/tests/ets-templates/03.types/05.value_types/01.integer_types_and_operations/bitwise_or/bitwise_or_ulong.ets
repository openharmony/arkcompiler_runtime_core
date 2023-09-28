{% for v in values %}
/*---
desc: check bitwise OR of two unsigned long integers
---*/

function main(): void {
    const a: ulong = {{v.left}}
    const b: ulong = {{v.right}}
    assert (a | b) == {{v.result}}
}

{% endfor %}
