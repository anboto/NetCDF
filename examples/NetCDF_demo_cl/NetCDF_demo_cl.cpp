// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>

using namespace Upp;

#include <NetCDF/NetCDF.h>
  
 /* This is the name of the data file we will create. */
 #define FILE_NAME "sfc_pres_temp.nc"
  
 /* We are writing 2D data, a 6 x 12 lat-lon grid. We will need two
  * netCDF dimensions. */
 #define NDIMS 2
 #define NLAT 6
 #define NLON 12
 #define LAT_NAME "latitude"
 #define LON_NAME "longitude"
  
 /* Names of things. */
 #define PRES_NAME "pressure"
 #define TEMP_NAME "temperature"
 #define UNITS "units"
 #define DEGREES_EAST "degrees_east"
 #define DEGREES_NORTH "degrees_north"
  
 /* These are used to construct some example data. */
 #define SAMPLE_PRESSURE 900
 #define SAMPLE_TEMP 9.0
 #define START_LAT 25.0
 #define START_LON -125.0
  
 /* Handle errors by printing an error message and exiting with a
  * non-zero status. */
 #define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}
 
 /* For the units attributes. */
 #define PRES_UNITS "hPa"
 #define TEMP_UNITS "celsius"
 #define LAT_UNITS "degrees_north"
 #define LON_UNITS "degrees_east"
 #define MAX_ATT_LEN 80
  
  

int ReadAny() {
    int retval;  
    int ncid;   
     
    /* Open the file. */
    if ((retval = nc_open("C:\\Users\\0203853\\OneDrive - SENER\\Escritorio\\Capytaine demo\\dataset.nc", NC_NOWRITE, &ncid)))
       ERR(retval);     
   
       // Inquire about the NetCDF format version
       int format;
    if (nc_inq_format(ncid, &format) != NC_NOERR) {
        fprintf(stderr, "Error inquiring about the NetCDF format.\n");
        nc_close(ncid);
        exit(EXIT_FAILURE);
    }
    // Print the NetCDF format version
    printf("NetCDF format version: ");
    switch (format) {
        case NC_FORMAT_CLASSIC:
            printf("Classic (v1)\n");
            break;
        case NC_FORMAT_64BIT_OFFSET:
            printf("64-bit offset (v2)\n");
            break;
        case NC_FORMAT_64BIT_DATA:
            printf("64-bit data (v2)\n");
            break;
        case NC_FORMAT_NETCDF4:
            printf("NetCDF-4 (HDF5)\n");
            break;
        case NC_FORMAT_NETCDF4_CLASSIC:
            printf("NetCDF-4 Classic (HDF5)\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    
   	int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
   
    if ((retval = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in,
             &unlimdimid_in)))
       ERR(retval);  

    // Print the names and values of the global attributes
    printf("Global attributes in the NetCDF file:\n");
     char att_name[256];
    size_t att_len;
    nc_type att_type;
    
    for (int i = 0; i < ngatts_in; i++) {
        // Get the name of the global attribute
        if ((retval = nc_inq_attname(ncid, NC_GLOBAL, i, att_name))) 
            ERR(retval);
        // Get the type and length of the global attribute
        if ((retval = nc_inq_att(ncid, NC_GLOBAL, att_name, &att_type, &att_len))) 
            ERR(retval);
        if (att_type == NC_CHAR) {
	        // Allocate memory to store attribute value
	        char *value = (char *)malloc((att_len + 1) * sizeof(char));
	        if (value == NULL) 
	            ERR(retval);
	        // Retrieve the value of the global attribute
	        if ((retval = nc_get_att_text(ncid, NC_GLOBAL, att_name, value))) 
	            ERR(retval);
        	value[att_len] = '\0';
        	// Print the name and value of the global attribute
        	printf("%s: %s\n", att_name, value);
        	free(value);
        } else {
            if (att_type == NC_BYTE || att_type == NC_SHORT || att_type == NC_INT) {
                int ret;
	            if ((retval = nc_get_att_int(ncid, NC_GLOBAL, att_name, &ret))) 
	            	ERR(retval);
	            printf("%s (%s): %d\n", att_name, ~NetCDFFile::TypeName(att_type), ret);
            } else if (att_type == NC_FLOAT) {
	            float ret;
	            if ((retval = nc_get_att_float(ncid, NC_GLOBAL, att_name, &ret))) 
	            	ERR(retval);
	            printf("%s (%s): %f\n", att_name, ~NetCDFFile::TypeName(att_type), ret);
	        } else if (att_type == NC_DOUBLE) {
	            double ret;
	            if ((retval = nc_get_att_double(ncid, NC_GLOBAL, att_name, &ret))) 
	            	ERR(retval);
	            printf("%s (%s): %f\n", att_name, ~NetCDFFile::TypeName(att_type), ret);
	        } else 
            	printf("%s (%s): Type not supported\n", att_name, ~NetCDFFile::TypeName(att_type));
        }
        
    }
     
     // Get the variable IDs
    int varids[MAX_NC_VARS];
    if ((retval = nc_inq_varids(ncid, &nvars_in, varids))) 
        ERR(retval);

    // Print the names of the variables
    printf("Variables in the NetCDF file:\n");
    char var_name[256];
    for (int i = 0; i < nvars_in; i++) {
        // Get the variable name
        if ((retval = nc_inq_varname(ncid, varids[i], var_name))) 
            ERR(retval);
        printf("%s\n", var_name);
    }

	int numgrps, *grpids;
    char grp_name[256];
     
     // Inquire about the number of groups
    if ((retval = nc_inq_grps(ncid, &numgrps, NULL)))
        ERR(retval);

    // Allocate memory to store group IDs
    grpids = (int *)malloc(numgrps * sizeof(int));
    if (grpids == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        nc_close(ncid);
        exit(EXIT_FAILURE);
    }

    // Inquire about the groups
    if ((retval = nc_inq_grps(ncid, NULL, grpids)))
    	ERR(retval);

    // Print the names of the groups
    printf("Groups in the NetCDF file:\n");
    for (int i = 0; i < numgrps; i++) {
        // Get the name of the group
        if ((retval = nc_inq_grpname(grpids[i], grp_name)))
            ERR(retval);
        
        printf("%s\n", grp_name);
    }

    // Free memory and close the NetCDF file
    free(grpids);

    if ((retval = nc_close(ncid)))
       ERR(retval);
        
	return 0;  
 }
  
 int
 Read()
 {
    int ncid, pres_varid, temp_varid;
    int lat_varid, lon_varid;
  
    /* We will read surface temperature and pressure fields. */
    float pres_in[NLAT][NLON];
    float temp_in[NLAT][NLON];
  
    /* For the lat lon coordinate variables. */
    float lats_in[NLAT], lons_in[NLON];
  
    /* To check the units attributes. */
    char pres_units_in[MAX_ATT_LEN], temp_units_in[MAX_ATT_LEN];
    char lat_units_in[MAX_ATT_LEN], lon_units_in[MAX_ATT_LEN];
  
    /* We will learn about the data file and store results in these
       program variables. */
    int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
  
    /* Loop indexes. */
    int lat, lon;
  
    /* Error handling. */
    int retval;
  
    String type;
    nc_type xtypep;
    int ndimsp, dimidsp[10], nattsp;
    size_t dim_size;
    
    /* Open the file. */
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
       ERR(retval);
  
    /* There are a number of inquiry functions in netCDF which can be
       used to learn about an unknown netCDF file. NC_INQ tells how
       many netCDF variables, dimensions, and global attributes are in
       the file; also the dimension id of the unlimited dimension, if
       there is one. */
    if ((retval = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in,
             &unlimdimid_in)))
       ERR(retval);
  
    /* In this case we know that there are 2 netCDF dimensions, 4
       netCDF variables, no global attributes, and no unlimited
       dimension. */
    if (ndims_in != 2 || nvars_in != 4 || ngatts_in != 0 ||
        unlimdimid_in != -1) return 2;
  
    /* Get the varids of the latitude and longitude coordinate
     * variables. */
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
       ERR(retval);
  
    /* Read the coordinate variable data. */
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats_in[0])))
       ERR(retval);
    if ((retval = nc_get_var_float(ncid, lon_varid, &lons_in[0])))
       ERR(retval);
  
    if ((retval = nc_inq_var(ncid, lat_varid, NULL, &xtypep, &ndimsp, &dimidsp[0], &nattsp)))
       ERR(retval);
    type = NetCDFFile::TypeName(xtypep);
  	if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &dim_size)))
  		ERR(retval);
  	
  	if ((retval = nc_inq_var(ncid, lon_varid, NULL, &xtypep, &ndimsp, &dimidsp[0], &nattsp)))
       ERR(retval);
    type = NetCDFFile::TypeName(xtypep);
    if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &dim_size)))
  		ERR(retval);
    
    /* Check the coordinate variable data. */
    for (lat = 0; lat < NLAT; lat++)
       if (lats_in[lat] != START_LAT + 5.*lat)
      return 2;
    for (lon = 0; lon < NLON; lon++)
       if (lons_in[lon] != START_LON + 5.*lon)
      return 2;
  
    /* Get the varids of the pressure and temperature netCDF
     * variables. */
    if ((retval = nc_inq_varid(ncid, PRES_NAME, &pres_varid)))
       ERR(retval);
    if ((retval = nc_inq_varid(ncid, TEMP_NAME, &temp_varid)))
       ERR(retval);

    
    if ((retval = nc_inq_var(ncid, pres_varid, NULL, &xtypep, &ndimsp, &dimidsp[0], &nattsp)))
       ERR(retval);
    type = NetCDFFile::TypeName(xtypep);
    if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &dim_size)))
  		ERR(retval);
  	if ((retval = nc_inq_dimlen(ncid, dimidsp[1], &dim_size)))
  		ERR(retval);
  
  	if ((retval = nc_inq_var(ncid, temp_varid, NULL, &xtypep, &ndimsp, &dimidsp[0], &nattsp)))
       ERR(retval);
    type = NetCDFFile::TypeName(xtypep);
    
    /* Read the data. Since we know the contents of the file we know
     * that the data arrays in this program are the correct size to
     * hold all the data. */
    if ((retval = nc_get_var_float(ncid, pres_varid, &pres_in[0][0])))
       ERR(retval);
    if ((retval = nc_get_var_float(ncid, temp_varid, &temp_in[0][0])))
       ERR(retval);
  
    /* Check the data. */
    for (lat = 0; lat < NLAT; lat++)
       for (lon = 0; lon < NLON; lon++)
      if (pres_in[lat][lon] != SAMPLE_PRESSURE + (lon * NLAT + lat) ||
          temp_in[lat][lon] != SAMPLE_TEMP + .25 * (lon * NLAT + lat))
         return 2;
  
    /* Each of the netCDF variables has a "units" attribute. Let's read
       them and check them. */
    if ((retval = nc_get_att_text(ncid, lat_varid, UNITS, lat_units_in)))
       ERR(retval);
    if (strncmp(lat_units_in, LAT_UNITS, strlen(LAT_UNITS)))
       return 2;
  
    if ((retval = nc_get_att_text(ncid, lon_varid, UNITS, lon_units_in)))
       ERR(retval);
    if (strncmp(lon_units_in, LON_UNITS, strlen(LON_UNITS)))
       return 2;
  
    if ((retval = nc_get_att_text(ncid, pres_varid, UNITS, pres_units_in)))
       ERR(retval);
    if (strncmp(pres_units_in, PRES_UNITS, strlen(PRES_UNITS)))
       return 2;
  
    if ((retval = nc_get_att_text(ncid, temp_varid, UNITS, temp_units_in)))
       ERR(retval);
    if (strncmp(temp_units_in, TEMP_UNITS, strlen(TEMP_UNITS))) return 2;
  
    /* Close the file. */
    if ((retval = nc_close(ncid)))
       ERR(retval);
  
    printf("*** SUCCESS reading example file sfc_pres_temp.nc!\n");
    return 0;
 }
 
 int
 Write()
 {
    int ncid, lon_dimid, lat_dimid, pres_varid, temp_varid;
  
 /* In addition to the latitude and longitude dimensions, we will also
    create latitude and longitude netCDF variables which will hold the
    actual latitudes and longitudes. Since they hold data about the
    coordinate system, the netCDF term for these is: "coordinate
    variables." */
    int lat_varid, lon_varid;
  
    int dimids[NDIMS];
  
    /* We will write surface temperature and pressure fields. */
    float pres_out[NLAT][NLON];
    float temp_out[NLAT][NLON];
    float lats[NLAT], lons[NLON];
  
    /* It's good practice for each netCDF variable to carry a "units"
     * attribute. */
    char pres_units[] = "hPa";
    char temp_units[] = "celsius";
  
    /* Loop indexes. */
    int lat, lon;
  
    /* Error handling. */
    int retval;
  
    /* Create some pretend data. If this wasn't an example program, we
     * would have some real data to write, for example, model
     * output. */
    for (lat = 0; lat < NLAT; lat++)
       lats[lat] = START_LAT + 5.*lat;
    for (lon = 0; lon < NLON; lon++)
       lons[lon] = START_LON + 5.*lon;
  
    for (lat = 0; lat < NLAT; lat++)
       for (lon = 0; lon < NLON; lon++)
       {
      pres_out[lat][lon] = SAMPLE_PRESSURE + (lon * NLAT + lat);
      temp_out[lat][lon] = SAMPLE_TEMP + .25 * (lon * NLAT + lat);
       }
  
    /* Create the file. */
    if ((retval = nc_create(FILE_NAME, NC_CLOBBER, &ncid)))
       ERR(retval);
  
    /* Define the dimensions. */
    if ((retval = nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)))
       ERR(retval);
    if ((retval = nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)))
       ERR(retval);
  
    /* Define coordinate netCDF variables. They will hold the
       coordinate information, that is, the latitudes and longitudes. A
       varid is returned for each.*/
    if ((retval = nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, &lat_dimid,
                 &lat_varid)))
       ERR(retval);
    if ((retval = nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, &lon_dimid,
                 &lon_varid)))
       ERR(retval);
  
    /* Define units attributes for coordinate vars. This attaches a
       text attribute to each of the coordinate variables, containing
       the units. Note that we are not writing a trailing NULL, just
       "units", because the reading program may be fortran which does
       not use null-terminated strings. In general it is up to the
       reading C program to ensure that it puts null-terminators on
       strings where necessary.*/
    if ((retval = nc_put_att_text(ncid, lat_varid, UNITS,
                  strlen(DEGREES_NORTH), DEGREES_NORTH)))
       ERR(retval);
    if ((retval = nc_put_att_text(ncid, lon_varid, UNITS,
                  strlen(DEGREES_EAST), DEGREES_EAST)))
       ERR(retval);
  
    /* Define the netCDF variables. The dimids array is used to pass
       the dimids of the dimensions of the variables.*/
    dimids[0] = lat_dimid;
    dimids[1] = lon_dimid;
    if ((retval = nc_def_var(ncid, PRES_NAME, NC_FLOAT, NDIMS,
                 dimids, &pres_varid)))
       ERR(retval);
    if ((retval = nc_def_var(ncid, TEMP_NAME, NC_FLOAT, NDIMS,
                 dimids, &temp_varid)))
       ERR(retval);
  
    /* Define units attributes for vars. */
    if ((retval = nc_put_att_text(ncid, pres_varid, UNITS,
                  strlen(pres_units), pres_units)))
       ERR(retval);
    if ((retval = nc_put_att_text(ncid, temp_varid, UNITS,
                  strlen(temp_units), temp_units)))
       ERR(retval);
  
    /* End define mode. */
    if ((retval = nc_enddef(ncid)))
       ERR(retval);
  
    /* Write the coordinate variable data. This will put the latitudes
       and longitudes of our data grid into the netCDF file. */
    if ((retval = nc_put_var_float(ncid, lat_varid, &lats[0])))
       ERR(retval);
    if ((retval = nc_put_var_float(ncid, lon_varid, &lons[0])))
       ERR(retval);
  
    /* Write the pretend data. This will write our surface pressure and
       surface temperature data. The arrays of data are the same size
       as the netCDF variables we have defined. */
    if ((retval = nc_put_var_float(ncid, pres_varid, &pres_out[0][0])))
       ERR(retval);
    if ((retval = nc_put_var_float(ncid, temp_varid, &temp_out[0][0])))
       ERR(retval);
  
    /* Close the file. */
    if ((retval = nc_close(ncid)))
       ERR(retval);
  
    printf("*** SUCCESS writing example file sfc_pres_temp.nc!\n");
    return 0;
 }
 
CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	try {
		UppLog() << "\nNetCDF wrapper test\n";
		String file = "datalib.nc";
		{
			NetCDFFile cdf;
			
			cdf.Create(file);
			
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
		}
	} catch (Exc err) {
		UppLog() << "\n" << Format(t_("Problem found: %s"), err);
		SetExitCode(-1);
	}
		
	UppLog() << "\nProgram ended\n";
	#ifdef flagDEBUG
	ReadStdIn();
	#endif
}

