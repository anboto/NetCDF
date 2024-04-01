// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2024, the Anboto author and contributors
#include <Core/Core.h>

using namespace Upp;

#include <NetCDF/NetCDF.h>
  

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	try {
		UppLog() << "\nNetCDF wrapper test\n";
		
		auto Test = [=](const char *file, int format = 0) {
			{
				NetCDFFile cdf;
				
				cdf.Create(file, format);
				
				cdf.Set("number_double_root", 14.5).SetAttribute("description", "This is a double in the root");
				
				cdf.CreateGroup("simulation_parameters", true);
				
				cdf.Set("number_integer", 23).SetAttribute("description", "This is an integer");
				cdf.Set("number_double", 24.5).SetAttribute("description", "This is a double");
				cdf.Set("text", "hello").SetAttribute("description", "This is a string");
				Eigen::MatrixXd a(2, 3);
				a << 1, 2, 3, 11, 22, 33;
				cdf.Set("matrix_double", a).SetAttribute("description", "This is matrix of double");
				Eigen::Tensor<double, 4> m(2, 3, 7, 1);
				m(0, 2, 5, 0) = 123.45;
				cdf.Set<4>("multi_matrix", m);
			}
			{
				NetCDFFile cdf;
				
				cdf.Open(file);
				
				double d0 = cdf.GetDouble("number_double_root");
				VERIFY(d0 == 14.5);
				
				cdf.ChangeGroup("simulation_parameters");
				
				int in = cdf.GetInt("number_integer");
				VERIFY(in == 23);
				double d = cdf.GetDouble("number_double");
				VERIFY(d == 24.5);
				String s = cdf.GetString("text");
				VERIFY(s == "hello");
				Eigen::MatrixXd m;
				cdf.GetDouble("matrix_double", m);
				VERIFY(m(1, 1) == 22);
				VERIFY(m(1, 2) == 33);
				MultiDimMatrixIndex icol(2,3, 7, 1);
				int ic = icol(0, 2, 5, 0);
				MultiDimMatrixIndexRowMajor irow(2, 3, 7, 1);
				int ir = irow(0, 2, 5, 0);
				MultiDimMatrixRowMajor<double> b;
				cdf.GetDouble("multi_matrix", b);
				VERIFY(b(0, 2, 5, 0) == 123.45);
				
				UppLog() << "\n" << cdf.ToString();
				
				UppLog() << "\nAll tests OK\n";
			}
		};
		Test("datalib.nc");
		Test("datalib_hdf5.nc", NC_NETCDF4);
	} catch (Exc err) {
		UppLog() << "\n" << Format(t_("Problem found: %s"), err);
		SetExitCode(-1);
	}
		
	UppLog() << "\nProgram ended\n";
	#ifdef flagDEBUG
	ReadStdIn();
	#endif
}

