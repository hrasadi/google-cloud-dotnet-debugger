// Copyright 2018 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef I_DBG_OBJECT_FACTORY_MOCK_H_
#define I_DBG_OBJECT_FACTORY_MOCK_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "i_dbg_object_factory.h"

namespace google_cloud_debugger_test {

class IDbgObjectFactoryMock : public google_cloud_debugger::IDbgObjectFactory {
 public:
  // Create a DbgObject with an evaluation depth of depth.
  HRESULT CreateDbgObject(
      ICorDebugValue* debug_value, int depth,
      std::unique_ptr<google_cloud_debugger::DbgObject>* result_object,
      std::ostream* err_stream) override;

  MOCK_METHOD4(CreateDbgObjectMockHelper,
               HRESULT(ICorDebugValue* debug_value, int depth,
                       google_cloud_debugger::DbgObject **result_object,
                       std::ostream* err_stream));

  MOCK_METHOD3(
      CreateDbgObject,
      HRESULT(ICorDebugType* debug_type,
              std::unique_ptr<google_cloud_debugger::DbgObject>* result_object,
              std::ostream* err_stream));

  MOCK_METHOD7(
      EvaluateAndCreateDbgObject,
      HRESULT(std::vector<ICorDebugType*> generic_types,
              std::vector<ICorDebugValue*> argument_values,
              ICorDebugFunction *debug_function,
              ICorDebugEval *debug_eval,
              google_cloud_debugger::IEvalCoordinator *eval_coordinator,
              std::unique_ptr<google_cloud_debugger::DbgObject> *evaluate_result,
              std::ostream *err_stream));

  MOCK_METHOD5(
      CreateDbgObjectFromLiteralConst,
      HRESULT(const CorElementType &value_type, UVCP_CONSTANT literal_value,
              ULONG literal_value_len, ULONG64 *numerical_value,
              std::unique_ptr<google_cloud_debugger::DbgObject> *dbg_object));
};

}  // namespace google_cloud_debugger_test

#endif  //  I_DBG_OBJECT_FACTORY_MOCK_H_
