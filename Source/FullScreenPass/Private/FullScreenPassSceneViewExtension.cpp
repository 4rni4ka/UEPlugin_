#include "FullScreenPassSceneViewExtension.h"
#include "FullScreenPassShaders.h"

#include "Engine/World.h"
#include "FXRenderingUtils.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "SceneView.h"
#include "StereoRendering.h"
#include "DynamicResolutionState.h"
#include "PostProcess/PostProcessInputs.h"

static TAutoConsoleVariable<int32> CVarEnabled(
	TEXT("r.FSP"),
	1,
	TEXT("Controls Full Screen Pass plugin\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled (default)"));

static TAutoConsoleVariable<float> CVarOpacity(
	TEXT("r.FSP.Opacity"),
	0.1f,
	TEXT("Fog opacity multiplier."));

static TAutoConsoleVariable<float> CVarFogStart(
	TEXT("r.FSP.FogStart"),
	500.0f,
	TEXT("Distance at which fog starts."));

static TAutoConsoleVariable<float> CVarFogCurveMin(
	TEXT("r.FSP.FogCurveMin"),
	0.0f,
	TEXT("Fog smoothstep min distance."));

static TAutoConsoleVariable<float> CVarFogCurveMax(
	TEXT("r.FSP.FogCurveMax"),
	1000.0f,
	TEXT("Fog smoothstep max distance."));

static TAutoConsoleVariable<float> CVarFogColorR(
	TEXT("r.FSP.FogColorR"),
	0.5f,
	TEXT("Fog color R."));

static TAutoConsoleVariable<float> CVarFogColorG(
	TEXT("r.FSP.FogColorG"),
	0.5f,
	TEXT("Fog color G."));

static TAutoConsoleVariable<float> CVarFogColorB(
	TEXT("r.FSP.FogColorB"),
	0.5f,
	TEXT("Fog color B."));

namespace
{
bool ShouldRenderFullScreenPass(const FSceneView& View)
{
	if (!View.bIsGameView ||
		View.bIsSceneCapture ||
		View.bIsSceneCaptureCube ||
		View.bIsReflectionCapture ||
		View.bIsPlanarReflection ||
		View.bIsVirtualTexture ||
		View.bIsOfflineRender)
	{
		return false;
	}

	if (View.Family == nullptr || !View.Family->EngineShowFlags.PostProcessing)
	{
		return false;
	}

	const FIntRect& ViewRect = View.UnscaledViewRect;
	if (ViewRect.Width() <= 0 || ViewRect.Height() <= 0)
	{
		return false;
	}

	return !IStereoRendering::IsASecondaryView(View);
}
}

FFullScreenPassSceneViewExtension::FFullScreenPassSceneViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
{
}

bool FFullScreenPassSceneViewExtension::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	if (CVarEnabled.GetValueOnGameThread() == 0)
	{
		return false;
	}

	const UWorld* World = Context.GetWorld();
	if (World == nullptr || !World->IsGameWorld())
	{
		return false;
	}

	return Context.Viewport != nullptr;
}

void FFullScreenPassSceneViewExtension::PrePostProcessPass_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessingInputs& Inputs
)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("FullScreenPass::PrePostProcessPass_RenderThread");

	if (CVarEnabled.GetValueOnRenderThread() == 0 || !ShouldRenderFullScreenPass(View))
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
	SceneColorDesc.ClearValue = FClearValueBinding(FLinearColor(0.f, 0.f, 0.f, 0.f));

	FRDGTexture* ResultTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("FullScreenPassResult"));
	FScreenPassRenderTarget ResultRenderTarget(ResultTexture, SceneColor.ViewRect, ERenderTargetLoadAction::EClear);

	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FScreenPassVS> ScreenPassVS(GlobalShaderMap);
	TShaderMapRef<FFullScreenPassPS> ScreenPassPS(GlobalShaderMap);

	FFullScreenPassPS::FParameters* Parameters = GraphBuilder.AllocParameters<FFullScreenPassPS::FParameters>();
	Parameters->View = View.ViewUniformBuffer;
	Parameters->SceneTexturesStruct = Inputs.SceneTextures;

	Parameters->Opacity = CVarOpacity.GetValueOnRenderThread();
	Parameters->FogStart = CVarFogStart.GetValueOnRenderThread();
	Parameters->FogCurve = FVector2f(
		CVarFogCurveMin.GetValueOnRenderThread(),
		CVarFogCurveMax.GetValueOnRenderThread()
	);
	Parameters->FogColor = FVector4f(
		CVarFogColorR.GetValueOnRenderThread(),
		CVarFogColorG.GetValueOnRenderThread(),
		CVarFogColorB.GetValueOnRenderThread(),
		1.0f
	);

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
