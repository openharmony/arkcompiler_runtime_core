/**
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

'use strict';
class Vec2 {
	constructor(x = 0, y = 0) {
		this.x = x;
		this.y = y;
	}
	x;
	y;
}

class SBody {
	r = new Vec2(0, 0);
	v = new Vec2(0, 0);
	rad = 0;
	m = 0;
}

function bouncingPandas() {
	const vec = new Vec2(1200, 800);
	const countSB = 512;
	const arrSBody = [];
	const step = 13;
	for (let i = 0; i < countSB; ++i) {
		arrSBody[i] = new SBody();
		arrSBody[i].rad = 11;
		arrSBody[i].m = 11 * 11;
		arrSBody[i].r.x = (step * i) % 1200;
		arrSBody[i].r.y = Math.round(step * ((step * i) / 1200 + 1));
		arrSBody[i].v.x = 5;
		arrSBody[i].v.y = 5;
	}
	const resArr = gtest.etsVm.callWithCopy('bouncing_pandas.recomputeFrameSBody', arrSBody, vec);
	let resultHash = 0;
	for (let i = 0; i < countSB; ++i) {
		resultHash += resArr[i].r.x + resArr[i].r.y + resArr[i].v.x + resArr[i].v.y + resArr[i].rad + resArr[i].m;
	}

	if (Math.round(resultHash) !== 395799) {
		console.log('Wrong result hash ' + Math.round(resultHash));
		gtest.ret = 1;
		return 1;
	} else {
		gtest.ret = 0;
		return 0;
	}
}

exports.bouncingPandas = bouncingPandas;
