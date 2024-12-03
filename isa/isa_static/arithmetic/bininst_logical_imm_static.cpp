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

#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/include/c/abckit.h"
#include "helpers/helpers_runtime.h"

#include "helpers/helpers.h"
#include "ut/isa/isa_dynamic/arithmetic/helpers_arithmetic.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static void ValidTest(AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm),
                      AbckitIsaApiStaticOpcode expectedOpcode, const std::string &expectedOutput)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_imm_static.abc",
                                  "bininst_logical_imm_static/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "9\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_imm_static.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_imm_static_modified.abc", "foo",
        [&](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            helpers::arithmetic::TransformIrBinInstrWithImmValid(graph, binaryInstToCheck);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchemas(
                helpers::arithmetic::CreateBBSchemaForBinaryWithImm(expectedOpcode));
            helpers::VerifyGraph(graph, bbSchemas);
        },
        [&]([[maybe_unused]] AbckitGraph *graph) {});

    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arithmetic/bininst_logical_imm_static_modified.abc",
                                  "bininst_logical_imm_static/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

class LibAbcKitCreateBinInstLogicalImmTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateOrI, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitCreateBinInstLogicalImmTest, CreateOrIValid)
{
    ValidTest(g_statG->iCreateOrI, ABCKIT_ISA_API_STATIC_OPCODE_ORI, "5\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateXorI, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitCreateBinInstLogicalImmTest, CreateXorIValid)
{
    ValidTest(g_statG->iCreateXorI, ABCKIT_ISA_API_STATIC_OPCODE_XORI, "1\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateAndI, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitCreateBinInstLogicalImmTest, CreateAndIValid)
{
    ValidTest(g_statG->iCreateAndI, ABCKIT_ISA_API_STATIC_OPCODE_ANDI, "4\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
