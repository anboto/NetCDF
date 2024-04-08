// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2024, the Anboto author and contributors
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
	
	void Create(const char *file, int format = 0);		
	void Open(const char *file);
	bool IsOpened();
	void Close();

	String GetFileFormat();

	const Vector<String> &ListGroups() const;
	Vector<String> ListGlobalAttributes();
	Vector<String> ListVariables();
	Vector<String> ListAttributes(const char *name);
	
	void ChangeGroup(const char *group);
	void ChangeGroupRoot();
	void ChangeGroupUp();
	void CreateGroup(const char *group, bool change = false);
	
	bool ExistVar(const char *name);
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
	NetCDFFile &Set(const char *name, const Eigen::Tensor<double, Rank> &d) {
		if ((retval = nc_redef(ncid)))
    		throw Exc(nc_strerror(retval));	
			
		String namedim;
		Buffer<int> dimids(Rank);
		for (int i = 0; i < Rank; ++i) {
			namedim = Format("%s_%d", name, i);
			if ((retval = nc_def_dim(ncid, ~namedim, d.dimension(i), &dimids[i])))
		       	throw Exc(nc_strerror(retval));	
		}
		int varid;
		if ((retval = nc_def_var(ncid, name, NC_DOUBLE, Rank, dimids, &varid)))
	    	throw Exc(nc_strerror(retval));	
	    	 
	    if ((retval = nc_enddef(ncid)))
	    	throw Exc(nc_strerror(retval));	
	
		int sz = 1;
		Vector<int> dimensions(Rank);
		for (int i = 0; i < Rank; ++i) { 
			sz *=  d.dimension(i);
			dimensions[i] = int(d.dimension(i));
		}
		Buffer<double> d_row(sz);
		ColMajorToRowMajor(d.data(), ~d_row, dimensions);
		  
	    if ((retval = nc_put_var_double(ncid, varid, ~d_row)))
	    	throw Exc(nc_strerror(retval));	
		
		lastvarid = varid;
		
		return *this;
	}
		
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
	int fileid = -1;
	int lastvarid = -1;
	int retval;
	Vector<int> groupIds;
	Vector<String> groupNames;
	Vector<int> groupPathIds;
	bool allowGroups = false;
	
	String GetAttributeString0(const char *name, int len);
	void GetVariableData(int id, nc_type &type, Vector<int> &dims);
	void GetVariableData0(int id, nc_type &type, Vector<int> &dims);
	String GetName(int id);
	void ChangeGroup(int group_id);
	String ToString0();
};

}
	
#endif

