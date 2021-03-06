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

#ifndef BREAKPOINT_CLIENT_H_
#define BREAKPOINT_CLIENT_H_

#include <cor.h>
#include <windef.h>
#include <iostream>
#include <mutex>
#include <string>

#include "dbg_breakpoint.h"
#include "constants.h"
#include "i_named_pipe.h"

namespace google_cloud_debugger {

class BreakpointClient {
 public:
  // Creates a breakpoint client and accepts a NamedPipeClient.
  BreakpointClient(std::unique_ptr<INamedPipe> pipe);

  // Initializes the client and returns an HRESULT.
  HRESULT Initialize();

  // Waits to connect to a server.  This function will
  // block until a connection is made or until a timeout
  // occurs.
  HRESULT WaitForConnection();

  // Reads a breakpoint from a breakpoint server and
  // returns an HRESULT.  This function will
  // block until there is a breakpoint to read.
  HRESULT ReadBreakpoint(
      google::cloud::diagnostics::debug::Breakpoint *breakpoint);

  // Writes a breakpoint to a breakpoint server
  // and returns an HRESULT.
  HRESULT WriteBreakpoint(
      const google::cloud::diagnostics::debug::Breakpoint &breakpoint);

  // Shuts down the pipe.
  HRESULT ShutDown();

 private:
  // The pipe client to send messages.
  std::unique_ptr<INamedPipe> pipe_;

  // A buffer to hold partial breakpoint messages.
  std::string buffer_;

  // Mutex to protect the buffer.
  std::mutex mutex_;
};

}  // namespace google_cloud_debugger

#endif  //  BREAKPOINT_CLIENT_H_
