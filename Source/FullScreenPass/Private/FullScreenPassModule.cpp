#include "FullScreenPassModule.h"
#include "FullScreenPassLog.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "SceneViewExtension.h"
#include "ShaderCore.h"


#define LOCTEXT_NAMESPACE "FFullScreenPassModule"

void FFullScreenPassModule::StartupModule()
{
	UE_LOG(FullScreenPass, Log, TEXT("FFullScreenPassModule startup"));

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FullScreenPass"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/FullScreenPass"), PluginShaderDir);
	
	FCoreDelegates::OnPostEngineInit.AddLambda([this]() {
		ViewExtension = FSceneViewExtensions::NewExtension<FFullScreenPassSceneViewExtension>();
	});

}

void FFullScreenPassModule::ShutdownModule()
{
	UE_LOG(FullScreenPass, Log, TEXT("FFullScreenPassModule shutdown"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFullScreenPassModule, FullScreenPass);
