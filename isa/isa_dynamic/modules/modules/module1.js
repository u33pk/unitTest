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
export function regularImportFunc1FromModule1() {
    print('regular import func1 from module1');
}
export function namespaceImportFunc2FromModule1() {
    print('namespace import func2 from module1');
}
export function namespaceImportFunc3FromModule1() {
    print('namespace import func3 from module1');
}
export function indirectExportFunc1FromModule1() {
    print('indirect export func1 from module1');
}
export function indirectExportFunc2FromModule1() {
    print('indirect export func2 from module1');
}
export function sameFuncInDifferentModules() {
    print('the same func from module1');
}
export let NewLocalExportLet = 3;
