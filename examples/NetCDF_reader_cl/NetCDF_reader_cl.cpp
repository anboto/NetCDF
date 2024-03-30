// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>

using namespace Upp;

#include <NetCDF/NetCDF.h>
  
 
CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	const UVector<String>& command = CommandLine();
	
	try {
		if (command.IsEmpty()) 
			throw Exc("Please enter the .nc (NetCDF) file name in the command line");
			
		String file = command[0];
		
		NetCDFFile cdf(file);			
		UppLog() << Format("\nFile '%s' loaded", file);
		
		UppLog() << cdf.ToString();
	} catch (Exc err) {
		UppLog() << "\n" << Format(t_("Problem found: %s"), err);
		SetExitCode(-1);
	}
		
	UppLog() << "\nProgram ended\n";
	#ifdef flagDEBUG
	ReadStdIn();
	#endif
}

