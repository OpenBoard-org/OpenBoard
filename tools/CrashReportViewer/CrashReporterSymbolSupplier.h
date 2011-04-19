/*
 * CrashReporterSymbolSupplier.h
 *
 *  Created on: Mar 17, 2009
 *      Author: julienbachmann
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
