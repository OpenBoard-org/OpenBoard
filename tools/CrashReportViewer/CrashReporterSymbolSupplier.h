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


#ifndef CRASHREPORTERSYMBOLSUPPLIER_H_
#define CRASHREPORTERSYMBOLSUPPLIER_H_

#include <QtGui>
#include "google_breakpad/processor/symbol_supplier.h"

using std::string;

class CrashReporterSymbolSupplier : public google_breakpad::SymbolSupplier
{
public:
    CrashReporterSymbolSupplier(QString pUniboardSymFiles);
    virtual ~CrashReporterSymbolSupplier() {}

    // Returns the path to the symbol file for the given module.
    SymbolResult GetSymbolFile(const google_breakpad::CodeModule *module,
            const google_breakpad::SystemInfo *system_info,
            string *symbol_file);

    // Returns the path to the symbol file for the given module.
    SymbolResult GetSymbolFile(const google_breakpad::CodeModule *module,
            const google_breakpad::SystemInfo *system_info,
            string *symbol_file,
            string *symbol_data);

private:
    QFile mUniboardSymFile;

};
#endif /* CRASHREPORTERSYMBOLSUPPLIER_H_ */
