/*
 * CrashWindow.h
 *
 *  Created on: Mar 13, 2009
 *      Author: julienbachmann
 */

#ifndef CRASHWINDOW_H_
#define CRASHWINDOW_H_

#include <QtGui>
#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/call_stack.h"
#include "google_breakpad/processor/code_module.h"
#include "google_breakpad/processor/minidump.h"
#include "google_breakpad/processor/minidump_processor.h"
#include "google_breakpad/processor/process_state.h"
#include "google_breakpad/processor/stack_frame_cpu.h"
#include "google_breakpad/processor/system_info.h"
#include "processor/pathname_stripper.h"
#include "processor/scoped_ptr.h"
#include "processor/simple_symbol_supplier.h"

using std::string;

using google_breakpad::BasicSourceLineResolver;
using google_breakpad::CallStack;
using google_breakpad::CodeModule;
using google_breakpad::CodeModules;
using google_breakpad::MinidumpProcessor;
using google_breakpad::PathnameStripper;
using google_breakpad::ProcessState;
using google_breakpad::scoped_ptr;
using google_breakpad::StackFrame;
using google_breakpad::StackFramePPC;
using google_breakpad::StackFrameX86;
using google_breakpad::SystemInfo;

namespace Ui
{
    class UniboardCrashreporter;
}

class CrashWindow : public QDialog
{
	Q_OBJECT;
public:
	CrashWindow(QWidget* parent = 0);
	virtual ~CrashWindow();

    void setDumpFilePath(const QString &fileName);

private slots:
	void showReport();
	void chooseDumpFile();
	void chooseSymboleFile();

private:
	int PrintRegister(const char *name, u_int32_t value, int sequence);
	void PrintStack(const CallStack* stack, const string &cpu);
	void PrintRegisters(const CallStack *stack, const string &cpu);
	void PrintModules(const CodeModules *modules);
    Ui::UniboardCrashreporter* mCrashReporterUi;
    QString mReport;
};

#endif /* CRASHWINDOW_H_ */
