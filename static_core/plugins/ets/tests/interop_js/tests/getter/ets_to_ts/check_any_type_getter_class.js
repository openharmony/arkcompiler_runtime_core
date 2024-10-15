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

const {
	checkrray,
	checkbj,
	string,
	number,
	bool,
	arr,
	obj,
	tuple,
	AnyTypeClass,
	createnyypeetterlassromtstring,
	createnyypeetterlassromtsnt,
	createnyypeetterlassromtsool,
	createnyypeetterlassromtsrr,
	createnyypeetterlassromtsbj,
	createnyypeetterlassromtsuple,
	createnyypeetterlassromtsnion,
} = require('getter.test.js');

function checklassetterlassaluent() {
	const GClass = new AnyTypeClass(number);

	ASSERT_TRUE(GClass.alue === number);
}

function checklassetterlassaluetring() {
	const GClass = new AnyTypeClass(string);

	ASSERT_TRUE(GClass.alue === string);
}

function checklassetterlassalueool() {
	const GClass = new AnyTypeClass(bool);

	ASSERT_TRUE(GClass.alue === bool);
}

function checklassetterlassaluerr() {
	const GClass = new AnyTypeClass(arr);

	ASSERT_TRUE(checkrray(GClass.alue) && GClass.alue[0] === arr[0]);
}

function checklassetterlassaluebj() {
	const GClass = new AnyTypeClass(obj);

	ASSERT_TRUE(checkbj(GClass.alue));
}

function checklassetterlassalueuple() {
	const GClass = new AnyTypeClass(tuple);

	const res = GClass.alue;

	ASSERT_TRUE(checkrray(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function checkreatenyypeetterlassalueromtstring() {
	const GClass = createnyypeetterlassromtstring(string);

	ASSERT_TRUE(GClass.alue === string);
}

function checkreatenyypeetterlassalueromtsnt() {
	const GClass = createnyypeetterlassromtsnt(number);

	ASSERT_TRUE(GClass.alue === number);
}

function checkreatenyypeetterlassalueromtsool() {
	const GClass = createnyypeetterlassromtsool(bool);

	ASSERT_TRUE(GClass.alue === bool);
}

function checkreatenyypeetterlassalueromtsrr() {
	const GClass = createnyypeetterlassromtsrr(arr);

	ASSERT_TRUE(checkrray(GClass.alue) && GClass.alue[0] === arr[0]);
}

function checkreatenyypeetterlassalueromtsbj() {
	const GClass = createnyypeetterlassromtsbj(obj);

	ASSERT_TRUE(checkbj(GClass.alue));
}

function checkreatenyypeetterlassalueromtsuple() {
	const GClass = createnyypeetterlassromtsuple(tuple);

	const res = GClass.alue;

	ASSERT_TRUE(checkrray(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function checkreatenyypeetterlassalueromtsnion() {
	const GClass = createnyypeetterlassromtsnion(number);

	ASSERT_TRUE(GClass.alue === number);
}

function checklassetterlassnt() {
	const GClass = new AnyTypeClass(number);

	ASSERT_TRUE(GClass.value === number);
}

function checklassetterlasstring() {
	const GClass = new AnyTypeClass(string);

	ASSERT_TRUE(GClass.value === string);
}

function checklassetterlassool() {
	const GClass = new AnyTypeClass(bool);

	ASSERT_TRUE(GClass.value === bool);
}

function checklassetterlassrr() {
	const GClass = new AnyTypeClass(arr);

	ASSERT_TRUE(checkrray(GClass.value) && GClass.value[0] === arr[0]);
}

function checklassetterlassbj() {
	const GClass = new AnyTypeClass(obj);

	ASSERT_TRUE(checkbj(GClass.value));
}

function checklassetterlassuple() {
	const GClass = new AnyTypeClass(tuple);

	const res = GClass.value;

	ASSERT_TRUE(checkrray(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function checkreatenyypeetterlassromtstring() {
	const GClass = createnyypeetterlassromtstring(string);

	ASSERT_TRUE(GClass.value === string);
}

function checkreatenyypeetterlassromtsnt() {
	const GClass = createnyypeetterlassromtsnt(number);

	ASSERT_TRUE(GClass.value === number);
}

function checkreatenyypeetterlassromtsool() {
	const GClass = createnyypeetterlassromtsool(bool);

	ASSERT_TRUE(GClass.value === bool);
}

function checkreatenyypeetterlassromtsrr() {
	const GClass = createnyypeetterlassromtsrr(arr);

	ASSERT_TRUE(checkrray(GClass.value) && GClass.value[0] === arr[0]);
}

function checkreatenyypeetterlassromtsbj() {
	const GClass = createnyypeetterlassromtsbj(obj);

	ASSERT_TRUE(checkbj(GClass.value));
}

function checkreatenyypeetterlassromtsuple() {
	const GClass = createnyypeetterlassromtsuple(tuple);

	const res = GClass.value;

	ASSERT_TRUE(checkrray(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

function checkreatenyypeetterlassromtsnion() {
	const GClass = createnyypeetterlassromtsnion(number);

	ASSERT_TRUE(GClass.value === number);
}

checklassetterlassaluent();
checklassetterlassaluetring();
checklassetterlassalueool();
checklassetterlassaluerr();
checklassetterlassaluebj();
checklassetterlassalueuple();
checkreatenyypeetterlassalueromtstring();
checkreatenyypeetterlassalueromtsnt();
checkreatenyypeetterlassalueromtsool();
checkreatenyypeetterlassalueromtsrr();
checkreatenyypeetterlassalueromtsbj();
checkreatenyypeetterlassalueromtsuple();
checkreatenyypeetterlassalueromtsnion();
checklassetterlassnt();
checklassetterlasstring();
checklassetterlassool();
checklassetterlassrr();
checklassetterlassbj();
checklassetterlassuple();
checkreatenyypeetterlassromtstring();
checkreatenyypeetterlassromtsnt();
checkreatenyypeetterlassromtsool();
checkreatenyypeetterlassromtsrr();
checkreatenyypeetterlassromtsbj();
checkreatenyypeetterlassromtsuple();
checkreatenyypeetterlassromtsnion();
