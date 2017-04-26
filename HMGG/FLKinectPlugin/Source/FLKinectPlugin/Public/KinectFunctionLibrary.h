#pragma once
#include "KinectFunctionLibrary.generated.h"


UENUM(BlueprintType)
enum class EFLKinect_HandState : uint8
{
	Unknown = 0, NotTracking = 1, Open = 2, Closed = 3, Lasso = 4
};

UENUM(BlueprintType)
enum class EFLKinect_JointType : uint8
{
	SpineBase = 0,
	SpineMid = 1,
	Neck = 2,
	Head = 3,
	ShoulderLeft = 4,
	ElbowLeft = 5,
	WristLeft = 6,
	HandLeft = 7,
	ShoulderRight = 8,
	ElbowRight = 9,
	WristRight = 10,
	HandRight = 11,
	HipLeft = 12,
	KneeLeft = 13,
	AnkleLeft = 14,
	FootLeft = 15,
	HipRight = 16,
	KneeRight = 17,
	AnkleRight = 18,
	FootRight = 19,
	SpineShoulder = 20,
	HandTipLeft = 21,
	ThumbLeft = 22,
	HandTipRight = 23,
	ThumbRight = 24,
	//HACK (OS): this compiler doesn't support 	Count = (ThumbRight + 1)
	Count = 25
};

UENUM(BlueprintType)
enum class EFLKinect_TrackingState : uint8
{
	Not_Tracked = 0, Inferred = 1, Tracking = 2
};

USTRUCT(BlueprintType)
struct FFLKinect_Body
{
	GENERATED_BODY()
	static const uint8 nJoints{ static_cast<uint8>(EFLKinect_JointType::Count) };

	UPROPERTY(BlueprintReadOnly, Category = "HandState")
		EFLKinect_HandState HandLeftState;

	UPROPERTY(BlueprintReadOnly, Category = "HandState")
		EFLKinect_HandState HandRightState;

	FVector positions[nJoints];
	FRotator orientations[nJoints];
	EFLKinect_TrackingState trackingStates[nJoints];
};


UCLASS()
class FLKINECTPLUGIN_API UKinectPluginFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Kinect")
		static FVector GetJointPosition(UPARAM(ref) FFLKinect_Body const& body, EFLKinect_JointType type);

	UFUNCTION(BlueprintPure, Category = "Kinect")
		static FRotator GetJointOrientation(UPARAM(ref) FFLKinect_Body const& body, EFLKinect_JointType type);

	UFUNCTION(BlueprintPure, Category = "Kinect")
		static EFLKinect_TrackingState GetTrackingState(UPARAM(ref) FFLKinect_Body const& body, EFLKinect_JointType type);

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		static void GetKinectTrackedBodies(TArray<bool>& out);

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		static void GetKinectBody(uint8 number, FFLKinect_Body& out);

	UFUNCTION(BlueprintPure, Category = "Kinect")
		static FVector2D GetHandInScreen();

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		static bool IsKinectBodyTracked(uint8 number);
	
	UFUNCTION(BlueprintCallable, Category = "Kinect")
		static void Stop();

	UFUNCTION(BlueprintCallable, Category = "Kinect|Debug")
		static bool GetFirstActiveKinectBody(FFLKinect_Body& out);

	UFUNCTION(BlueprintCallable, Category = "Kinect|Debug")
		static void GetAllPositions(uint8 num, TArray<FVector>& out);

	UFUNCTION(BlueprintCallable, Category = "Kinect|Debug")
		static void DrawBody(const AActor* worldActor, UPARAM(ref) FFLKinect_Body& body, FTransform transform, float PointSize, float LineLength);

};