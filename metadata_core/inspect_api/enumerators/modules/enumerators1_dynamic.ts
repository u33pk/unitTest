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
function m1F0(): void {
    () => {};
}

function* m1F1(): Iterable<void> {}
class M1C0 {
    M1C0F0(): void {}
    static M1C0F1(): void {}
    async M1C0F2(): Promise<void> {}
}
enum M1E0 {}

namespace M1N0 {
    function m1N0F0(): void {}

    function* m1N0F1(): Iterable<void> {}
    class M1N0C0 {
        M1N0C0F0(): void {}
        static M1N0C0F1(): void {}
        async M1N0C0F2(): Promise<void> {}
    }
    enum M1N0E0 {}
}
