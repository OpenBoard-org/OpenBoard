/*
 * CrashReporterSymbolSupplier.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: julienbachmann
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
