// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2023, the Anboto author and contributors
#ifndef _hdf5_h_
#define _hdf5_h_

#include <plugin/Hdf5/src/hdf5.h>
#include <Eigen/Eigen.h>
#include <Eigen/MultiDimMatrixIndex.h>

#include <plugin/NetCDF/include/config.h>
#include <plugin/NetCDF/include/netcdf.h>

namespace Upp {

class NetCDFFile {
public:
	NetCDFFile()				{}
	NetCDFFile(const char *file){Open(file);}
	~NetCDFFile()				{Close();}	
	
	void Create(const char *file);		
	void Open(const char *file);
	bool IsOpened();
	void Close();

	String GetFileFormat();

	Vector<String> ListGroup();
	Vector<String> ListGlobalAttributes();
	Vector<String> ListVariables();
	Vector<String> ListAttributes(const char *name);
	
	bool ChangeGroup(const char *group);
	bool CreateGroup(const char *group, bool change = false);
	
	int GetId(const char *name);
	
	void GetVariableData(const char *name, nc_type &type, Vector<int> &dims);
	
	String GetVariableString(const char *name);
	int GetInt(const char *name);
	float GetFloat(const char *name);
	double GetDouble(const char *name);
	String GetString(const char *name);
	void GetDouble(const char *name, Eigen::VectorXd &data);
	void GetDouble(const char *name, Vector<double> &data);
	void GetDouble(const char *name, Eigen::MatrixXd &data);
	void GetDouble(const char *name, MultiDimMatrixRowMajor<double> &d);
	template <int Rank>
	void GetDouble(const char *name, Eigen::Tensor<double, Rank> &data) {
		lastvarid = GetId(name);
		nc_type type;
		Vector<int> dims;
		GetVariableData(lastvarid, type, dims);
		
		if (type != NC_DOUBLE)
			throw Exc(Format("Data is not double. Found %s", TypeName(type)));	
	
		if (dims.size() != Rank)
			throw Exc(Format("Wrong number of dimensions in GetDouble(). Found %d", dims.size()));
		
		int sz = 1;
		for (int n : dims)
			sz *= n;
	
		Buffer<double> d_row(sz), d_col(sz);
		if ((retval = nc_get_var_double(ncid, lastvarid, d_row.Get())))
    		throw Exc(nc_strerror(retval));	

		RowMajorToColMajor(~d_row, ~d_col, dims);
		
		Eigen::array<Eigen::Index, Rank> dimensions;
		for (int i = 0; i < Rank; ++i)
			dimensions[i] = dims[i];

		data = Eigen::TensorMap<Eigen::Tensor<double, Rank>>(~d_col, dimensions);		
	}
	void GetInt(const char *name, Vector<int> &data);
	void GetFloat(const char *name, Vector<float> &data);
	void GetString(const char *name, Vector<String> &data);
		
	NetCDFFile &Set(const char *name, int d);
	NetCDFFile &Set(const char *name, double d);
	NetCDFFile &Set(const char *name, const char *d);
	NetCDFFile &Set(const char *name, const Eigen::VectorXd &d);
	NetCDFFile &Set(const char *name, const Vector<double> &d);
	NetCDFFile &Set(const char *name, const Eigen::MatrixXd &d);
	NetCDFFile &Set(const char *name, const MultiDimMatrixRowMajor<double> &d);
	template <int Rank>
	NetCDFFile &Set(const char *name, const Eigen::Tensor<double, Rank> &d);
		
	NetCDFFile &SetAttribute(const char *name, int d);
	NetCDFFile &SetAttribute(const char *name, double d);
	NetCDFFile &SetAttribute(const char *name, const char *d);
	
	void SetGlobalAttributes()	{lastvarid = NC_GLOBAL;}
	nc_type GetAttributeType(const char *name);
	int GetAttributeInt(const char *name);
	float GetAttributeFloat(const char *name);
	double GetAttributeDouble(const char *name);
	String GetAttributeString(const char *name);
	
	String GetLastError();
	
	static String TypeName(nc_type type);
	
	String ToString();

private:
	int ncid = -1;
	int lastvarid = -1;
	int retval;
	
	String GetAttributeString0(const char *name, int len);
	void GetVariableData(int id, nc_type &type, Vector<int> &dims);
	void GetVariableData0(int id, nc_type &type, Vector<int> &dims);
	String GetName(int id);
};

}
	
#endif

