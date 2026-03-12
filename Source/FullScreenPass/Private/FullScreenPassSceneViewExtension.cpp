#include "FullScreenPassSceneViewExtension.h"
#include "FullScreenPassShaders.h"

#include "FXRenderingUtils.h"
#include "PostProcess/PostProcessInputs.h"
#include "DynamicResolutionState.h"


static TAutoConsoleVariable<int32> CVarEnabled(
	TEXT("r.FSP"),
	1,
	TEXT("Controls Full Screen Pass plugin\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled (default)"));

static TAutoConsoleVariable<float> CVarEffectStrenght(
	TEXT("r.FSP.EffectStrenght"),
	0.1f,
	TEXT("Controls an amount of depth buffer blending to the base color."));


FFullScreenPassSceneViewExtension::FFullScreenPassSceneViewExtension(const FAutoRegister& AutoRegister) :
	FSceneViewExtensionBase(AutoRegister)
{

}

void FFullScreenPassSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	if (CVarEnabled->GetInt() == 0)
	{
		return;
	}

	Inputs.Validate();

	const FIntRect PrimaryViewRect = UE::FXRenderingUtils::GetRawViewRectUnsafe(View);

	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

	if (!SceneColor.IsValid())
	{
		return;
	}

	const FScreenPassTextureViewport Viewport(SceneColor);

	FRDGTextureDesc SceneColorDesc = SceneColor.Texture->Desc;

	SceneColorDesc.Format = PF_FloatRGBA;
	FLinearColor ClearColor(0., 0., 0., 0.);
	SceneColorDesc.ClearValue = FClearValueBinding(ClearColor);

	FRDGTexture* ResultTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("FulllScreenPassResult"));
	FScreenPassRenderTarget ResultRenderTarget = FScreenPassRenderTarget(ResultTexture, SceneColor.ViewRect, ERenderTargetLoadAction::EClear);
	
	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FFullScreenPassVS> ScreenPassVS(GlobalShaderMap);
	TShaderMapRef<FFullScreenPassPS> ScreenPassPS(GlobalShaderMap);

	FFullScreenPassPS::FParameters* Parameters = GraphBuilder.AllocParameters<FFullScreenPassPS::FParameters>();
	Parameters->View = View.ViewUniformBuffer;
	Parameters->SceneTexturesStruct = Inputs.SceneTextures;
	Parameters->Strenght = CVarEffectStrenght->GetFloat();

	Parameters->RenderTargets[0] = ResultRenderTarget.GetRenderTargetBinding();

	AddDrawScreenPass(
		GraphBuilder,
		RDG_EVENT_NAME("FullScreenPassShader"),
		View,
		Viewport,
		Viewport,
		ScreenPassVS,
		ScreenPassPS,
		Parameters
	);

	AddCopyTexturePass(GraphBuilder, ResultTexture, SceneColor.Texture);
}