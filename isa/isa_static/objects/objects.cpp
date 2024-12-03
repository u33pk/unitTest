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
#include "logger.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/ir_core.h"
#include "ir_impl.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <gtest/gtest.h>

// NOLINTBEGIN(readability-magic-numbers)
namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitICreateObjectsStaticTest : public ::testing::Test {};

static void TransformIrCreateNewObject(AbckitGraph *graph)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "objects"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ClassByNameContext ctxClassFinder = {nullptr, "B"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &ctxClassFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxClassFinder.klass, nullptr);

    auto method = helpers::FindMethodByName(graph->file, "bar");

    AbckitInst *newObjectInst = g_statG->iCreateNewObject(graph, ctxClassFinder.klass);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);

    g_implG->iInsertBefore(newObjectInst, lastInst);
    AbckitInst *callInst = g_statG->iCreateCallStatic(graph, method, 1, newObjectInst);
    g_implG->iInsertAfter(callInst, newObjectInst);
}

static AbckitInst *GetInitObjectInst(AbckitGraph *graph, AbckitInst *constantInst, size_t numArg)
{
    switch (numArg) {
        case 0: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg);
        }
        case 1: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst);
        }
        case 2U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst);
        }
        case 3U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst, constantInst);
        }
        case 4U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst, constantInst,
                                              constantInst);
        }
        case 5U: {
            auto method1 = helpers::FindMethodByName(graph->file, "_ctor_:objects.C;i32;i32;i32;i32;i32;void;");
            EXPECT_NE(method1, nullptr);
            return g_statG->iCreateInitObject(graph, method1, numArg, constantInst, constantInst, constantInst,
                                              constantInst, constantInst);
        }
        default:
            LIBABCKIT_UNREACHABLE_TEST(DEBUG);
    }
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformIrCreateInitObject(AbckitGraph *graph, size_t numArg)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, "objects"};
    g_implI->fileEnumerateModules(graph->file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::ClassByNameContext ctxClassFinder = {nullptr, "C"};
    g_implI->moduleEnumerateClasses(ctxFinder.module, &ctxClassFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxClassFinder.klass, nullptr);

    AbckitBasicBlock *mainBB = g_implG->bbGetSuccBlock(g_implG->gGetStartBasicBlock(graph), 0);
    AbckitInst *lastInst = g_implG->bbGetLastInst(mainBB);
    AbckitInst *constantInst = numArg > 0 ? g_implG->gCreateConstantI64(graph, 0x1) : nullptr;
    if (constantInst != nullptr) {
        g_implG->iInsertBefore(constantInst, lastInst);
    }

    g_implG->iInsertBefore(GetInitObjectInst(graph, constantInst, numArg), lastInst);
}

void InitObjectTest(size_t numArg, helpers::InstSchema<AbckitIsaApiStaticOpcode> initObjectInst,
                    const std::string &expectedOutput)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc", "objects/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "A\n"));

    std::vector<helpers::InstSchema<AbckitIsaApiStaticOpcode>> startBB = {};
    if (numArg != 0) {
        startBB = {{0, ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT, {}}};
    }

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "main",
        [numArg](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateInitObject(graph, numArg);
        },
        [&](AbckitGraph *graph) {
            std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
                {{}, {1}, startBB},
                {
                    {0},
                    {2U},
                    {
                        {1, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                        {4, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL, {1}},
                        initObjectInst,
                        {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},

                    },
                },
                {{1}, {}, {}}};
            helpers::VerifyGraph(graph, bbSchema);
        });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc",
                                       "objects/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, expectedOutput));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateNewObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateNewObject)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc", "objects/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "A\n"));

    std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> bbSchema = {
        {{}, {1}, {}},
        {
            {0},
            {2U},
            {
                {0, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}},
                {3, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL, {0}},
                {4, ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT, {}},
                {5, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC, {4}},
                {6, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID, {}},

            },
        },
        {{1}, {}, {}}};

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "main",
        [](AbckitFile * /*file*/, AbckitCoreFunction * /*method*/, AbckitGraph *graph) {
            TransformIrCreateNewObject(graph);
        },
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc",
                                       "objects/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "A\nB\n"));

    helpers::TransformMethod(
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc",
        ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc", "main",
        []([[maybe_unused]] AbckitFile *file, [[maybe_unused]] AbckitCoreFunction *method,
           [[maybe_unused]] AbckitGraph *graph) {},
        [&](AbckitGraph *graph) { helpers::VerifyGraph(graph, bbSchema); });

    output = helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/isa/isa_static/objects/objects_modified.abc",
                                       "objects/ETSGLOBAL", "main");
    EXPECT_TRUE(helpers::Match(output, "A\nB\n"));
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg0)
{
    InitObjectTest(0, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {}}, "A\nC\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg1)
{
    InitObjectTest(1, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0}}, "A\nC 1\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg2)
{
    InitObjectTest(2U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0}}, "A\nC 11\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg3)
{
    InitObjectTest(3U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0, 0}}, "A\nC 111\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg4)
{
    InitObjectTest(4U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0, 0, 0}}, "A\nC 1111\n");
}

// Test: test-kind=api, api=IsaApiStaticImpl::iCreateInitObject, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitICreateObjectsStaticTest, LibAbcKitTestCreateIcreateInitObjectArg5)
{
    InitObjectTest(5U, {4U, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT, {0, 0, 0, 0, 0}}, "A\nC 11111\n");
}

}  // namespace libabckit::test
// NOLINTEND(readability-magic-numbers)
