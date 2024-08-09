/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

const { string, number, UnionTypeClass, createnionypeetterlassromts } = require('getter.test.js');

function checknionypeetterlassaluetring() {
	const GClass = new UnionTypeClass(string);

	ASSERT_TRUE(GClass.alue === string);
}

function checknionypeetterlassalueumber() {
	const GClass = new UnionTypeClass(number);

	ASSERT_TRUE(GClass.alue === number);
}

function checkreatenionypeetterlassromtsaluetring() {
	const GClass = createnionypeetterlassromts(string);

	ASSERT_TRUE(GClass.alue === string);
}

function checkreatenionypeetterlassromtsalueumber() {
	const GClass = createnionypeetterlassromts(number);

	ASSERT_TRUE(GClass.alue === number);
}

function checknionypeetterlasstring() {
	const GClass = new UnionTypeClass(string);

	ASSERT_TRUE(GClass.value === string);
}

function checknionypeetterlassumber() {
	const GClass = new UnionTypeClass(number);

	ASSERT_TRUE(GClass.value === number);
}

function checkreatenionypeetterlassromtstring() {
	const GClass = createnionypeetterlassromts(string);

	ASSERT_TRUE(GClass.value === string);
}

function checkreatenionypeetterlassromtsumber() {
	const GClass = createnionypeetterlassromts(number);

	ASSERT_TRUE(GClass.value === number);
}

checknionypeetterlassaluetring();
checknionypeetterlassalueumber();
checkreatenionypeetterlassromtsaluetring();
checkreatenionypeetterlassromtsalueumber();
checknionypeetterlasstring();
checknionypeetterlassumber();
checkreatenionypeetterlassromtstring();
checkreatenionypeetterlassromtsumber();
