// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "FLKinectPluginPrivatePCH.h"
#include "KinectPluginCore.h"


class FFLKinectPlugin : public IFLKinectPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FFLKinectPlugin, FLKinectPlugin )


void FFLKinectPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	
	UE_LOG(LogTemp, Log, TEXT("Loaded KinectPluginCore"))
	//FKinectPluginCore* kinect = FKinectPluginCore::GetInstance();
	
}


void FFLKinectPlugin::ShutdownModule()
{
	
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	FKinectPluginCore::GetInstance()->Stop();

}


