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


#include <string>
#include <iostream>
#include <fstream>

#include "CrashReporterSymbolSupplier.h"
#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/minidump.h"
#include "google_breakpad/processor/system_info.h"
#include "processor/pathname_stripper.h"



CrashReporterSymbolSupplier::CrashReporterSymbolSupplier(QString pUniboardSymFiles) {
    mUniboardSymFile.setFileName(pUniboardSymFiles);
}
// Returns the path to the symbol file for the given module.
google_breakpad::SymbolSupplier::SymbolResult CrashReporterSymbolSupplier::GetSymbolFile(const google_breakpad::CodeModule *module,
        const google_breakpad::SystemInfo *system_info,
        string *symbol_file)
{
    QString moduleName = QString::fromStdString(module->code_file());
    if (moduleName.contains("Uniboard"))
    {
        *symbol_file = mUniboardSymFile.fileName().toStdString();
        return FOUND;
    }
    return NOT_FOUND;
}

// Returns the path to the symbol file for the given module.
google_breakpad::SymbolSupplier::SymbolResult CrashReporterSymbolSupplier::GetSymbolFile(const google_breakpad::CodeModule *module,
        const google_breakpad::SystemInfo *system_info,
        string *symbol_file,
        string *symbol_data)
{
    SymbolSupplier::SymbolResult s = GetSymbolFile(module,
            system_info,
            symbol_file);


    if (s == FOUND) {
        std::ifstream in(symbol_file->c_str());
        getline(in, *symbol_data, std::string::traits_type::to_char_type(
                    std::string::traits_type::eof()));
        in.close();
    }

    return s;
}
