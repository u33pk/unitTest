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
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/ir_core.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/src/logger.h"
#include <gtest/gtest.h>

// libabckit::Logger *libabckit::Logger::Logger_ = nullptr;

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

struct TransformStoreArrayByIdxArgs {
    AbckitGraph *graph = nullptr;
    enum AbckitStatus expectedStatus = ABCKIT_STATUS_BAD_ARGUMENT;
    AbckitInst *idx = nullptr;
    AbckitInst *newValue = nullptr;
    enum AbckitTypeId valueTypeId = ABCKIT_TYPE_ID_INVALID;
};

void TransformStoreArrayByIdx(TransformStoreArrayByIdxArgs &args,
                              AbckitInst *(*icreateStoreArray)(AbckitGraph *, AbckitInst *, AbckitInst *, AbckitInst *,
                                                               enum AbckitTypeId))
{
    auto *graph = args.graph;
    auto expectedStatus = args.expectedStatus;
    auto *idx = args.idx;
    auto *newValue = args.newValue;
    auto valueTypeId = args.valueTypeId;

    AbckitInst *arr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_NEWARRAY);
    ASSERT_NE(arr, nullptr);

    AbckitInst *store = icreateStoreArray(graph, arr, idx, newValue, valueTypeId);

    if (expectedStatus != ABCKIT_STATUS_NO_ERROR && g_impl->getLastError() == expectedStatus) {
        return;
    }

    ASSERT_NE(store, nullptr);

    AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(ret, nullptr);

    g_implG->iInsertBefore(store, ret);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

class LibAbcKitArrayStaticTest : public ::testing::Test {};

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreArrayWide, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestStoreArrayWide)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_wide.abc",
                                            "store_array_wide/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "3\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_wide.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_wide_modified.abc", "store_element",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_F64;

            AbckitInst *newValue = g_implG->gCreateConstantF64(graph, 4);
            ASSERT_NE(newValue, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *idx = g_implG->gCreateConstantI64(graph, 2);
            ASSERT_NE(idx, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            TransformStoreArrayByIdxArgs args {graph, ABCKIT_STATUS_NO_ERROR, idx, newValue, valueTypeId};
            TransformStoreArrayByIdx(args, g_statG->iCreateStoreArrayWide);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_wide_modified.abc",
                                       "store_array_wide/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "4\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreArrayWide, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestStoreArrayWideNeg)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array.abc",
                                            "store_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "3\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_modified_neg.abc", "store_element",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_F64;

            AbckitInst *newValue = g_implG->gCreateConstantF64(graph, 4);
            ASSERT_NE(newValue, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *idx = g_implG->gCreateConstantF64(graph, 2);  // idx's type should be I64 or I32
            ASSERT_NE(idx, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            TransformStoreArrayByIdxArgs args {graph, ABCKIT_STATUS_BAD_ARGUMENT, idx, newValue, valueTypeId};
            TransformStoreArrayByIdx(args, g_statG->iCreateStoreArrayWide);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreArray, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestStoreArray)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array.abc",
                                            "store_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "3\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_modified.abc", "store_element",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_I32;

            AbckitInst *newValue = g_implG->gCreateConstantI32(graph, 4);
            ASSERT_NE(newValue, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *idx = g_implG->gCreateConstantI32(graph, 2);
            ASSERT_NE(idx, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            TransformStoreArrayByIdxArgs args {graph, ABCKIT_STATUS_NO_ERROR, idx, newValue, valueTypeId};
            TransformStoreArrayByIdx(args, g_statG->iCreateStoreArray);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_modified.abc",
                                       "store_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "4\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateStoreArray, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestStoreArrayNeg)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array.abc",
                                            "store_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "3\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/store_array_modified_neg.abc", "store_element",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId valueTypeId = AbckitTypeId::ABCKIT_TYPE_ID_I32;

            AbckitInst *newValue = g_implG->gCreateConstantI32(graph, 4);
            ASSERT_NE(newValue, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *idx = g_implG->gCreateConstantI64(graph, 2);  // idx's type should be I32
            ASSERT_NE(idx, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            TransformStoreArrayByIdxArgs args {graph, ABCKIT_STATUS_BAD_ARGUMENT, idx, newValue, valueTypeId};
            TransformStoreArrayByIdx(args, g_statG->iCreateStoreArray);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadArray, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestLoadArray)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array.abc",
                                            "load_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array_modified.abc", "get_element",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId returnTypeId = AbckitTypeId::ABCKIT_TYPE_ID_F64;

            AbckitInst *arr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_NEWARRAY);
            ASSERT_NE(arr, nullptr);

            AbckitInst *idx = g_implG->gCreateConstantI64(graph, 1);
            ASSERT_NE(idx, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *ld = g_statG->iCreateLoadArray(graph, arr, idx, returnTypeId);
            ASSERT_NE(ld, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);

            g_implG->iInsertBefore(ld, ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iSetInput(ret, ld, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array_modified.abc",
                                       "load_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "2\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadArray, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestLoadArrayNeg)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array.abc",
                                            "load_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_array_modified_neg.abc", "get_element",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitTypeId returnTypeId = AbckitTypeId::ABCKIT_TYPE_ID_F64;

            AbckitInst *fakeArr = g_implG->gCreateConstantI64(graph, 1);
            ASSERT_NE(fakeArr, nullptr);

            AbckitInst *idx = g_implG->gCreateConstantI64(graph, 1);
            ASSERT_NE(idx, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            [[maybe_unused]] AbckitInst *ld = g_statG->iCreateLoadArray(graph, fakeArr, idx, returnTypeId);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLenArray, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestLenArray)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array.abc",
                                            "len_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array_modified.abc", "get_len",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *arr = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_NEWARRAY);
            ASSERT_NE(arr, nullptr);

            AbckitInst *len = g_statG->iCreateLenArray(graph, arr);
            ASSERT_NE(len, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);

            g_implG->iInsertBefore(len, ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iSetInput(ret, len, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array_modified.abc",
                                       "len_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "4\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLenArray, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestLenArrayNeg)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array.abc",
                                            "len_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/len_array_modified_neg.abc", "get_len",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            AbckitInst *fakeArr = g_implG->gCreateConstantI64(graph, 1);
            ASSERT_NE(fakeArr, nullptr);

            [[maybe_unused]] AbckitInst *len = g_statG->iCreateLenArray(graph, fakeArr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateLoadConstArray, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArrayStaticTest, LibAbcKitTestLoadConstArray)
{
    auto output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_const_array.abc",
                                            "load_const_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "1\n"));
    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_const_array.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_const_array_modified.abc", "get_element",
        [](AbckitFile *file, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            auto arr = std::vector<AbckitLiteral *>({
                g_implM->createLiteralU32(file, 4),
                g_implM->createLiteralU32(file, 2),
                g_implM->createLiteralU32(file, 3),
            });
            auto litArray = g_implM->createLiteralArray(file, arr.data(), arr.size());
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *loadConstArray = g_statG->iCreateLoadConstArray(graph, litArray);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            auto *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            g_implG->iInsertBefore(loadConstArray, ret);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iSetInput(ret, loadConstArray, 0);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            helpers::ReplaceInst(ret, g_statG->iCreateReturn(graph, loadConstArray));
        },
        []([[maybe_unused]] AbckitGraph *graph) {});
    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/arrays/load_const_array_modified.abc",
                                       "load_const_array/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "4\n"));
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
