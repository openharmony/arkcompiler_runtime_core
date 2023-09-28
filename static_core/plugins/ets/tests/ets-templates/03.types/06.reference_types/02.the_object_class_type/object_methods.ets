{% for method in methods %}

/*---
desc: Inheritance methods from Object class.
params: {{method}}
---*/

class A {}

function main(): void {
    let a: A = new A()
    a.{{method}}
}

{% endfor %}
