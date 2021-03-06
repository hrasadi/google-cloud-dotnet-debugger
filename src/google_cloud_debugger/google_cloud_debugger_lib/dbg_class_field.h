// Copyright 2017 Google Inc. All Rights Reserved.
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

#ifndef DBG_CLASS_FIELD_H_
#define DBG_CLASS_FIELD_H_

#include <memory>
#include <vector>

#include "dbg_object.h"
#include "i_dbg_class_member.h"

namespace google_cloud_debugger {

// Class that represents a field in a .NET class.
class DbgClassField : public IDbgClassMember {
 public:
  // fieldDef is the metadata token for the field.
  // class_type represents the type of the class.
  // Creation_depth sets the depth used when creating a DbgObject
  // representing this field.
  DbgClassField(mdFieldDef field_def, int creation_depth,
                ICorDebugType *class_type,
                std::shared_ptr<ICorDebugHelper> debug_helper,
                std::shared_ptr<IDbgObjectFactory> obj_factory);

  // Initialize the field names, metadata signature, flags and values.
  // HRESULT will be stored in initialized_hr_.
  // metadata_import is used to extract metadata from the field.
  // debug_obj_value represents the class object.
  // debug_class represents the class itself.
  void Initialize(ICorDebugModule *debug_module,
                  IMetaDataImport *metadata_import,
                  ICorDebugObjectValue *debug_obj_value,
                  ICorDebugClass *debug_class);

  // Evaluates and sets member_value_ to the value of the field
  // that is represented by this class.
  // Reference_value and generic_types are ignored.
  HRESULT Evaluate(ICorDebugValue *debug_value,
                   IEvalCoordinator *eval_coordinator,
                   std::vector<CComPtr<ICorDebugType>> *generic_types) override;

  // Returns true if this is a backing field for a property.
  const bool IsBackingField() const { return is_backing_field_; }

  // Returns true if this is a static field.
  bool IsStatic() const override { return IsFdStatic(member_attributes_); }

 private:
  // Processes the case where field is a constant literal.
  HRESULT ProcessConstField(ICorDebugModule *debug_module,
                            IMetaDataImport *metadata_import);

  // Processes the case where field is a const enum literal.
  // enum_type is the underlying numerical type of the enum
  // and enum_numerical_value is the underlying numerical value of the enum.
  HRESULT ProcessConstEnumField(ICorDebugModule *debug_module,
                                IMetaDataImport *metadata_import,
                                const CorElementType &enum_type,
                                ULONG64 enum_numerical_value);

  // Extracts out static field value (with name field_name_) using the
  // ICorDebugValue class_value that represents the class object (may be null
  // since this is a static field). Depth of the static field object will
  // be creation_depth_.
  HRESULT ExtractStaticFieldValue(IEvalCoordinator *eval_coordinator);

  // Token that represents this field.
  mdFieldDef field_def_ = 0;

  // True if this is a backing field for a property.
  bool is_backing_field_ = false;

  // True if field is an enum.
  bool is_enum_ = false;

  // Debug type of the class that this field belongs to.
  CComPtr<ICorDebugType> class_type_;
};

}  //  namespace google_cloud_debugger

#endif  //  DBG_CLASS_FIELD_H_
