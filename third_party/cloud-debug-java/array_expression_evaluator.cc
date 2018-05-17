/**
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "array_expression_evaluator.h"

#include "error_messages.h"
#include "compiler_helpers.h"
#include "dbg_array.h"
#include "i_dbg_object_factory.h"
#include "constants.h"

namespace google_cloud_debugger {

IndexerAccessExpressionEvaluator::IndexerAccessExpressionEvaluator(
    std::unique_ptr<ExpressionEvaluator> source_collection,
    std::unique_ptr<ExpressionEvaluator> source_index)
    : source_collection_(std::move(source_collection)),
      source_index_(std::move(source_index)) {
}

HRESULT IndexerAccessExpressionEvaluator::Compile(
    IDbgStackFrame *stack_frame,
    ICorDebugILFrame *debug_frame,
    std::ostream *err_stream) {
  HRESULT hr = source_collection_->Compile(stack_frame, debug_frame,
                                           err_stream);
  if (FAILED(hr)) {
    return hr;
  }

  hr = source_index_->Compile(stack_frame, debug_frame, err_stream);
  if (FAILED(hr)) {
    return hr;
  }

  const TypeSignature &source_type = source_collection_->GetStaticType();

  // TODO(quoct): Implement logic for a[b] where a is not an array.
  if (!source_type.is_array) {
    return E_NOTIMPL;
  }

  // Type of each element in the array.
  if (source_type.generic_types.size() != 1) {
    *err_stream << "Cannot find type of each array element.";
    return E_FAIL;
  }

  return_type_ = source_type.generic_types[0];
  return S_OK;
}

HRESULT IndexerAccessExpressionEvaluator::Evaluate(
      std::shared_ptr<DbgObject> *dbg_object,
      IEvalCoordinator *eval_coordinator,
      IDbgObjectFactory *obj_factory,
      std::ostream *err_stream) const {
  std::shared_ptr<DbgObject> source_obj;
  HRESULT hr = source_collection_->Evaluate(&source_obj, eval_coordinator,
                                            obj_factory, err_stream);
  if (FAILED(hr)) {
    return hr;
  }

  if (source_obj->GetIsNull()) {
    return E_FAIL;
  }

  std::shared_ptr<DbgObject> index_obj;
  hr = source_index_->Evaluate(&index_obj, eval_coordinator,
                               obj_factory, err_stream);
  if (FAILED(hr)) {
    return hr;
  }

  // In the case that the source is an array, we retrieve the index as a long
  // and use that index to access the item in the array.
  if (TypeCompilerHelper::IsArrayType(source_obj->GetCorElementType())) {
    int64_t index;
    hr = NumericCompilerHelper::ExtractPrimitiveValue<int64_t>(index_obj.get(), &index);
    if (FAILED(hr)) {
      return hr;
    }

    DbgArray *array_obj = dynamic_cast<DbgArray *>(source_obj.get());
    if (array_obj == nullptr) {
      return E_INVALIDARG;
    }

    CComPtr<ICorDebugValue> array_item;
    hr = array_obj->GetArrayItem(index, &array_item);
    if (FAILED(hr)) {
      return hr;
    }

    std::unique_ptr<DbgObject> target_object;
    std::ostringstream err_stream;
    hr = obj_factory->CreateDbgObject(array_item, kDefaultObjectEvalDepth,
        &target_object, &err_stream);
    if (FAILED(hr)) {
      return hr;
    }

    *dbg_object = std::move(target_object);
    return hr;
  }

  // TODO(quoct): Implement logic for accessing item in Dictionary, List, etc.
  // We can do this by calling get_Item function.
  return E_NOTIMPL;
}


}  // namespace google_cloud_debugger