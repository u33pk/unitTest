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

#include <cassert>
#include <cstring>
#include <gtest/gtest.h>

#include "libabckit/include/c/abckit.h"
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/include/c/metadata_core.h"
#include "metadata_inspect_impl.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "ir_impl.h"
#include "logger.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_dynG = AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitJSInspectApiModulesTest : public ::testing::Test {};

// ========================================
// Modules Tests
// ========================================

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test, solid logic
// Test: test-kind=internal, abc-kind=JS, category=positive
TEST_F(LibAbcKitJSInspectApiModulesTest, DynamicFillModule)
{
    auto output = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/modules/JSmodules_dynamic.abc", "JSmodules_dynamic");
    auto expected =
        "regular import func1 from module1\nregular import func1 from module2\nregular default import func1 from "
        "module3\nregular default import func2 from module2\nnamespace import func2 from module1\nnamespace import "
        "func3 from module1\n2\n";
    EXPECT_TRUE(helpers::Match(output, expected));

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/modules/JSmodules_dynamic.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "JSmodules_dynamic"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto *m = ctxFinder.module;

    EXPECT_NE(m->GetJsImpl()->impl.record, nullptr);
    EXPECT_NE(m->GetJsImpl()->impl.moduleLiteralArray, nullptr);
    EXPECT_EQ(m->GetJsImpl()->impl.moduleRequestsOffset, 1);
    EXPECT_EQ(m->GetJsImpl()->impl.regularImportsOffset, 5U);
    EXPECT_EQ(m->GetJsImpl()->impl.namespaceImportsOffset, 18U);
    EXPECT_EQ(m->GetJsImpl()->impl.localExportsOffset, 25U);
    EXPECT_EQ(m->GetJsImpl()->impl.indirectExportsOffset, 40U);
    EXPECT_EQ(m->GetJsImpl()->impl.starExportsOffset, 47U);

    // Request Modules
    EXPECT_EQ(m->md.size(), 3U);
    size_t idx = 0;
    EXPECT_EQ(helpers::AbckitStringToString(m->md[idx]->moduleName), "modules/JSmodule1");
    EXPECT_EQ(m->md[idx]->isExternal, false);
    idx++;
    EXPECT_EQ(helpers::AbckitStringToString(m->md[idx]->moduleName), "modules/JSmodule2");
    EXPECT_EQ(m->md[idx]->isExternal, false);
    idx++;
    EXPECT_EQ(helpers::AbckitStringToString(m->md[idx]->moduleName), "modules/JSmodule3");
    EXPECT_EQ(m->md[idx]->isExternal, false);

    // Imports
    EXPECT_EQ(m->id.size(), 5U);

    // Regular imports
    idx = 0;
    EXPECT_EQ(m->id[idx]->importingModule, m);
    EXPECT_EQ(m->id[idx]->importedModule, m->md[1]);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_TRUE(m->id[idx]->GetJsImpl()->payload.GetDynId().isRegularImport);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->payload.GetDynId().moduleRecordIndexOff, idx);
    idx++;
    EXPECT_EQ(m->id[idx]->importingModule, m);
    EXPECT_EQ(m->id[idx]->importedModule, m->md[2U]);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_TRUE(m->id[idx]->GetJsImpl()->payload.GetDynId().isRegularImport);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->payload.GetDynId().moduleRecordIndexOff, idx);
    idx++;
    EXPECT_EQ(m->id[idx]->importingModule, m);
    EXPECT_EQ(m->id[idx]->importedModule, m->md[1]);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_TRUE(m->id[idx]->GetJsImpl()->payload.GetDynId().isRegularImport);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->payload.GetDynId().moduleRecordIndexOff, idx);
    idx++;
    EXPECT_EQ(m->id[idx]->importingModule, m);
    EXPECT_EQ(m->id[idx]->importedModule, m->md[0]);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_TRUE(m->id[idx]->GetJsImpl()->payload.GetDynId().isRegularImport);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->payload.GetDynId().moduleRecordIndexOff, idx);
    idx++;

    // Namespace imports
    EXPECT_EQ(m->id[idx]->importingModule, m);
    EXPECT_EQ(m->id[idx]->importedModule, m->md[0]);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_FALSE(m->id[idx]->GetJsImpl()->payload.GetDynId().isRegularImport);
    EXPECT_EQ(m->id[idx]->GetJsImpl()->payload.GetDynId().moduleRecordIndexOff, 0);

    // Exports
    EXPECT_EQ(m->ed.size(), 10U);

    // Local Exports
    idx = 0;

    // Combined StarExport
    EXPECT_EQ(m->ed[idx]->exportingModule, m);
    EXPECT_EQ(m->ed[idx]->exportedModule, m->md[0]);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().kind,
              AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport, true);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().serviceNamespaceImportIdx, 1);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff, idx);
    idx++;
    EXPECT_EQ(m->ed[idx]->exportingModule, m);
    EXPECT_EQ(m->ed[idx]->exportedModule, m->md[1]);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().kind,
              AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport, true);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().serviceNamespaceImportIdx, 2U);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff, idx);

    idx++;
    while (idx < 7U) {
        EXPECT_EQ(m->ed[idx]->exportingModule, m);
        EXPECT_EQ(m->ed[idx]->exportedModule, m);
        EXPECT_EQ(m->ed[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
        EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().kind,
                  AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT);
        EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport, false);
        EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff, idx);
        idx++;
    }

    // Indirect Exports
    EXPECT_EQ(m->ed[idx]->exportingModule, m);
    EXPECT_EQ(m->ed[idx]->exportedModule, m->md[0]);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().kind,
              AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport, false);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff, 0);
    idx++;
    EXPECT_EQ(m->ed[idx]->exportingModule, m);
    EXPECT_EQ(m->ed[idx]->exportedModule, m->md[0]);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().kind,
              AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport, false);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff, 1);
    idx++;

    // Star Exports
    EXPECT_EQ(m->ed[idx]->exportingModule, m);
    EXPECT_EQ(m->ed[idx]->exportedModule, m->md[1]);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->kind, AbckitImportExportDescriptorKind::UNTYPED);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().kind,
              AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport, false);
    EXPECT_EQ(m->ed[idx]->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff, 0);

    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/modules/JSmodules_dynamic_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto output2 = helpers::ExecuteDynamicAbc(
        ABCKIT_ABC_DIR "ut/extensions/js/inspect_api/modules/JSmodules_dynamic_modified.abc", "JSmodules_dynamic");
    EXPECT_TRUE(helpers::Match(output2, expected));
}

}  // namespace libabckit::test
