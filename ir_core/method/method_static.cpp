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

#include "libabckit/include/c/abckit.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/ir_core.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformSetSetCallMethod(AbckitGraph *graph, AbckitCoreFunction *bar)
{
    auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iSetFunction(call, bar);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

class LibAbcKitMethodStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=GraphApiImpl::iSetFunction, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitMethodStaticTest, LibAbcKitTestSetCallMethod)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc",
                                            "method_static/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "foo\n"));

    helpers::TransformMethod(ABCKIT_ABC_DIR "ut/ir_core/method/method_static.abc",
                             ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc", "main",
                             [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
                                 auto *bar = helpers::FindMethodByName(g_implI->functionGetFile(method), "bar");
                                 ASSERT_NE(bar, nullptr);
                                 TransformSetSetCallMethod(graph, bar);
                             });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/ir_core/method/method_static_modified.abc",
                                       "method_static/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "bar\n"));
}

}  // namespace libabckit::test
