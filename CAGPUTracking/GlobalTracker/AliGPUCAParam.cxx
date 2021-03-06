#include "AliGPUCAParam.h"
#include "AliGPUTPCDef.h"

#if !defined(GPUCA_GPUCODE) && defined(GPUCA_ALIROOT_LIB)
#include "AliTPCClusterParam.h"
#include "AliTPCcalibDB.h"
#include <iostream>
#endif

#if !defined(GPUCA_GPUCODE)
#include "AliGPUTPCGeometry.h"
#include <cstring>

void AliGPUCAParam::SetDefaults(float solenoidBz)
{
	memset((void*) this, 0, sizeof(*this));
	rec.SetDefaults();
    
	float const kParamS0Par[2][3][6]=
	{
		{  { 6.45913474727e-04, 2.51547407970e-05, 1.57551113516e-02, 1.99872811635e-08, -5.86769729853e-03, 9.16301505640e-05 },
		{ 9.71546804067e-04, 1.70938055817e-05, 2.17084009200e-02, 3.90275758377e-08, -1.68631039560e-03, 8.40498323669e-05 },
		{ 7.27469159756e-05, 2.63869314949e-05, 3.29690799117e-02, -2.19274429725e-08, 1.77378822118e-02, 3.26595727529e-05 }
	},
		{  { 1.46874145139e-03, 6.36232061879e-06, 1.28665426746e-02, 1.19409449439e-07, 1.15883778781e-02, 1.32179644424e-04 },
		{ 1.15970033221e-03, 1.30452335725e-05, 1.87015570700e-02, 5.39766737973e-08, 1.64790824056e-02, 1.44115634612e-04 },
		{ 6.27940462437e-04, 1.78520094778e-05, 2.83537860960e-02, 1.16867742150e-08, 5.02607785165e-02, 1.88510020962e-04 }
	}
	};

	float const kParamRMS0[2][3][4] =
	{
		{  { 4.17516864836e-02, 1.87623649254e-04, 5.63788712025e-02, 5.38373768330e-01,  },
		{ 8.29434990883e-02, 2.03291710932e-04, 6.81538805366e-02, 9.70965325832e-01,  },
		{ 8.67543518543e-02, 2.10733342101e-04, 1.38366967440e-01, 2.55089461803e-01,  }
	},
		{  { 5.96254616976e-02, 8.62886518007e-05, 3.61776389182e-02, 4.79704320431e-01,  },
		{ 6.12571723759e-02, 7.23929333617e-05, 3.93057651818e-02, 9.29222583771e-01,  },
		{ 6.58465921879e-02, 1.03639606095e-04, 6.07583411038e-02, 9.90289509296e-01,  }
	}
	};

	for( int i=0; i<2; i++)
	{
		for( int j=0; j<3; j++)
		{
			for( int k=0; k<6; k++)
			{
				ParamS0Par[i][j][k] = kParamS0Par[i][j][k];
			}
		}
	}

	for( int i=0; i<2; i++)
	{
		for( int j=0; j<3; j++)
		{
			for( int k=0; k<4; k++)
			{
				ParamRMS0[i][j][k] = kParamRMS0[i][j][k];
			}
		}
	}
    
	for (int irow = 0;irow < GPUCA_ROW_COUNT;irow++)
	{
		RowX[irow] = AliGPUTPCGeometry::Row2X(irow);
	}

	RMin = 83.65f;
	RMax = 247.7f;
	DAlpha = 0.349066f;
	PadPitch = 0.4f;
	BzkG = solenoidBz;
	constexpr double kCLight = 0.000299792458f;
	ConstBz = solenoidBz * kCLight;
	ErrX = PadPitch / CAMath::Sqrt(12.f);
	ErrY = 1.;
	ErrZ = 0.228808;
    
	constexpr float plusZmin = 0.0529937;
	constexpr float plusZmax = 249.778;
	constexpr float minusZmin = -249.645;
	constexpr float minusZmax = -0.0799937;
	for (int i = 0;i < 36;i++)
	{
		const bool zPlus = (i < 18);
		SliceParam[i].ZMin = zPlus ? plusZmin : minusZmin;
		SliceParam[i].ZMax = zPlus ? plusZmax : minusZmax;
		int tmp = i;
		if (tmp >= 18) tmp -= 18;
		if (tmp >= 9) tmp -= 18;
		SliceParam[i].Alpha = 0.174533 + DAlpha * tmp;
		SliceParam[i].CosAlpha = CAMath::Cos(SliceParam[i].Alpha);
		SliceParam[i].SinAlpha = CAMath::Sin(SliceParam[i].Alpha);
		SliceParam[i].AngleMin = SliceParam[i].Alpha - DAlpha / 2.f;
		SliceParam[i].AngleMax = SliceParam[i].Alpha + DAlpha / 2.f;
	}
    
	AssumeConstantBz = false;
	ToyMCEventsFlag = false;
	ContinuousTracking = false;
	continuousMaxTimeBin = 0;
	debugLevel = 0;
	  resetTimers = false;
}

void AliGPUCAParam::UpdateEventSettings(const AliGPUCASettingsEvent* e, const AliGPUCASettingsDeviceProcessing* p)
{
	AssumeConstantBz = e->constBz;
	ToyMCEventsFlag = e->homemadeEvents;
	ContinuousTracking = e->continuousMaxTimeBin != 0;
	continuousMaxTimeBin = e->continuousMaxTimeBin == -1 ? (0.023 * 5e6) : e->continuousMaxTimeBin;
	if (p)
	{
		debugLevel = p->debugLevel;
		resetTimers = p->resetTimers;
	}
}

void AliGPUCAParam::SetDefaults(const AliGPUCASettingsEvent *e, const AliGPUCASettingsRec *r, const AliGPUCASettingsDeviceProcessing *p)
{
	SetDefaults(e->solenoidBz);
	if (r) rec = *r;
	UpdateEventSettings(e, p);
}

#endif

#if !defined(GPUCA_GPUCODE)
#if !defined(GPUCA_ALIROOT_LIB)
void AliGPUCAParam::LoadClusterErrors(bool Print)
{
}

#else

#include <iomanip>
#include <iostream>
void AliGPUCAParam::LoadClusterErrors(bool Print)
{
	// update of calculated values
	const AliTPCClusterParam *clparam = AliTPCcalibDB::Instance()->GetClusterParam();
	if (!clparam)
	{
		std::cout << "Error: AliGPUCAParam::LoadClusterErrors():: No AliTPCClusterParam instance found !!!! " << std::endl;
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 6; k++)
			{
				ParamS0Par[i][j][k] = clparam->GetParamS0Par(i, j, k);
			}
		}
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				ParamRMS0[i][j][k] = clparam->GetParamRMS0(i, j, k);
			}
		}
	}

	if (Print)
	{
		typedef std::numeric_limits<float> flt;
		std::cout << std::scientific;
#if __cplusplus >= 201103L
		std::cout << std::setprecision(flt::max_digits10 + 2);
#endif
		std::cout << "ParamS0Par[2][3][7]=" << std::endl;
		std::cout << " { " << std::endl;
		for (int i = 0; i < 2; i++)
		{
			std::cout << "   { " << std::endl;
			for (int j = 0; j < 3; j++)
			{
				std::cout << " { ";
				for (int k = 0; k < 6; k++)
				{
					std::cout << ParamS0Par[i][j][k] << ", ";
				}
				std::cout << " }, " << std::endl;
			}
			std::cout << "   }, " << std::endl;
		}
		std::cout << " }; " << std::endl;

		std::cout << "ParamRMS0[2][3][4]=" << std::endl;
		std::cout << " { " << std::endl;
		for (int i = 0; i < 2; i++)
		{
			std::cout << "   { " << std::endl;
			for (int j = 0; j < 3; j++)
			{
				std::cout << " { ";
				for (int k = 0; k < 4; k++)
				{
					std::cout << ParamRMS0[i][j][k] << ", ";
				}
				std::cout << " }, " << std::endl;
			}
			std::cout << "   }, " << std::endl;
		}
		std::cout << " }; " << std::endl;

		const THnBase *waveMap = clparam->GetWaveCorrectionMap();
		const THnBase *resYMap = clparam->GetResolutionYMap();
		std::cout << "waveMap = " << (void *) waveMap << std::endl;
		std::cout << "resYMap = " << (void *) resYMap << std::endl;
	}
}
#endif
#endif

MEM_CLASS_PRE()
void MEM_LG(AliGPUCAParam)::Slice2Global(int iSlice, float x, float y, float z, float *X, float *Y, float *Z) const
{
	// conversion of coorinates sector->global
	*X = x * SliceParam[iSlice].CosAlpha - y * SliceParam[iSlice].SinAlpha;
	*Y = y * SliceParam[iSlice].CosAlpha + x * SliceParam[iSlice].SinAlpha;
	*Z = z;
}

MEM_CLASS_PRE()
void MEM_LG(AliGPUCAParam)::Global2Slice(int iSlice, float X, float Y, float Z, float *x, float *y, float *z) const
{
	// conversion of coorinates global->sector
	*x = X * SliceParam[iSlice].CosAlpha + Y * SliceParam[iSlice].SinAlpha;
	*y = Y * SliceParam[iSlice].CosAlpha - X * SliceParam[iSlice].SinAlpha;
	*z = Z;
}

MEM_CLASS_PRE()
GPUd() float MEM_LG(AliGPUCAParam)::GetClusterRMS(int yz, int type, float z, float angle2) const
{
	//* recalculate the cluster error wih respect to the track slope

	MakeType(const float *) c = ParamRMS0[yz][type];
	float v = c[0] + c[1] * z + c[2] * angle2;
	v = fabs(v);
	return v;
}

MEM_CLASS_PRE()
GPUd() void MEM_LG(AliGPUCAParam)::GetClusterRMS2(int iRow, float z, float sinPhi, float DzDs, float &ErrY2, float &ErrZ2) const
{
	int rowType = (iRow < 63) ? 0 : ((iRow > 126) ? 1 : 2);
	z = CAMath::Abs((250. - 0.275) - CAMath::Abs(z));
	float s2 = sinPhi * sinPhi;
	if (s2 > 0.95f * 0.95f) s2 = 0.95f * 0.95f;
	float sec2 = 1.f / (1.f - s2);
	float angleY2 = s2 * sec2;          // dy/dx
	float angleZ2 = DzDs * DzDs * sec2; // dz/dx

	ErrY2 = GetClusterRMS(0, rowType, z, angleY2);
	ErrZ2 = GetClusterRMS(1, rowType, z, angleZ2);
	ErrY2 *= ErrY2;
	ErrZ2 *= ErrZ2;
}

MEM_CLASS_PRE()
GPUd() float MEM_LG(AliGPUCAParam)::GetClusterError2(int yz, int type, float z, float angle2) const
{
	//* recalculate the cluster error wih respect to the track slope

	MakeType(const float *) c = ParamS0Par[yz][type];
	float v = c[0] + c[1] * z + c[2] * angle2 + c[3] * z * z + c[4] * angle2 * angle2 + c[5] * z * angle2;
	v = fabs(v);
	if (v < 0.01) v = 0.01;
	v *= yz ? rec.ClusterError2CorrectionZ : rec.ClusterError2CorrectionY;
	return v;
}

MEM_CLASS_PRE()
GPUd() void MEM_LG(AliGPUCAParam)::GetClusterErrors2(int iRow, float z, float sinPhi, float DzDs, float &ErrY2, float &ErrZ2) const
{
	// Calibrated cluster error from OCDB for Y and Z
	int rowType = (iRow < 63) ? 0 : ((iRow > 126) ? 1 : 2);
	z = CAMath::Abs((250. - 0.275) - CAMath::Abs(z));
	float s2 = sinPhi * sinPhi;
	if (s2 > 0.95f * 0.95f) s2 = 0.95f * 0.95f;
	float sec2 = 1.f / (1.f - s2);
	float angleY2 = s2 * sec2;          // dy/dx
	float angleZ2 = DzDs * DzDs * sec2; // dz/dx

	ErrY2 = GetClusterError2(0, rowType, z, angleY2);
	ErrZ2 = GetClusterError2(1, rowType, z, angleZ2);
}
