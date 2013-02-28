/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "CrashWindow.h"
#include "ui_CrashReportViewer.h"
#include "CrashReporterSymbolSupplier.h"

CrashWindow::CrashWindow(QWidget* parent) : QDialog(parent)
{
    mCrashReporterUi = new Ui::UniboardCrashreporter();
    mCrashReporterUi->setupUi(this);
    mCrashReporterUi->crashReportResult->setFont(QFont("Monaco", 11));

    connect(mCrashReporterUi->viewReportButton, SIGNAL(clicked(bool)), this, SLOT(showReport()));
    connect(mCrashReporterUi->dumpFileButton, SIGNAL(clicked(bool)), this, SLOT(chooseDumpFile()));
    connect(mCrashReporterUi->symFileButton, SIGNAL(clicked(bool)), this, SLOT(chooseSymboleFile()));
}

CrashWindow::~CrashWindow()
{
    // NOOP
}

void CrashWindow::setDumpFilePath(const QString &fileName)
{
    mCrashReporterUi->dumpFilePath->setText(fileName);
}

void CrashWindow::chooseDumpFile()
{
    QSettings settings("Mnemis", "CrashReporter");
    QString dumpFileName = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    if (settings.contains("DumpFileName"))
    {
        dumpFileName = settings.value("DumpFileName").toString();
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
            dumpFileName,
            tr("Dump file (*.dmp)"));
    settings.setValue("DumpFileName", QVariant(fileName));
    setDumpFilePath(fileName);
}

void CrashWindow::chooseSymboleFile()
{
    QSettings settings("Mnemis", "CrashReporter");
    QString symFileName = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    if (settings.contains("SymFileName"))
    {
        symFileName = settings.value("SymFileName").toString();
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
            symFileName,
            tr("Symbol file (*.sym)"));
    settings.setValue("SymFileName", QVariant(fileName));
    mCrashReporterUi->symFilePath->setText(fileName);
}

//=============================================================================
int CrashWindow::PrintRegister(const char *name, u_int32_t value, int sequence) {
    if (sequence % 4 == 0) {
        mReport.append("\n");
    }
    // string should not exceed 200 byte!!!
    size_t BufSize = 2000;
    char buf[BufSize];

    snprintf(buf, BufSize, "%6s = 0x%08x ", name, value);
    QString str = QString::fromAscii(buf);
    mReport.append(str);
    return ++sequence;
}

//=============================================================================
void CrashWindow::PrintStack(const CallStack* stack, const string &cpu) {
  int frame_count = stack->frames()->size();
  char buffer[1024];

  for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
    const StackFrame *frame = stack->frames()->at(frame_index);
    const CodeModule *module = frame->module;
    // string should not exceed 200 byte!!!
    size_t BufSize = 2000;
    char buf[BufSize];
    snprintf(buf, BufSize, "%2d ", frame_index);
    mReport.append(QString::fromAscii(buf));

    if (module) {
      // Module name (20 chars max)
      strcpy(buffer, PathnameStripper::File(module->code_file()).c_str());
      int maxStr = 20;
      buffer[maxStr] = 0;
      snprintf(buf, BufSize, "%-*s", maxStr, buffer);
      mReport.append(QString::fromAscii(buf));


      strcpy(buffer, module->version().c_str());
      buffer[maxStr] = 0;
      snprintf(buf, BufSize, "%-*s",maxStr, buffer);
      mReport.append(QString::fromAscii(buf));

      u_int64_t instruction = frame->instruction;

      // PPC only: Adjust the instruction to match that of Crash reporter.  The
      // instruction listed is actually the return address.  See the detailed
      // comments in stackwalker_ppc.cc for more information.
      if (cpu == "ppc" && frame_index)
        instruction += 4;
      snprintf(buf, BufSize, " 0x%08llx ", instruction);
      mReport.append(QString::fromAscii(buf));

      // Function name
      if (!frame->function_name.empty()) {
          snprintf(buf, BufSize, "%s", frame->function_name.c_str());
          mReport.append(QString::fromAscii(buf));
        if (!frame->source_file_name.empty()) {
          string source_file = PathnameStripper::File(frame->source_file_name);
          snprintf(buf, BufSize, " + 0x%llx (%s:%d)",
                  instruction - frame->source_line_base,
                  source_file.c_str(), frame->source_line);
          mReport.append(QString::fromAscii(buf));
        } else {
            snprintf(buf, BufSize, " + 0x%llx", instruction - frame->function_base);
            mReport.append(QString::fromAscii(buf));
        }
      }
    }
    mReport.append("\n");
  }
}

//=============================================================================
void CrashWindow::PrintRegisters(const CallStack *stack, const string &cpu) {
  int sequence = 0;
  const StackFrame *frame = stack->frames()->at(0);
  if (cpu == "x86") {
    const StackFrameX86 *frame_x86 =
      reinterpret_cast<const StackFrameX86*>(frame);

    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EIP)
      sequence = PrintRegister("eip", frame_x86->context.eip, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_ESP)
      sequence = PrintRegister("esp", frame_x86->context.esp, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EBP)
      sequence = PrintRegister("ebp", frame_x86->context.ebp, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EBX)
      sequence = PrintRegister("ebx", frame_x86->context.ebx, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_ESI)
      sequence = PrintRegister("esi", frame_x86->context.esi, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EDI)
      sequence = PrintRegister("edi", frame_x86->context.edi, sequence);
    if (frame_x86->context_validity == StackFrameX86::CONTEXT_VALID_ALL) {
      sequence = PrintRegister("eax", frame_x86->context.eax, sequence);
      sequence = PrintRegister("ecx", frame_x86->context.ecx, sequence);
      sequence = PrintRegister("edx", frame_x86->context.edx, sequence);
      sequence = PrintRegister("efl", frame_x86->context.eflags, sequence);
    }
  } else if (cpu == "ppc") {
    const StackFramePPC *frame_ppc =
      reinterpret_cast<const StackFramePPC*>(frame);

    if (frame_ppc->context_validity & StackFramePPC::CONTEXT_VALID_ALL ==
        StackFramePPC::CONTEXT_VALID_ALL) {
      sequence = PrintRegister("srr0", frame_ppc->context.srr0, sequence);
      sequence = PrintRegister("srr1", frame_ppc->context.srr1, sequence);
      sequence = PrintRegister("cr", frame_ppc->context.cr, sequence);
      sequence = PrintRegister("xer", frame_ppc->context.xer, sequence);
      sequence = PrintRegister("lr", frame_ppc->context.lr, sequence);
      sequence = PrintRegister("ctr", frame_ppc->context.ctr, sequence);
      sequence = PrintRegister("mq", frame_ppc->context.mq, sequence);
      sequence = PrintRegister("vrsave", frame_ppc->context.vrsave, sequence);

      sequence = 0;
      char buffer[5];
      for (int i = 0; i < MD_CONTEXT_PPC_GPR_COUNT; ++i) {
        sprintf(buffer, "r%d", i);
        sequence = PrintRegister(buffer, frame_ppc->context.gpr[i], sequence);
      }
    } else {
      if (frame_ppc->context_validity & StackFramePPC::CONTEXT_VALID_SRR0)
        sequence = PrintRegister("srr0", frame_ppc->context.srr0, sequence);
      if (frame_ppc->context_validity & StackFramePPC::CONTEXT_VALID_GPR1)
        sequence = PrintRegister("r1", frame_ppc->context.gpr[1], sequence);
    }
  }

  mReport.append("\n");
}

void CrashWindow::PrintModules(const CodeModules *modules) {
  if (!modules)
    return;

  mReport.append("\n");
  mReport.append("Loaded modules:\n");

  u_int64_t main_address = 0;
  const CodeModule *main_module = modules->GetMainModule();
  if (main_module) {
    main_address = main_module->base_address();
  }

  unsigned int module_count = modules->module_count();
  for (unsigned int module_sequence = 0;
       module_sequence < module_count;
       ++module_sequence) {
    const CodeModule *module = modules->GetModuleAtSequence(module_sequence);
    assert(module);
    u_int64_t base_address = module->base_address();
    // string should not exceed 200 byte!!!
    size_t BufSize = 2000;
    char buf[BufSize];

    snprintf(buf, BufSize, "0x%08llx - 0x%08llx  %s  %s%s  %s\n",
               base_address, base_address + module->size() - 1,
               PathnameStripper::File(module->code_file()).c_str(),
               module->version().empty() ? "???" : module->version().c_str(),
               main_module != NULL && base_address == main_address ?
               "  (main)" : "",
               module->code_file().c_str());
    mReport.append(QString::fromAscii(buf));
  }
}

void CrashWindow::showReport()
{
    qDebug() << "compute report";
    mReport.clear();
    QString minidump_file = mCrashReporterUi->dumpFilePath->text();
    BasicSourceLineResolver resolver;

    scoped_ptr<CrashReporterSymbolSupplier> symbol_supplier(

            new CrashReporterSymbolSupplier(mCrashReporterUi->symFilePath->text()));
    scoped_ptr<MinidumpProcessor>
    minidump_processor(new MinidumpProcessor(symbol_supplier.get(), &resolver));
    ProcessState process_state;
    if (minidump_processor->Process(minidump_file.toStdString(), &process_state) !=
        MinidumpProcessor::PROCESS_OK) {
        mReport += "MinidumpProcessor::Process failed\n";
        return;
    }
    const SystemInfo *system_info = process_state.system_info();
    string cpu = system_info->cpu;
    // Convert the time to a string
    u_int32_t time_date_stamp = process_state.time_date_stamp();
    struct tm timestruct;
    gmtime_r(reinterpret_cast<time_t*>(&time_date_stamp), &timestruct);
    char timestr[20];
    // string should not exceed 200 byte!!!
    size_t BufSize = 2000;
    char buf[BufSize];
    strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", &timestruct);
    snprintf(buf, BufSize, "Date: %s GMT\n", timestr);
    mReport.append(QString::fromAscii(buf));
    snprintf(buf, BufSize, "Operating system: %s (%s)\n", system_info->os.c_str(),
            system_info->os_version.c_str());
    mReport.append(QString::fromAscii(buf));
    snprintf(buf, BufSize, "Architecture: %s\n", cpu.c_str());
    mReport.append(QString::fromAscii(buf));

    if (process_state.crashed()) {
        snprintf(buf, BufSize, "Crash reason:  %s\n", process_state.crash_reason().c_str());
        mReport.append(QString::fromAscii(buf));
        snprintf(buf, BufSize, "Crash address: 0x%llx\n", process_state.crash_address());
        mReport.append(QString::fromAscii(buf));
    } else {
        mReport.append("No crash\n");
    }

    int requesting_thread = process_state.requesting_thread();
    if (requesting_thread != -1) {
        mReport.append("\n");
        snprintf(buf, BufSize, "Thread %d (%s)\n",
                requesting_thread,
                process_state.crashed() ? "crashed" :
        "requested dump, did not crash");
        mReport.append(QString::fromAscii(buf));
        PrintStack(process_state.threads()->at(requesting_thread), cpu);
    }

    // Print all of the threads in the dump.
    int thread_count = process_state.threads()->size();
    for (int thread_index = 0; thread_index < thread_count; ++thread_index) {
        if (thread_index != requesting_thread) {
            // Don't print the crash thread again, it was already printed.
            mReport.append("\n");
            snprintf(buf, BufSize,"Thread %d\n", thread_index);
            mReport.append(QString::fromAscii(buf));
            PrintStack(process_state.threads()->at(thread_index), cpu);
        }
    }

    // Print the crashed registers
    if (requesting_thread != -1) {
        snprintf(buf, BufSize,"\nThread %d:", requesting_thread);
        mReport.append(QString::fromAscii(buf));
        PrintRegisters(process_state.threads()->at(requesting_thread), cpu);
    }

    // Print information about modules
    PrintModules(process_state.modules());
    mCrashReporterUi->crashReportResult->setPlainText(mReport);
}


