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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Multi-method implementation under interface nested inheritance.
---*/

interface IAction {
  start(): string;
  stop(): string;
}

interface IPlayable extends IAction {
  play(): string;
  pause(): string;
}

interface IRecordable extends IAction {
  record(): string;
  save(): string;
}

class MediaPlayer implements IPlayable, IRecordable {
  start() {
    return 'Media player started.';
  }

  stop() {
    return 'Media player stopped.';
  }

  play() {
    return 'Media is playing.';
  }

  pause() {
    return 'Media is paused.';
  }

  record() {
    return 'Media is being recorded.';
  }
  
  save() {
    return 'Recording saved.';
  }
}

function main(): void {
  const player = new MediaPlayer();
  const result = player.start();
  const result1 = player.play();
  const result2 = player.pause();
  const result3 = player.record();
  const result4 = player.save();
  const result5 = player.stop();
  assertEQ(result, 'Media player started.');
  assertEQ(result1, 'Media is playing.');
  assertEQ(result2, 'Media is paused.');
  assertEQ(result3, 'Media is being recorded.');
  assertEQ(result4, 'Recording saved.');
  assertEQ(result5, 'Media player stopped.');
}