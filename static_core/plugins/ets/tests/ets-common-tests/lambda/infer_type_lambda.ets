/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class Foo {
    public lambdaInferType() {
        let x = () => {
            return 1
        }
        return x
    }
    
    public lambdaInferTypeReturnLambda() {
        let x = () => () => {
           return 1
        }
        return x
    }

    public lambdaInferTypeReturnLambda1() {
        let x = () => {
            let y = () => {
                return 1
            }
            return y
        }
        return x
    }

    public lambdaInferTypeReturnLiteral() {
        let x = () => 1
        return x
    }

    public lambdaNestingReturnStatement() {
        return () => {
            return () => {
                return 1
            }
        }
    }
}

function main() {
    let foo = new Foo()

    let x = foo.lambdaInferType()()
    assertEQ(x, 1)

    let x1 = foo.lambdaInferTypeReturnLambda()()()
    assertEQ(x1, 1)

    let x2 = foo.lambdaInferTypeReturnLambda1()()()
    assertEQ(x2, 1)

    let x3 = foo.lambdaInferTypeReturnLiteral()()
    assertEQ(x3, 1)
    
    let x4 = foo.lambdaNestingReturnStatement()()()
    assertEQ(x4, 1)
}
