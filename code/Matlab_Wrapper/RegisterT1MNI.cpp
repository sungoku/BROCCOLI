/*
 * BROCCOLI: An open source multi-platform software for parallel analysis of fMRI data on many core CPUs and GPUS
 * Copyright (C) <2013>  Anders Eklund, andek034@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mex.h"
#include "help_functions.cpp"
#include "broccoli_lib.h"
#include <math.h>

float round_( float d )
{
    return floor( d + 0.5f );
}

void cleanUp()
{
    //cudaDeviceReset();
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    //-----------------------
    // Input pointers
    
    double		    *h_T1_Volume_double, *h_MNI_Volume_double, *h_Quadrature_Filter_1_Real_double, *h_Quadrature_Filter_2_Real_double, *h_Quadrature_Filter_3_Real_double, *h_Quadrature_Filter_1_Imag_double, *h_Quadrature_Filter_2_Imag_double, *h_Quadrature_Filter_3_Imag_double;
    float           *h_T1_Volume, *h_MNI_Volume, *h_Quadrature_Filter_1_Real, *h_Quadrature_Filter_2_Real, *h_Quadrature_Filter_3_Real, *h_Quadrature_Filter_1_Imag, *h_Quadrature_Filter_2_Imag, *h_Quadrature_Filter_3_Imag;
    int             IMAGE_REGISTRATION_FILTER_SIZE, NUMBER_OF_ITERATIONS_FOR_IMAGE_REGISTRATION;
    
    float           T1_VOXEL_SIZE_X, T1_VOXEL_SIZE_Y, T1_VOXEL_SIZE_Z;
    float           MNI_VOXEL_SIZE_X, MNI_VOXEL_SIZE_Y, MNI_VOXEL_SIZE_Z;
    
    //-----------------------
    // Output pointers        
    
    double     		*h_Aligned_T1_Volume_double, *h_Registration_Parameters_double;
    double          *h_Quadrature_Filter_Response_1_Real_double, *h_Quadrature_Filter_Response_1_Imag_double;
    double          *h_Quadrature_Filter_Response_2_Real_double, *h_Quadrature_Filter_Response_2_Imag_double;
    double          *h_Quadrature_Filter_Response_3_Real_double, *h_Quadrature_Filter_Response_3_Imag_double;
    double          *h_Phase_Differences_double, *h_Phase_Certainties_double, *h_Phase_Gradients_double;
    float           *h_Quadrature_Filter_Response_1_Real, *h_Quadrature_Filter_Response_1_Imag;
    float           *h_Quadrature_Filter_Response_2_Real, *h_Quadrature_Filter_Response_2_Imag;
    float           *h_Quadrature_Filter_Response_3_Real, *h_Quadrature_Filter_Response_3_Imag;  
    float           *h_Phase_Differences, *h_Phase_Certainties, *h_Phase_Gradients;
    float           *h_Aligned_T1_Volume, *h_Registration_Parameters;
    
    //---------------------
    
    /* Check the number of input and output arguments. */
    if(nrhs<12)
    {
        mexErrMsgTxt("Too few input arguments.");
    }
    if(nrhs>12)
    {
        mexErrMsgTxt("Too many input arguments.");
    }
    if(nlhs<8)
    {
        mexErrMsgTxt("Too few output arguments.");
    }
    if(nlhs>8)
    {
        mexErrMsgTxt("Too many output arguments.");
    }
    
    /* Input arguments */
    
    // The data
    h_T1_Volume_double =  (double*)mxGetData(prhs[0]);
    h_MNI_Volume_double =  (double*)mxGetData(prhs[1]);
    T1_VOXEL_SIZE_X = (float)mxGetScalar(prhs[2]);
    T1_VOXEL_SIZE_Y = (float)mxGetScalar(prhs[3]);
    T1_VOXEL_SIZE_Z = (float)mxGetScalar(prhs[4]);
    MNI_VOXEL_SIZE_X = (float)mxGetScalar(prhs[5]);
    MNI_VOXEL_SIZE_Y = (float)mxGetScalar(prhs[6]);
    MNI_VOXEL_SIZE_Z = (float)mxGetScalar(prhs[7]);    
    h_Quadrature_Filter_1_Real_double =  (double*)mxGetPr(prhs[8]);
    h_Quadrature_Filter_1_Imag_double =  (double*)mxGetPi(prhs[8]);
    h_Quadrature_Filter_2_Real_double =  (double*)mxGetPr(prhs[9]);
    h_Quadrature_Filter_2_Imag_double =  (double*)mxGetPi(prhs[9]);
    h_Quadrature_Filter_3_Real_double =  (double*)mxGetPr(prhs[10]);
    h_Quadrature_Filter_3_Imag_double =  (double*)mxGetPi(prhs[10]);
    NUMBER_OF_ITERATIONS_FOR_IMAGE_REGISTRATION  = (int)mxGetScalar(prhs[11]);
    
    int NUMBER_OF_DIMENSIONS = mxGetNumberOfDimensions(prhs[0]);
    const int *ARRAY_DIMENSIONS_DATA = mxGetDimensions(prhs[0]);
    const int *ARRAY_DIMENSIONS_FILTER = mxGetDimensions(prhs[8]);
    
    int DATA_H, DATA_W, DATA_D, NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS;
    NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS = 12;
    
    DATA_H = ARRAY_DIMENSIONS_DATA[0];
    DATA_W = ARRAY_DIMENSIONS_DATA[1];
    DATA_D = ARRAY_DIMENSIONS_DATA[2];
                
    IMAGE_REGISTRATION_FILTER_SIZE = ARRAY_DIMENSIONS_FILTER[0];
            
   	int T1_DATA_W_NEW = (int)round_((float)DATA_W * T1_VOXEL_SIZE_X / MNI_VOXEL_SIZE_X);
	int T1_DATA_H_NEW = (int)round_((float)DATA_H * T1_VOXEL_SIZE_Y / MNI_VOXEL_SIZE_Y);
	int T1_DATA_D_NEW = (int)round_((float)DATA_D * T1_VOXEL_SIZE_Z / MNI_VOXEL_SIZE_Z);

    int IMAGE_REGISTRATION_PARAMETERS_SIZE = NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS * sizeof(float);
    int FILTER_SIZE = IMAGE_REGISTRATION_FILTER_SIZE * IMAGE_REGISTRATION_FILTER_SIZE * IMAGE_REGISTRATION_FILTER_SIZE * sizeof(float);
    int VOLUME_SIZE = DATA_W * DATA_H * DATA_D * sizeof(float);
    int NEW_VOLUME_SIZE = T1_DATA_W_NEW * T1_DATA_H_NEW * T1_DATA_D_NEW * sizeof(float);
    
    mexPrintf("Data size : %i x %i x %i \n",  DATA_W, DATA_H, DATA_D);
    mexPrintf("New data size : %i x %i x %i \n",  T1_DATA_W_NEW, T1_DATA_H_NEW, T1_DATA_D_NEW);
    mexPrintf("Filter size : %i x %i x %i \n",  IMAGE_REGISTRATION_FILTER_SIZE,IMAGE_REGISTRATION_FILTER_SIZE,IMAGE_REGISTRATION_FILTER_SIZE);
    mexPrintf("Number of iterations : %i \n",  NUMBER_OF_ITERATIONS_FOR_IMAGE_REGISTRATION);
    
    //-------------------------------------------------
    // Output to Matlab
    
    // Create pointer for volumes to Matlab        
    NUMBER_OF_DIMENSIONS = 3;
    int ARRAY_DIMENSIONS_OUT_ALIGNED_T1_VOLUME[3];
    ARRAY_DIMENSIONS_OUT_ALIGNED_T1_VOLUME[0] = T1_DATA_H_NEW;
    ARRAY_DIMENSIONS_OUT_ALIGNED_T1_VOLUME[1] = T1_DATA_W_NEW;
    ARRAY_DIMENSIONS_OUT_ALIGNED_T1_VOLUME[2] = T1_DATA_D_NEW;
    
    plhs[0] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_ALIGNED_T1_VOLUME,mxDOUBLE_CLASS, mxREAL);
    h_Aligned_T1_Volume_double = mxGetPr(plhs[0]);          
    
    NUMBER_OF_DIMENSIONS = 2;
    int ARRAY_DIMENSIONS_OUT_IMAGE_REGISTRATION_PARAMETERS[2];
    ARRAY_DIMENSIONS_OUT_IMAGE_REGISTRATION_PARAMETERS[0] = 1;
    ARRAY_DIMENSIONS_OUT_IMAGE_REGISTRATION_PARAMETERS[1] = NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS;    
    
    plhs[1] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_IMAGE_REGISTRATION_PARAMETERS,mxDOUBLE_CLASS, mxREAL);
    h_Registration_Parameters_double = mxGetPr(plhs[1]);          
    
    NUMBER_OF_DIMENSIONS = 3;
    int ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE[3];
    ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE[0] = DATA_H;
    ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE[1] = DATA_W;    
    ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE[2] = DATA_D;  
    
    plhs[2] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE,mxDOUBLE_CLASS, mxCOMPLEX);
    h_Quadrature_Filter_Response_1_Real_double = mxGetPr(plhs[2]);          
    h_Quadrature_Filter_Response_1_Imag_double = mxGetPi(plhs[2]);          
    
    plhs[3] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE,mxDOUBLE_CLASS, mxCOMPLEX);
    h_Quadrature_Filter_Response_2_Real_double = mxGetPr(plhs[3]);          
    h_Quadrature_Filter_Response_2_Imag_double = mxGetPi(plhs[3]);          
        
    plhs[4] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE,mxDOUBLE_CLASS, mxCOMPLEX);
    h_Quadrature_Filter_Response_3_Real_double = mxGetPr(plhs[4]);          
    h_Quadrature_Filter_Response_3_Imag_double = mxGetPi(plhs[4]);          
                
    plhs[5] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE,mxDOUBLE_CLASS, mxREAL);
    h_Phase_Differences_double = mxGetPr(plhs[5]);          
    
    plhs[6] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE,mxDOUBLE_CLASS, mxREAL);
    h_Phase_Certainties_double = mxGetPr(plhs[6]);          
    
    plhs[7] = mxCreateNumericArray(NUMBER_OF_DIMENSIONS,ARRAY_DIMENSIONS_OUT_FILTER_RESPONSE,mxDOUBLE_CLASS, mxREAL);
    h_Phase_Gradients_double = mxGetPr(plhs[7]);          
    
    // ------------------------------------------------
    
    // Allocate memory on the host
    h_T1_Volume                            = (float *)mxMalloc(VOLUME_SIZE);
    h_Quadrature_Filter_1_Real             = (float *)mxMalloc(FILTER_SIZE);
    h_Quadrature_Filter_1_Imag             = (float *)mxMalloc(FILTER_SIZE);
    h_Quadrature_Filter_2_Real             = (float *)mxMalloc(FILTER_SIZE);
    h_Quadrature_Filter_2_Imag             = (float *)mxMalloc(FILTER_SIZE);    
    h_Quadrature_Filter_3_Real             = (float *)mxMalloc(FILTER_SIZE);
    h_Quadrature_Filter_3_Imag             = (float *)mxMalloc(FILTER_SIZE);    
    h_Quadrature_Filter_Response_1_Real    = (float *)mxMalloc(VOLUME_SIZE);
    h_Quadrature_Filter_Response_1_Imag    = (float *)mxMalloc(VOLUME_SIZE);
    h_Quadrature_Filter_Response_2_Real    = (float *)mxMalloc(VOLUME_SIZE);
    h_Quadrature_Filter_Response_2_Imag    = (float *)mxMalloc(VOLUME_SIZE);
    h_Quadrature_Filter_Response_3_Real    = (float *)mxMalloc(VOLUME_SIZE);
    h_Quadrature_Filter_Response_3_Imag    = (float *)mxMalloc(VOLUME_SIZE);    
    h_Phase_Differences                    = (float *)mxMalloc(VOLUME_SIZE);    
    h_Phase_Certainties                    = (float *)mxMalloc(VOLUME_SIZE);  
    h_Phase_Gradients                      = (float *)mxMalloc(VOLUME_SIZE);  
    h_Aligned_T1_Volume                    = (float *)mxMalloc(NEW_VOLUME_SIZE);
    h_Registration_Parameters              = (float *)mxMalloc(IMAGE_REGISTRATION_PARAMETERS_SIZE);
    
    // Reorder and cast data
    pack_double2float_volume(h_T1_Volume, h_T1_Volume_double, DATA_W, DATA_H, DATA_D);
    pack_double2float_volume(h_Quadrature_Filter_1_Real, h_Quadrature_Filter_1_Real_double, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE);
    pack_double2float_volume(h_Quadrature_Filter_1_Imag, h_Quadrature_Filter_1_Imag_double, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE);
    pack_double2float_volume(h_Quadrature_Filter_2_Real, h_Quadrature_Filter_2_Real_double, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE);
    pack_double2float_volume(h_Quadrature_Filter_2_Imag, h_Quadrature_Filter_2_Imag_double, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE);
    pack_double2float_volume(h_Quadrature_Filter_3_Real, h_Quadrature_Filter_3_Real_double, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE);
    pack_double2float_volume(h_Quadrature_Filter_3_Imag, h_Quadrature_Filter_3_Imag_double, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE, IMAGE_REGISTRATION_FILTER_SIZE);

    //------------------------
    
    BROCCOLI_LIB BROCCOLI;
    
    BROCCOLI.SetT1Width(DATA_W);
    BROCCOLI.SetT1Height(DATA_H);
    BROCCOLI.SetT1Depth(DATA_D);
    BROCCOLI.SetT1VoxelSizeX(T1_VOXEL_SIZE_X);
    BROCCOLI.SetT1VoxelSizeY(T1_VOXEL_SIZE_Y);
    BROCCOLI.SetT1VoxelSizeZ(T1_VOXEL_SIZE_Z);            
    BROCCOLI.SetMNIVoxelSizeX(MNI_VOXEL_SIZE_X);
    BROCCOLI.SetMNIVoxelSizeY(MNI_VOXEL_SIZE_Y);
    BROCCOLI.SetMNIVoxelSizeZ(MNI_VOXEL_SIZE_Z);                
    BROCCOLI.SetInputT1Volume(h_T1_Volume);
    BROCCOLI.SetMotionCorrectionFilterSize(IMAGE_REGISTRATION_FILTER_SIZE);
    BROCCOLI.SetMotionCorrectionFilters(h_Quadrature_Filter_1_Real, h_Quadrature_Filter_1_Imag, h_Quadrature_Filter_2_Real, h_Quadrature_Filter_2_Imag, h_Quadrature_Filter_3_Real, h_Quadrature_Filter_3_Imag);
    BROCCOLI.SetNumberOfIterationsForMotionCorrection(NUMBER_OF_ITERATIONS_FOR_IMAGE_REGISTRATION);
    BROCCOLI.SetOutputData(h_Aligned_T1_Volume);
    BROCCOLI.SetOutputMotionParameters(h_Registration_Parameters);
    BROCCOLI.SetOutputQuadratureFilterResponses(h_Quadrature_Filter_Response_1_Real, h_Quadrature_Filter_Response_1_Imag, h_Quadrature_Filter_Response_2_Real, h_Quadrature_Filter_Response_2_Imag, h_Quadrature_Filter_Response_3_Real, h_Quadrature_Filter_Response_3_Imag);
    BROCCOLI.SetOutputPhaseDifferences(h_Phase_Differences);
    BROCCOLI.SetOutputPhaseCertainties(h_Phase_Certainties);
    BROCCOLI.SetOutputPhaseGradients(h_Phase_Gradients);
    
    mexPrintf("Device info \n \n %s \n", BROCCOLI.GetOpenCLDeviceInfoChar());
    mexPrintf("Build info \n \n %s \n", BROCCOLI.GetOpenCLBuildInfoChar());
            
    BROCCOLI.ChangeT1VolumeResolutionAndSizeWrapper();
    
    int error = BROCCOLI.GetOpenCLError();
    mexPrintf("Error is %d \n",error);
    
    int createKernelError = BROCCOLI.GetOpenCLCreateKernelError();
    mexPrintf("Create kernel error is %d \n",createKernelError);
    
    double convolution_time = BROCCOLI.GetProcessingTimeConvolution();
    mexPrintf("Convolution time is %f ms \n",convolution_time/1000000.0);
    
    
    //unpack_float2double_volume(h_Aligned_T1_Volume_double, h_Motion_Corrected_fMRI_Volumes, DATA_W, DATA_H, DATA_D, DATA_T);
    unpack_float2double_volume(h_Aligned_T1_Volume_double, h_Aligned_T1_Volume, T1_DATA_W_NEW, T1_DATA_H_NEW, T1_DATA_D_NEW);
    unpack_float2double(h_Registration_Parameters_double, h_Registration_Parameters, NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS);
    unpack_float2double_volume(h_Quadrature_Filter_Response_1_Real_double, h_Quadrature_Filter_Response_1_Real, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Quadrature_Filter_Response_1_Imag_double, h_Quadrature_Filter_Response_1_Imag, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Quadrature_Filter_Response_2_Real_double, h_Quadrature_Filter_Response_2_Real, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Quadrature_Filter_Response_2_Imag_double, h_Quadrature_Filter_Response_2_Imag, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Quadrature_Filter_Response_3_Real_double, h_Quadrature_Filter_Response_3_Real, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Quadrature_Filter_Response_3_Imag_double, h_Quadrature_Filter_Response_3_Imag, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Phase_Differences_double, h_Phase_Differences, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Phase_Certainties_double, h_Phase_Certainties, DATA_W, DATA_H, DATA_D);
    unpack_float2double_volume(h_Phase_Gradients_double, h_Phase_Gradients, DATA_W, DATA_H, DATA_D);    
    
    // Free all the allocated memory on the host
    mxFree(h_T1_Volume);
    mxFree(h_Quadrature_Filter_1_Real);
    mxFree(h_Quadrature_Filter_1_Imag);
    mxFree(h_Quadrature_Filter_2_Real);
    mxFree(h_Quadrature_Filter_2_Imag);
    mxFree(h_Quadrature_Filter_3_Real);
    mxFree(h_Quadrature_Filter_3_Imag);
    mxFree(h_Quadrature_Filter_Response_1_Real);
    mxFree(h_Quadrature_Filter_Response_1_Imag);
    mxFree(h_Quadrature_Filter_Response_2_Real);
    mxFree(h_Quadrature_Filter_Response_2_Imag);
    mxFree(h_Quadrature_Filter_Response_3_Real);
    mxFree(h_Quadrature_Filter_Response_3_Imag);
    mxFree(h_Phase_Differences);
    mxFree(h_Phase_Certainties);
    mxFree(h_Phase_Gradients);
    mxFree(h_Aligned_T1_Volume); 
    mxFree(h_Registration_Parameters);
    
    //mexAtExit(cleanUp);
    
    return;
}

