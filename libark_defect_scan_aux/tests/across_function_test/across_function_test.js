/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


class Database {
  addData(data) {
    console.log('addData: ', data);
  }

  removeData(data) {
    console.log('removeData: ', data);
  }

  querySql(sql) {
    return 'querySql: ' + sql;
  }

  updateData(oldData, newData) {
    console.log('updateData: ', oldData, newData);
  }
}

class UserInput {
  constructor(text) {
    this.text = text;
  }

  getText() {
    return this.text;
  }

  setText(text) {
    this.text = text;
  }
}

function getData(store, userInput) {
  let name = userInput.getText();
  let sql = "select * from user_data where name = '" + name + "'";
  let result = store.querySql(sql);
  return result;
}

function funcEntry() {
  let store = new Database();
  let userInput = new UserInput('UserInput');
  getData(store, userInput);
}
