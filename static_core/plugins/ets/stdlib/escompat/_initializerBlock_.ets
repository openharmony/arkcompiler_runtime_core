/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package escompat;
// Note: This file initialize all the `const` property that non-immediately initialized(spec-issue 217).
static {
    FROM_BUFFER = new FromBuffer()
    hexChars = [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'A', c'B', c'C', c'D', c'E', c'F'];
    uriArray = fillInURINotComponent();
    uriSpecialChars = createAndFillInURISpecial();
    uriArrayComponent = fillInCommonURI();
    NaN = Double.NaN;
    Infinity = Double.POSITIVE_INFINITY;
    IS_LITTLE_ENDIAN = Runtime.isLittleEndianPlatform()
}
