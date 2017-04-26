#include "FLKinectPluginPrivatePCH.h"
#include "KinectPluginCore.h"



FKinectPluginCore::FKinectPluginCore():

		m_nStartTime(0),
		m_nLastCounter(0),
		m_nFramesSinceUpdate(0),
		m_fFreq(0),
		m_nNextStatusTime(0LL),
		m_pKinectSensor(NULL),
		m_pCoordinateMapper(NULL),
		m_pBodyFrameReader(NULL),
		m_pkinectCoreWindow(NULL)
		
{
	UE_LOG(LogTemp, Warning, TEXT("Kinect Plugin Core Activated"))
		
		LARGE_INTEGER qpf = { 0 };
		if (QueryPerformanceFrequency(&qpf))
		{
			m_fFreq = double(qpf.QuadPart);
		}
	
}

//Singleton access
FKinectPluginCore* FKinectPluginCore::Instance;

FKinectPluginCore* const FKinectPluginCore::GetInstance()
{	
	if (Instance == nullptr)
	{
		//TODO (OS): Make this a little more RAII
		Instance = new FKinectPluginCore();
		Instance->Thread = FRunnableThread::Create(Instance, TEXT("KinectThread"), 0, EThreadPriority::TPri_Normal);
	}
	return Instance;
}

	FKinectPluginCore::~FKinectPluginCore()
	{
		Stop();
		
		SafeRelease(&m_pBodyFrameReader);

		SafeRelease(&m_pCoordinateMapper);

		if (m_pKinectSensor)
		{
			m_pKinectSensor->Close();
		}
		if (m_pkinectCoreWindow)
		{
			m_pkinectCoreWindow->Release();
		}
		SafeRelease(&m_pkinectCoreWindow);
		SafeRelease(&m_pKinectSensor);
	}


	bool FKinectPluginCore::Init()
	{
		InitializeDefaultSensor();
		bRunning = true;
		//HACK (OS): should return status
		for(int32 i =0; i< BODY_COUNT; i++)
		{
			TrackFlags[i] = false;
		}
		return bRunning;
	}

	/* Runs a looping thread for Kinect	*/
	uint32 FKinectPluginCore::Run()
	{
		UE_LOG(LogTemp, Warning, TEXT("Kinect Running: %d"), bRunning)
		while (bRunning)
		{
			Update();
		}
		//TODO (OS): Should return status;
		return 0;
	}

	void FKinectPluginCore::Stop()
	{
		//TODO (OS - @MR): Implement this;
		UE_LOG(LogTemp, Warning, TEXT("Kinect Stopped"))
		bRunning = false;
	}

	
	void FKinectPluginCore::Update()
	{
		if (!m_pBodyFrameReader)
		{
			return;
		}

		IBodyFrame* pBodyFrame = NULL;

		HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

		if (SUCCEEDED(hr))
		{
			INT64 nTime = 0;

			hr = pBodyFrame->get_RelativeTime(&nTime);

			IBody* ppBodies[BODY_COUNT] = { 0 };

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
			}

			if (SUCCEEDED(hr))
			{
				ProcessBody(nTime, BODY_COUNT, ppBodies);
			}

			for (int i = 0; i < _countof(ppBodies); ++i)
			{
				SafeRelease(&(ppBodies[i]));
			}
		}
		 
		SafeRelease(&pBodyFrame);
	}


	HRESULT FKinectPluginCore::InitializeDefaultSensor()
	{
		HRESULT hr;

		hr = GetDefaultKinectSensor(&m_pKinectSensor);
		if (FAILED(hr))
		{
			return hr;
		}

		if (m_pKinectSensor)
		{
			// Initialize the Kinect and get coordinate mapper and the body reader
			IBodyFrameSource* pBodyFrameSource = NULL;

			hr = m_pKinectSensor->Open();

			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
			}

			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
			}
			SafeRelease(&pBodyFrameSource);
			
		}

		if (!m_pKinectSensor || FAILED(hr))
		{
			UE_LOG(LogTemp, Fatal, TEXT("No Kinect Found!"))
			return E_FAIL;
		}
		return hr;
	}

	FORCEINLINE const FVector JointPositionToFVector(Joint& joint)
	{
		//Meter to Centimeter
		float const m_to_cm = 100;
		return FVector(m_to_cm * joint.Position.X, m_to_cm * joint.Position.Z, m_to_cm * joint.Position.Y);
	}

	FORCEINLINE const FRotator JointOrientationToFRotator(JointOrientation& orientation)
	{
		//Chirality for exchanging the order.
		float const chirality = -1.0; 
		return FRotator(FQuat(chirality * orientation.Orientation.w, orientation.Orientation.x, orientation.Orientation.z, orientation.Orientation.y));
	}

	void FKinectPluginCore::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies)
	{
		HRESULT hr;
		hr = GetKinectCoreWindowForCurrentThread(&m_pkinectCoreWindow);
		if (FAILED(hr))
		{
			UE_LOG(LogTemp, Fatal, TEXT("No kinectCoreWindow Found!"))
		}

		for (int32 i = 0; i < nBodyCount; ++i)
		{
			IBody* pBody = ppBodies[i];
			if (pBody)
			{
				BOOLEAN bTracked;
				hr = pBody->get_IsTracked(&bTracked);
				TrackFlags[i] = bTracked != 0;

				if (SUCCEEDED(hr) && bTracked)
				{
					FFLKinect_Body* body = &bodies[i];
					
					//NOTE (OS): Could be optimized, but I prioritize type safety.
					HandState leftHandState = HandState_Unknown;
					HandState rightHandState = HandState_Unknown;

					pBody->get_HandLeftState(&leftHandState);
					pBody->get_HandRightState(&rightHandState);
					body->HandLeftState	= EFLKinect_HandState(leftHandState);
					body->HandRightState = EFLKinect_HandState(rightHandState);

					//NOTE (OS): *sigh* we can't guarantee the memory model here, so for safety I will make an intermediate copy.
					Joint joints[body->nJoints];
					hr = pBody->GetJoints(body->nJoints, joints);
					if (SUCCEEDED(hr))
					{
						for (int32 j = 0; j < body->nJoints; ++j)
						{
							body->positions[j] = JointPositionToFVector(joints[j]);
							body->trackingStates[j] = EFLKinect_TrackingState(joints[j].TrackingState);
						}
					}				
					JointOrientation jointOrientations[body->nJoints];
					hr = pBody->GetJointOrientations(body->nJoints, jointOrientations);
					if (SUCCEEDED(hr))
					{
						for (int32 j = 0; j < body->nJoints; ++j)
						{
							body->orientations[j] = JointOrientationToFRotator(jointOrientations[j]);
						}
					}
				}
			}

			//TODO (OS): Microsoft Performance code

			if (!m_nStartTime)
			{
				m_nStartTime = nTime;
			}
			double fps = 0.0;

			LARGE_INTEGER qpcNow = { 0 };
			if (m_fFreq && QueryPerformanceCounter(&qpcNow) && m_nLastCounter)
			{			
				m_nFramesSinceUpdate++;
				fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);	
			}
		}
	}

	FFLKinect_Body const* FKinectPluginCore::GetBody(uint8 number)
	{
		//definitely throws if something is wrong.
		return &bodies[number];
	}

	FVector2D  FKinectPluginCore::kinectCoreWindow_PointerMoved()
	{
		if (!m_pkinectCoreWindow)
		{
			return FVector2D(-1,-1);
		}
		WAITABLE_HANDLE waitableHandle;
		HRESULT hr;
		hr = m_pkinectCoreWindow->SubscribePointerMoved(&waitableHandle);
		if(SUCCEEDED(hr))
		{
			IKinectPointerEventArgs* eventData = NULL;
			hr = m_pkinectCoreWindow->GetPointerEnteredEventData(waitableHandle, &eventData);

			hr = m_pkinectCoreWindow->GetPointerMovedEventData(waitableHandle,&eventData);
			if (SUCCEEDED(hr))
			{
				IKinectPointerPoint* kinectPointerPoint = NULL;
				eventData->get_CurrentPoint(&kinectPointerPoint);
				eventData->Release();
				SafeRelease(&eventData);
				PointF point;
				kinectPointerPoint->get_Position(&point);
				hr = m_pkinectCoreWindow->UnsubscribePointerMoved(waitableHandle);
			//	SafeRelease(&waitableHandle);
				return FVector2D(point.X, point.Y);
			}

		}

		return FVector2D::ZeroVector;
	}
