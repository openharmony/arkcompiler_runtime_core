/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

/*---
desc: 07.32 Lambda Expressions
name: 07.expressions/32.lambda_expressions/callRegularLambda
tags: []
---*/

type regularLambda = () => void

class Test {
    regularFunc(): boolean {
        return true;
    }

    testCallRegularLambda(): void {
        let is_call_regular_lambda: boolean = false;

        let regular_lambda: regularLambda = (): void => { 
            this.regularFunc();
            is_call_regular_lambda = true;
        }

        assert(is_call_regular_lambda == false);
        regular_lambda();
        assert(is_call_regular_lambda == true);
    }
}

function main(): void {
    new Test().testCallRegularLambda();
}
