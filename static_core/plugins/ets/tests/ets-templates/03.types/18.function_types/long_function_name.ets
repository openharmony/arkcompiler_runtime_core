/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Long-named function declaration (300 characters)
---*/

function longfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characters123(): int {
  return 1;
}

function main() {
  assertEQ( longfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characterslongfunctionnameover256characters123(), 1 )
}
