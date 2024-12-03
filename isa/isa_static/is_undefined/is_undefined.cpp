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

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {
auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

void TransformIsUndefinedIr(AbckitGraph *graph)
{
    AbckitInst *initObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
    ASSERT_NE(initObj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(initObj, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitInst *isUndefined = g_statG->iCreateIsUndefined(graph, initObj);
    ASSERT_NE(isUndefined, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertAfter(isUndefined, initObj);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::ReplaceInst(ret, g_statG->iCreateReturn(graph, isUndefined));
}
}  // namespace

class LibAbcKitIsUndefinedStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateIsUndefined, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitIsUndefinedStaticTest, LibAbcKitTestIsUndefined)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/is_undefined/is_undefined_static.abc",
                                            "is_undefined_static/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "true\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/is_undefined/is_undefined_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/is_undefined/is_undefined_static_modified.abc", "foo",
        [](AbckitFile * /*file*/, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *module = g_implI->functionGetModule(method);
            helpers::ClassByNameContext classCtxFinder = {nullptr, "A"};
            g_implI->moduleEnumerateClasses(module, &classCtxFinder, helpers::ClassByNameFinder);
            ASSERT_NE(classCtxFinder.klass, nullptr);
            TransformIsUndefinedIr(graph);
        },
        [](AbckitGraph *graph) {
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb1({{}, {1}, {{}}});
            std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> insts({
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {2, ABCKIT_ISA_API_STATIC_OPCODE_ISUNDEFINED, {0}},
                {1, ABCKIT_ISA_API_STATIC_OPCODE_RETURN, {2}},
            });
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb2({{0}, {2}, insts});
            helpers::BBSchema<AbckitIsaApiStaticOpcode> bb3({{1}, {}, {}});
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas({bb1, bb2, bb3});
            helpers::VerifyGraph(graph, bbSchemas);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/is_undefined/is_undefined_static_modified.abc",
                                       "is_undefined_static/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "false\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
