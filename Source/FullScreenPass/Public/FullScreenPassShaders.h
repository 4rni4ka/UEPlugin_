#pragma once
#include "CoreMinimal.h"
#include "ScreenPass.h"
#include "SceneTexturesConfig.h"


class FFullScreenPassVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFullScreenPassVS);

	FFullScreenPassVS() = default;
	FFullScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}
};

class FFullScreenPassPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FFullScreenPassPS);
	SHADER_USE_PARAMETER_STRUCT(FFullScreenPassPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		RENDER_TARGET_BINDING_SLOTS()
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneTextureUniformParameters, SceneTexturesStruct)
		SHADER_PARAMETER(float, Strenght)
	END_SHADER_PARAMETER_STRUCT()
};