// ConsoleApplication1.cpp : Defines the entry point for the console
// application.

// TODO: Add cleanup to release pointer.

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "dbgobject.h"
#include "debugger.h"
#include "optionparser.h"
#include "winerror.h"

using google_cloud_debugger::ConvertStringToWCharPtr;
using google_cloud_debugger::Debugger;
using std::cerr;
using std::cin;
using std::endl;
using std::hex;
using std::string;

// If given this option, the debugger will not perform property evaluation.
const string kEvaluationOption = "property-evaluation";

// If given this option, the debugger will start the application using this
// path.
const string kApplicationPathOption = "application-path";

// If given this option, the debugger will attach to the running application
// with this process ID.
const string kApplicationIDOption = "application-id";

enum optionIndex {
  UNKNOWN,
  APPLICATIONPATH,
  APPLICATIONID,
  PROPERTYEVALUATION
};
const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", option::Arg::None,
     "USAGE: example [options]\n\n"
     "Options:"},
    {APPLICATIONPATH, 0, "", kApplicationPathOption.c_str(),
     option::Arg::Optional,
     "  --application-path  \tPath to the application. If used, the debugger "
     "will start the application using this path."},
    {APPLICATIONID, 0, "", kApplicationIDOption.c_str(), option::Arg::Optional,
     "  --application-id  \tProcess ID of the application to be debugged. If "
     "used, the debugger will attach to the running application using this "
     "ID."},
    {PROPERTYEVALUATION, 0, "", kEvaluationOption.c_str(), option::Arg::None,
     "  --property-evaluation  \tIf used, the debugger will attempt to "
     "evaluate property of classes. This may modify the state of the "
     "application"},
    {0, 0, 0, 0, 0, 0}  // Needs this, otherwise the parser throws error.
};

int main(int argc, char *argv[]) {
  if (argc > 0) {
    // Skips first argument.
    argc -= 1;
    argv += 1;
  }

  // Initializing codes from http://optionparser.sourceforge.net/index.html
  // example.
  option::Stats stats(usage, argc, argv);
  std::vector<option::Option> options(stats.options_max);
  std::vector<option::Option> buffer(stats.options_max);

  option::Parser parse(usage, argc, argv, options.data(), buffer.data());

  if (parse.error()) return 1;

  if (argc == 0) {
    option::printUsage(std::cout, usage);
    return 0;
  }

  bool property_evaluation = options[PROPERTYEVALUATION].count();

  // Has to supply either path or ID, not both.
  if ((options[APPLICATIONPATH].count() && options[APPLICATIONID].count()) ||
      (!options[APPLICATIONPATH].count() && !options[APPLICATIONID].count())) {
    cerr << "The debugger can only take in either the application path or the "
            "process ID of the running application, not both.";
    return -1;
  }

  Debugger debugger;
  HRESULT hr;

  if (options[APPLICATIONPATH].count()) {
    string app_path = string(options[APPLICATIONPATH].arg);
    string command_line = "dotnet " + app_path;
    std::vector<WCHAR> wchar_command_line =
        ConvertStringToWCharPtr(command_line);

    if (wchar_command_line.size() == 0) {
      cerr << "Application's name is not valid." << endl;
      return -1;
    }

    hr = debugger.StartDebugging(wchar_command_line);
  } else {
    try {
      int app_id = stoi(string(options[APPLICATIONID].arg));
      if (app_id < 0) {
        cerr << "Application ID has to be a positive.";
      }
      hr = debugger.StartDebugging(app_id);
    } catch (std::invalid_argument &ex) {
      cerr << "Application ID is not a valid positive number.";
      return -1;
    }
  }

  if (FAILED(hr)) {
    cerr << "Debugger fails with HRESULT " << hex << hr << endl;
    return -1;
  }

  // Sets property evaluation.
  debugger.SetPropertyEvaluation(property_evaluation);

  // This will launch an infinite while loop to wait and read.
  // When the server connection of the named pipe breaks, the loop
  // will be broken and the application process will be terminated
  // in the debugger's destructor.
  debugger.SyncBreakpoints();

  return 0;
}
