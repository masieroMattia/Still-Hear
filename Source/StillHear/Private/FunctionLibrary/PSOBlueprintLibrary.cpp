#include "FunctionLibrary/PSOBlueprintLibrary.h"
#include "ShaderPipelineCache.h"

int32 UPSOBlueprintLibrary::GetPSORemaining()
{
	return FShaderPipelineCache::NumPrecompilesRemaining();
}

bool UPSOBlueprintLibrary::IsPSOCompilationComplete()
{
	return FShaderPipelineCache::NumPrecompilesRemaining() == 0;
}