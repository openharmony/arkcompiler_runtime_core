{% for v in values %}
/*---
desc: check bitwise XOR of two unsigned bytes
---*/

function main(): void {
    const a: ubyte = {{v.left}}
    const b: ubyte = {{v.right}}
    assert (a ^ b) == {{v.result}}
}

{% endfor %}
