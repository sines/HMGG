#pragma once

//#include "FLKinectPluginPrivatePCH.h"
#include "CoreUObject.h"
#include "Vector.h"
#include "KinectFunctionLibrary.h"
/*
#include "AllowWindowsPlatformTypes.h"
#include "Kinect.h"
#include "HideWindowsPlatformTypes.h"
*/


class FKinectPluginCore : public FRunnable
{
public:	
	
	/* Begin FRunnable interface. */
	virtual bool	Init()	override;
	virtual uint32	Run()	override;
	virtual void	Stop()	override;
	/* End FRunnable interface */

	/* Begin Singleton Interface*/
	static FKinectPluginCore* const GetInstance();
	/* End Singleton Interface*/

	FFLKinect_Body const* GetBody(uint8 number);

	FVector2D  kinectCoreWindow_PointerMoved();

	bool			TrackFlags[BODY_COUNT];
	
private:
	FKinectPluginCore();
	FKinectPluginCore(FKinectPluginCore const&);
	FKinectPluginCore& operator=(FKinectPluginCore const&);
	~FKinectPluginCore();
	
	void ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies);
	void Update();
	HRESULT InitializeDefaultSensor();

	// Current Kinect
	IKinectSensor*          m_pKinectSensor;
	ICoordinateMapper*      m_pCoordinateMapper;
	uint32                  m_nFramesSinceUpdate;
	int64                   m_nStartTime;
	int64                   m_nLastCounter;
	int64                   m_nNextStatusTime;
	double                  m_fFreq;
	IBodyFrameReader*       m_pBodyFrameReader;
	IKinectCoreWindow*		m_pkinectCoreWindow;

	FRunnableThread* Thread;
	
	FFLKinect_Body	bodies[BODY_COUNT];
	
	/* Frunnable Helper. */
	bool bRunning;
	static FKinectPluginCore* Instance;
};



