/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Data Types
var name = "Alice";
let age = 25;
const pi = 3.14;
let isSame = true;
let u = undefined;
let n = null;
const mySymbol = Symbol();
let val1 = 10n;
let obj = {
    mySymbol : "prop",
};
let arr = [1, 2, 3];
//Function Type
var sum = function(num1, num2) {
    return num1 + num2;
};
sum(1, 2);
// Date Type
let now = new Date();

console.log(name, age, pi, u, n, mySymbol, val1, obj, arr[1]);

// if Statement
if (age >= 18) {
    console.log("Adult");
} else {
    console.log("Minor");
}

// Switch Statement
let day = "Monday";
switch (day) {
    case "Monday":
      console.log("Start of the week");
      break;
    case "Friday":
      console.log("End of the week");
      break;
    default:
      console.log("Midweek");
}

// For Statement
for (let i = 0; i < 5; i++) {
    console.log(i);
}

// While Statement
let i = 0;
while (i < 5) {
  console.log(i);
  i++;
}

do {
    console.log(i);
    i++;
} while (i < 5);

// Arrow Function
const greet = (name) => {
    console.log("Hello, " + name);
};
greet("js");

// Object
const person = {
    name: "Alice",
    age: 25,
    greet() {
      console.log("Hello, " + this.name);
    }
};
person.greet();

// Template string
const temp = "Js";
const greeting = `Hello, ${temp}!`;

// Destructuring assignment
const student = { gender: "男", grade: "2"};
const { gender, grade } = person;

// Default parameters
function defaultGreet(name = "js") {
    console.log("Hello, " + name);
}
defaultGreet();

// Extended operator
const arr1 = [1, 2, 3];
const arr2 = [...arr1, 4, 5]; 

// Class
class Person {
    constructor(name, age) {
      this.name = name;
      this.age = age;
    }
   
    greet() {
      console.log("Hello, " + this.name);
    }
}
const alice = new Person("Alice", 25);
alice.greet();

// Asynchronous function
function fetchData() {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            resolve('Data fetched successfully!');
        }, 1000);
    });
}
 
fetchData().then(data => {
    console.log(data);
}).catch(error => {
    console.error('An error occurred:', error);
});

async function fetchAndDisplayData() {
    try {
        const data = await fetchData();
        console.log(data);
    } catch (error) {
        console.error('An error occurred:', error);
    }
}
fetchAndDisplayData();

class TestA {
	constructor(public value: number) {}
}

export class ClassA {
	a = new TestA(100);
}

// Exception Handling Statement
a.then(result => {
	console.error('YY start throw error');
	console.log('yy execute then');
}).catch((error: Error) => {
	console.error('yy occur error');
}).finally(()=>{
	console.error('yy occur finally');
})

// Chain Expression
let result = obj.
foo().
bar[0].
baz()

// SuperExpression
class Parent {
    sayHello() {
        return "Hello from Parent";
    }
}

class Child extends Parent {
    sayHello() {
        return super.
        sayHello() + " and Child";
    }
}

const d = new Child();
console.log(d.sayHello());
