#include "VulkanValidate.h"

std::string te::vkh::errorString(vk::Result errorCode)
{
	switch (errorCode)
	{
             
#define STR(r) case vk::Result:: ##r: return #r
		STR(eNotReady);
		STR(eTimeout);
		STR(eEventSet);
		STR(eEventReset);
		STR(eIncomplete);
		STR(eErrorOutOfHostMemory);
		STR(eErrorOutOfDeviceMemory);
		STR(eErrorInitializationFailed);
		STR(eErrorDeviceLost);
		STR(eErrorMemoryMapFailed);
		STR(eErrorLayerNotPresent);
		STR(eErrorExtensionNotPresent);
		STR(eErrorFeatureNotPresent);
		STR(eErrorIncompatibleDriver);
		STR(eErrorTooManyObjects);
		STR(eErrorFormatNotSupported);
		STR(eErrorFragmentedPool);
		STR(eErrorUnknown);
		STR(eErrorOutOfPoolMemory);
		STR(eErrorInvalidExternalHandle);
		STR(eErrorFragmentation);
		STR(eErrorInvalidOpaqueCaptureAddress);
		STR(eErrorSurfaceLostKHR);
        STR(eErrorNativeWindowInUseKHR);
        STR(eSuboptimalKHR);
        STR(eErrorOutOfDateKHR);
        STR(eErrorIncompatibleDisplayKHR);
        STR(eErrorValidationFailedEXT);
        STR(eErrorInvalidShaderNV);
        STR(eErrorInvalidDrmFormatModifierPlaneLayoutEXT);
        STR(eErrorNotPermittedEXT);
        STR(eErrorFullScreenExclusiveModeLostEXT);
        STR(eThreadIdleKHR);
        STR(eThreadDoneKHR);
        STR(eOperationDeferredKHR);
        STR(eOperationNotDeferredKHR);
        STR(ePipelineCompileRequiredEXT);
         
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}