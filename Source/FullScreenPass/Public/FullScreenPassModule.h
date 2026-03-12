#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "FullScreenPassSceneViewExtension.h"

class FFullScreenPassModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	TSharedPtr< class FFullScreenPassSceneViewExtension, ESPMode::ThreadSafe > ViewExtension;
};
