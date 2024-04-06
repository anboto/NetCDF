// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2024, the Anboto author and contributors
#include <Core/Core.h>
#include <Eigen/Eigen.h>
#include <Functions4U/Functions4U.h>

#include "NetCDF.h"

namespace Upp {

void NetCDFFile::Open(const char *file) {
	Close();
	
	if (!FileExists(file))
		throw Exc(Format("File '%s' does not exist", file));

    if ((retval = nc_open(file, NC_NOWRITE, &ncid)))
       throw Exc(nc_strerror(retval)); 
    fileid = ncid;
 
  	int format;
    if ((retval = nc_inq_format(ncid, &format)))
    	throw Exc(nc_strerror(retval));
    
    allowGroups = format == NC_FORMAT_NETCDF4 || format == NC_FORMAT_NETCDF4_CLASSIC;
	
	ChangeGroupRoot();
}

bool NetCDFFile::IsOpened() {
	return fileid >= 0;
}

void NetCDFFile::Create(const char *file, int format) {
	Close();
	
    if ((retval = nc_create(file, NC_CLOBBER | format, &ncid)))
       throw Exc(nc_strerror(retval)); 
    fileid = ncid;
    
    allowGroups = format == NC_NETCDF4;
 
    ChangeGroupRoot();
    
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
}
    
void NetCDFFile::Close() {
	if (IsOpened() && (retval = nc_close(fileid)))
       throw Exc(nc_strerror(retval)); 
	fileid = ncid = -1;
}

String NetCDFFile::GetFileFormat() {
  	int format;
    if ((retval = nc_inq_format(ncid, &format)))
    	throw Exc(nc_strerror(retval)); 

    switch (format) {
    case NC_FORMAT_CLASSIC:			return "Classic (v1)";
    case NC_FORMAT_64BIT_OFFSET:	return "64-bit offset (v2)";
    case NC_FORMAT_64BIT_DATA:		return "64-bit data (v2)";
    case NC_FORMAT_NETCDF4:			return "NetCDF-4 (HDF5)";
    case NC_FORMAT_NETCDF4_CLASSIC:	return "NetCDF-4 Classic (HDF5)";
    default:						return Format("Unknown format %d", format);
    }
}
	
String NetCDFFile::GetAttributeString(const char *name) {
	size_t att_len;
	nc_type type;
	if ((retval = nc_inq_att(ncid, lastvarid, name, &type, &att_len))) 
    	throw Exc(nc_strerror(retval)); 

	switch(type) {
	case NC_CHAR:	return GetAttributeString0(name, int(att_len));
	case NC_BYTE:
	case NC_SHORT:
	case NC_INT:	return FormatInt(GetAttributeInt(name));
	case NC_FLOAT:	return FormatDouble(GetAttributeFloat(name));
	case NC_DOUBLE:	return FormatDouble(GetAttributeDouble(name));
	default:		return NetCDFFile::TypeName(type);
	}
}
	
String NetCDFFile::GetAttributeString0(const char *name, int len) {
	StringBuffer bstr((int)len);
	if ((retval = nc_get_att_text(ncid, lastvarid, name, ~bstr)))
		throw Exc(nc_strerror(retval)); 	 
	return String(bstr);
}

int NetCDFFile::GetAttributeInt(const char *name) {
	int ret;
	if ((retval = nc_get_att_int(ncid, lastvarid, name, &ret))) 
		throw Exc(nc_strerror(retval)); 	
	return ret;
}

float NetCDFFile::GetAttributeFloat(const char *name) {
	float ret;
	if ((retval = nc_get_att_float(ncid, lastvarid, name, &ret))) 
		throw Exc(nc_strerror(retval)); 	
	return ret;
}

double NetCDFFile::GetAttributeDouble(const char *name) {
	double ret;
	if ((retval = nc_get_att_double(ncid, lastvarid, name, &ret))) 
		throw Exc(nc_strerror(retval)); 	
	return ret;
}

NetCDFFile &NetCDFFile::SetAttribute(const char *name, int d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
    
    if ((retval = nc_put_att_int(ncid, lastvarid, name, NC_INT, 1, &d)))
		throw Exc(nc_strerror(retval)); 
   
   	if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
   
	return *this;
}

NetCDFFile &NetCDFFile::SetAttribute(const char *name, double d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
    
    if ((retval = nc_put_att_double(ncid, lastvarid, name, NC_DOUBLE, 1, &d)))
		throw Exc(nc_strerror(retval)); 
    
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
    
	return *this;
}

NetCDFFile &NetCDFFile::SetAttribute(const char *name, const char *d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
	    
    if ((retval = nc_put_att_text(ncid, lastvarid, name, strlen(d), d)))
        throw Exc(nc_strerror(retval)); 
    
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
        
	return *this;
}
							
String NetCDFFile::TypeName(nc_type type) {
    switch (type) {
    case NC_NAT: 	return "NC_NAT";
    case NC_BYTE: 	return "NC_BYTE";
    case NC_CHAR: 	return "NC_CHAR";
    case NC_SHORT: 	return "NC_SHORT";
    case NC_INT: 	return "NC_INT";
    case NC_FLOAT: 	return "NC_FLOAT";
    case NC_DOUBLE: return "NC_DOUBLE";
    case NC_UBYTE: 	return "NC_UBYTE";
    case NC_USHORT: return "NC_USHORT";
    case NC_UINT: 	return "NC_UINT";
    case NC_INT64: 	return "NC_INT64";
    case NC_UINT64: return "NC_UINT64";
    case NC_STRING: return "NC_STRING";
    case NC_VLEN: 	return "NC_VLEN";
    case NC_OPAQUE: return "NC_OPAQUE";
    case NC_ENUM: 	return "NC_ENUM";
    case NC_COMPOUND:return "NC_COMPOUND";
    default: 		return Format("Type %d is unknown", type);
    }
}

Vector<String> NetCDFFile::ListGlobalAttributes() {
	int ngatts_in;
   
    if ((retval = nc_inq(ncid, NULL, NULL, &ngatts_in, NULL)))
 		throw Exc(nc_strerror(retval)); 
    
    Vector<String> ret(ngatts_in);	
    
    char att_name[256];
    
    for (int i = 0; i < ngatts_in; i++) {
        if ((retval = nc_inq_attname(ncid, NC_GLOBAL, i, att_name))) 
            throw Exc(nc_strerror(retval)); 
        ret[i] = att_name;
    }
    SetGlobalAttributes();
    return ret;
}

nc_type NetCDFFile::GetAttributeType(const char *name) {
    size_t att_len;
    nc_type att_type;
    
	if ((retval = nc_inq_att(ncid, lastvarid, name, &att_type, &att_len))) 
    	throw Exc(nc_strerror(retval)); 

	return att_type;            	
}

Vector<String> NetCDFFile::ListVariables() {
	int nvars;
	if ((retval = nc_inq(ncid, NULL, &nvars, NULL, NULL)))
 		throw Exc(nc_strerror(retval)); 
 			
    Vector<int> ids(nvars);
    if ((retval = nc_inq_varids(ncid, &nvars, ids.begin()))) 
        throw Exc(nc_strerror(retval)); 
    
    Vector<String> ret(nvars);
    for (int i = 0; i < nvars; ++i)
        ret[i] = GetName(ids[i]);
    return ret;
}

String NetCDFFile::GetName(int id) {
    char var_name[NC_MAX_NAME];
    if ((retval = nc_inq_varname(ncid, id, var_name))) 
		throw Exc(nc_strerror(retval)); 
    return String(var_name);
}

int NetCDFFile::GetId(const char *name) {
	int ret;
    if ((retval = nc_inq_varid(ncid, name, &ret)))
    	throw Exc(nc_strerror(retval)); 
    return ret;
}

int NetCDFFile::GetInt(const char *name) {
	lastvarid = GetId(name);
	int ret;
	if ((retval = nc_get_var_int(ncid, lastvarid, &ret)))	
		throw Exc(nc_strerror(retval)); 
	return ret;
}

float NetCDFFile::GetFloat(const char *name) {
	lastvarid = GetId(name);
	float ret;
	if ((retval = nc_get_var_float(ncid, lastvarid, &ret)))	
		throw Exc(nc_strerror(retval)); 
	return ret;
}

double NetCDFFile::GetDouble(const char *name) {
	lastvarid = GetId(name);
	double ret;
	if ((retval = nc_get_var_double(ncid, lastvarid, &ret)))	
		throw Exc(nc_strerror(retval)); 
	return ret;
}

String NetCDFFile::GetString(const char *name) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData0(lastvarid, type, dims);
	
	if (type != NC_CHAR)
		throw Exc(Format("Data is not char. Found %s", TypeName(type)));
	
	if (dims.size() != 1)
		throw Exc(Format("Wrong number of dimensions in GetString(). Found %d", dims.size()));
	
	StringBuffer data(dims[0]);
	if ((retval = nc_get_var_text(ncid, lastvarid, ~data)))
    	throw Exc(nc_strerror(retval));	
	return String(data);
}

void NetCDFFile::GetVariableData(const char *name, nc_type &type, Vector<int> &dims) {
	GetVariableData(GetId(name), type, dims);
}

void NetCDFFile::GetVariableData(int id, nc_type &type, Vector<int> &dims) {
	GetVariableData0(id, type, dims);
	if (type == NC_CHAR) {	// The strings are set as vector of chars
        if (dims.size() == 1)
            dims.Clear();
        else if (dims.size() == 2) 
            dims.SetCount(1);
	}
}

void NetCDFFile::GetVariableData0(int id, nc_type &type, Vector<int> &dims) {
	lastvarid = id;
	int ndim, natts;

    if ((retval = nc_inq_var(ncid, id, NULL, &type, &ndim, NULL, &natts)))
      	throw Exc(nc_strerror(retval));

	Vector<int> dimids(ndim);    
    if ((retval = nc_inq_var(ncid, id, NULL, NULL, NULL, dimids.begin(), NULL)))
      	throw Exc(nc_strerror(retval));
    
    dims.SetCount(ndim);
    for (int i = 0; i < ndim; ++i) {
        size_t dim_size;
	    if ((retval = nc_inq_dimlen(ncid, dimids[i], &dim_size)))
	  		throw Exc(nc_strerror(retval));
	    dims[i] = int(dim_size);
    }
}

void NetCDFFile::GetDouble(const char *name, Eigen::VectorXd &data) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData(lastvarid, type, dims);
	
	if (type != NC_DOUBLE)
		throw Exc(Format("Data is not double. Found %s", TypeName(type)));
	
	if (dims.size() != 1)
		throw Exc(Format("Wrong number of dimensions in GetDouble(). Found %d", dims.size()));
	
	data.resize(dims[0]);
	if ((retval = nc_get_var_double(ncid, lastvarid, data.data())))
    	throw Exc(nc_strerror(retval));
}

void NetCDFFile::GetDouble(const char *name, Vector<double> &data) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData(lastvarid, type, dims);
	
	if (type != NC_DOUBLE)
		throw Exc(Format("Data is not double. Found %s", TypeName(type)));
	
	if (dims.size() != 1)
		throw Exc(Format("Wrong number of dimensions in GetDouble(). Found %d", dims.size()));
	
	data.SetCount(dims[0]);
	if ((retval = nc_get_var_double(ncid, lastvarid, data.begin())))
    	throw Exc(nc_strerror(retval));	
}
	
void NetCDFFile::GetDouble(const char *name, Eigen::MatrixXd &data) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData(lastvarid, type, dims);
	
	if (type != NC_DOUBLE)
		throw Exc(Format("Data is not double. Found %s", TypeName(type)));	

	if (dims.size() != 2)
		throw Exc(Format("Wrong number of dimensions in GetDouble(). Found %d", dims.size()));
	
	int sz = 1;
	for (int n : dims)
		sz *= n;
	
	Buffer<double> d(sz);
	if ((retval = nc_get_var_double(ncid, lastvarid, d.Get())))
    	throw Exc(nc_strerror(retval));
		
	CopyRowMajor(d.Get(), dims[0], dims[1], data);
}

void NetCDFFile::GetDouble(const char *name, MultiDimMatrixRowMajor<double> &d) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData(lastvarid, type, dims);
	
	if (type != NC_DOUBLE)
		throw Exc(Format("Data is not double. Found %s", TypeName(type)));	
	
	d.Resize(dims);
	
	if ((retval = nc_get_var_double(ncid, lastvarid, d.begin())))
    	throw Exc(nc_strerror(retval));
}

void NetCDFFile::GetInt(const char *name, Vector<int> &data) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData(lastvarid, type, dims);
	
	if (type != NC_INT)
		throw Exc(Format("Data is not int. Found %s", TypeName(type)));
	
	if (dims.size() != 1)
		throw Exc(Format("Wrong number of dimensions in GetInt(). Found %d", dims.size()));
	
	data.SetCount(dims[0]);
	if ((retval = nc_get_var_int(ncid, lastvarid, data.begin())))
    	throw Exc(nc_strerror(retval));	
}

void NetCDFFile::GetFloat(const char *name, Vector<float> &data) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData(lastvarid, type, dims);
	
	if (type != NC_FLOAT)
		throw Exc(Format("Data is not float. Found %s", TypeName(type)));
	
	if (dims.size() != 1)
		throw Exc(Format("Wrong number of dimensions in GetFloat(). Found %d", dims.size()));
	
	data.SetCount(dims[0]);
	if ((retval = nc_get_var_float(ncid, lastvarid, data.begin())))
    	throw Exc(nc_strerror(retval));	
}

void NetCDFFile::GetString(const char *name, Vector<String> &data) {
	lastvarid = GetId(name);
	nc_type type;
	Vector<int> dims;
	GetVariableData0(lastvarid, type, dims);
	
	if (type != NC_CHAR)
		throw Exc(Format("Data is not char. Found %s", TypeName(type)));
	
	if (dims.size() != 2)
		throw Exc(Format("Wrong number of dimensions in GetString(). Found %d", dims.size()));
	
	Buffer<char> str(dims[0]*dims[1]);
	if ((retval = nc_get_var_text(ncid, lastvarid, ~str)))
    	throw Exc(nc_strerror(retval));	
	data.SetCount(dims[0]);
	StringBuffer bstr(dims[1]);
	for (int i = 0; i < dims[0]; ++i) {
		memcpy(bstr.begin(), str+i*dims[1], dims[1]);
		bstr.Strlen();
		data[i] = bstr;
	}
}

String NetCDFFile::GetVariableString(const char *name) {
	nc_type type;
	Vector<int> dims;
	GetVariableData(name, type, dims);
	if (dims.size() == 0) {
		switch(type) {
		case NC_CHAR:	return GetString(name);
		case NC_BYTE:
		case NC_SHORT:
		case NC_INT:	return FormatInt(GetInt(name));
		case NC_FLOAT:	return FormatFloat(GetFloat(name));
		case NC_DOUBLE:	return FormatDouble(GetDouble(name));
		default:		return NetCDFFile::TypeName(type);
		}
	} else if (dims.size() == 1) {
		String ret;
		if (type == NC_CHAR) {
			Vector<String> data;
			GetString(name, data);
			for (int i = 0; i < data.size(); ++i) {
				if (i > 0)
					ret << ",";
				ret << data[i];
			}
		} else if (type == NC_BYTE || type == NC_SHORT || type == NC_INT) {
			Vector<int> data;
			GetInt(name, data);
			for (int i = 0; i < data.size(); ++i) {
				if (i > 0)
					ret << ",";
				ret << data[i];
			}
		} else if (type == NC_FLOAT) {
			Vector<float> data;
			GetFloat(name, data);
			for (int i = 0; i < data.size(); ++i) {
				if (i > 0)
					ret << ",";
				ret << data[i];
			}
		} else if (type == NC_DOUBLE) {
			Vector<double> data;
			GetDouble(name, data);
			for (int i = 0; i < data.size(); ++i) {
				if (i > 0)
					ret << ",";
				ret << data[i];
			}
		} else
			return NetCDFFile::TypeName(type);
		return ret;
	}
	return String();
}

NetCDFFile &NetCDFFile::Set(const char *name, int d) {
	int varid;
	
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
		
	if ((retval = nc_def_var(ncid, name, NC_INT, 0, NULL, &varid)))
    	throw Exc(nc_strerror(retval));	
 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
    if ((retval = nc_put_var_int(ncid, varid, &d)))
    	throw Exc(nc_strerror(retval));	
    
    lastvarid = varid;
	
	return *this;
}

NetCDFFile &NetCDFFile::Set(const char *name, double d) {
	int varid;
	
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
	   
	if ((retval = nc_def_var(ncid, name, NC_DOUBLE, 0, NULL, &varid)))
    	throw Exc(nc_strerror(retval));	
 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
    if ((retval = nc_put_var_double(ncid, varid, &d)))
    	throw Exc(nc_strerror(retval));	
    
    lastvarid = varid;
	
	return *this;
}

NetCDFFile &NetCDFFile::Set(const char *name, const char *d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
		
	int dimid;
	if ((retval = nc_def_dim(ncid, name, strlen(d), &dimid)))
       	throw Exc(nc_strerror(retval));	

	int varid;
	if ((retval = nc_def_var(ncid, name, NC_CHAR, 1, &dimid, &varid)))
    	throw Exc(nc_strerror(retval));	
    	 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
    if ((retval = nc_put_var_text(ncid, varid, d)))
    	throw Exc(nc_strerror(retval));	
	
	lastvarid = varid;
	
	return *this;
}

NetCDFFile &NetCDFFile::Set(const char *name, const Eigen::VectorXd &d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
		
	int dimid;
	if ((retval = nc_def_dim(ncid, name, d.size(), &dimid)))
       	throw Exc(nc_strerror(retval));	

	int varid;
	if ((retval = nc_def_var(ncid, name, NC_DOUBLE, 1, &dimid, &varid)))
    	throw Exc(nc_strerror(retval));	
    	 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
    if ((retval = nc_put_var_double(ncid, varid, d.data())))
    	throw Exc(nc_strerror(retval));	
	
	lastvarid = varid;
	
	return *this;
}

NetCDFFile &NetCDFFile::Set(const char *name, const Vector<double> &d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
		
	int dimid;
	if ((retval = nc_def_dim(ncid, name, d.size(), &dimid)))
       	throw Exc(nc_strerror(retval));	

	int varid;
	if ((retval = nc_def_var(ncid, name, NC_DOUBLE, 1, &dimid, &varid)))
    	throw Exc(nc_strerror(retval));	
    	 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
    if ((retval = nc_put_var_double(ncid, varid, d.begin())))
    	throw Exc(nc_strerror(retval));	
	
	lastvarid = varid;
	
	return *this;
}

NetCDFFile &NetCDFFile::Set(const char *name, const Eigen::MatrixXd &d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
		
	String namedim;
	int dimids[2];
	namedim = Format("%s_0", name);
	if ((retval = nc_def_dim(ncid, ~namedim, d.rows(), &dimids[0])))
       	throw Exc(nc_strerror(retval));	
    namedim = Format("%s_1", name);
    if ((retval = nc_def_dim(ncid, ~namedim, d.cols(), &dimids[1])))
       	throw Exc(nc_strerror(retval));	
    
	int varid;
	if ((retval = nc_def_var(ncid, name, NC_DOUBLE, 2, dimids, &varid)))
    	throw Exc(nc_strerror(retval));	
    	 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
  	Vector<double> data;
	CopyRowMajor(d, data);
	
    if ((retval = nc_put_var_double(ncid, varid, data.begin())))
    	throw Exc(nc_strerror(retval));	
	
	lastvarid = varid;
	
	return *this;
}

NetCDFFile &NetCDFFile::Set(const char *name, const MultiDimMatrixRowMajor<double> &d) {
	if ((retval = nc_redef(ncid)))
    	throw Exc(nc_strerror(retval));	
		
	String namedim;
	Buffer<int> dimids(d.GetNumAxis());
	for (int i = 0; i < d.GetNumAxis(); ++i) {
		namedim = Format("%s_%d", name, i);
		if ((retval = nc_def_dim(ncid, ~namedim, d.size(i), &dimids[i])))
	       	throw Exc(nc_strerror(retval));	
	}
	int varid;
	if ((retval = nc_def_var(ncid, name, NC_DOUBLE, d.GetNumAxis(), dimids, &varid)))
    	throw Exc(nc_strerror(retval));	
    	 
    if ((retval = nc_enddef(ncid)))
    	throw Exc(nc_strerror(retval));	
  
    if ((retval = nc_put_var_double(ncid, varid, d.begin())))
    	throw Exc(nc_strerror(retval));	
	
	lastvarid = varid;
	
	return *this;
}
	
Vector<String> NetCDFFile::ListAttributes(const char *name) {
	if (!name) 
		lastvarid = GetId(name);
	int num;
    if ((retval = nc_inq_varnatts(ncid, lastvarid, &num))) 
        throw Exc(nc_strerror(retval));	

	Vector<String> ret;
	char att_name[NC_MAX_NAME + 1];
	for (int i = 0; i < num; ++i) {
	    if ((retval = nc_inq_attname(ncid, lastvarid, i, att_name)))
			throw Exc(nc_strerror(retval));	
		ret << att_name;
	}
	return ret;
}
    
String NetCDFFile::ToString0() {
	String ret;
	String indent = "\n";
	if (allowGroups)
		indent += String('\t', groupPathIds.size()-1);
	
	Vector<String> listGlobal = ListGlobalAttributes();
	ret << indent << Format("Global attributes (%d):", listGlobal.size());
	for (const String &name : listGlobal) {
		nc_type type = GetAttributeType(name);
		ret << indent << Format(">%s (%s): %s", name, NetCDFFile::TypeName(type), GetAttributeString(name));
	}
	Vector<String> listVars = ListVariables();
	ret << indent << Format("Variables (%d):", listVars.size());
	for (const String &name : listVars) {
		nc_type type;
		Vector<int> dims;
		GetVariableData(name, type, dims);
		ret << indent << Format(">%s (%s)", name, NetCDFFile::TypeName(type));
		if (!dims.IsEmpty()) {
			String sdims;
			for (int i = 0; i < dims.size(); ++i) {
				if (i != 0)
					sdims << ",";
				sdims << dims[i];
			}
			ret << indent << Format("(%s)", sdims);
		}
		ret << Format(": %s", GetVariableString(name));
		Vector<String> attributes = ListAttributes(name);
		for (const String &attr : attributes) {
			nc_type type = GetAttributeType(attr);
			ret << indent << Format("\tattrib>%s (%s): %s", attr, NetCDFFile::TypeName(type), GetAttributeString(attr));
		}
	}
	ret << indent << Format("SubGroups (%d):", ListGroups().size());
	for (int i = 0; i < ListGroups().size(); ++i) {
		ret << indent << "\t" << Format("Group: %s", ListGroups()[i]);	
		ChangeGroup(ListGroups()[i]);
		ret << ToString0();
		ChangeGroupUp();
	}
	return ret;	
}
   
String NetCDFFile::ToString() {
	String ret = Format("Format: %s", GetFileFormat());
	
	ChangeGroupRoot();
	
	ret << ToString0();
	
	return ret;
}

const Vector<String> &NetCDFFile::ListGroups() const {
	return groupNames;
}

void NetCDFFile::ChangeGroup(int group_id) {
	if (!allowGroups)
		return;
	ncid = group_id;
	int numgrps;
    if ((retval = nc_inq_grps(ncid, &numgrps, NULL)))
        throw Exc(nc_strerror(retval));	

	groupIds.SetCount(numgrps);
    if ((retval = nc_inq_grps(ncid, NULL, groupIds.begin())))
    	throw Exc(nc_strerror(retval));	

	char grp_name[256];
	groupNames.SetCount(numgrps);
    for (int i = 0; i < numgrps; i++) {
        if ((retval = nc_inq_grpname(groupIds[i], grp_name)))
            throw Exc(nc_strerror(retval));	
        
        groupNames[i] = grp_name;
    }
	int id = Find(groupPathIds, ncid);
	if (id >= 0)
		groupPathIds.SetCount(id+1);
	else
		groupPathIds << ncid;
}

void NetCDFFile::ChangeGroupRoot() {
	ChangeGroup(fileid);
}

void NetCDFFile::ChangeGroupUp() {
	if (groupPathIds.size() <= 1)
		return;
	ChangeGroup(groupPathIds[groupPathIds.size()-2]);
}
	
void NetCDFFile::ChangeGroup(const char *group) {
	if (!allowGroups)
		return;
	int id = Find(groupNames, group);
	if (id < 0)
		throw Exc(t_("Group not found"));
	ChangeGroup(groupIds[id]);
}

void NetCDFFile::CreateGroup(const char *group, bool change) {
	if (!allowGroups)
		return;
	int group_id;
    if ((retval = nc_def_grp(ncid, group, &group_id))) 
        throw Exc(nc_strerror(retval));	
    if (change) 
        ChangeGroup(group_id);
}

}