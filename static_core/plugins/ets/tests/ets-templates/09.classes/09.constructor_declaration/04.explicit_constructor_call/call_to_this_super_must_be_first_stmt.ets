{% for stmt in statements %}

/*---
desc: Negative tests for different set of statements before calling super()/this()
assert: Call to 'super'/'this' must be first statement in constructor
params: {{stmt}}
tags: [compile-only, negative]
---*/

class A {

    protected parentField: int;

    protected parentMethod() {

    }

    public constructor() {

    }

    public constructor(value: String) {

    }

}

class B extends A {
    childField1: int
    childField2: String

    childMethod1() : int {
        return this.childField1;
    }

    childMethod2() : String {
        return this.childField2;
    }

    public constructor(value: String) {

    }

    public constructor() {
        {{stmt}}
    }
}

{% endfor %}
