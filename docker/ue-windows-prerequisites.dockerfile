# escape=`
ARG NAMESPACE
ARG PREREQS_TAG

# qwave.dll ships only with the Windows Desktop Experience and is absent from the Windows Server Core
# base image. UE 5.8 makes the Unreal Build Accelerator the mandatory local executor, and its native
# UbaHost.dll imports qwave.dll at load time. Without it UbaHost.dll fails to load and the engine build
# dies with a misleading "UBA executor is not expected to be invoked from a recursive UBT call." error.
# Pull qwave.dll from the full Windows image (the same DLL source ue4-docker uses for the other
# Desktop-Experience system DLLs) and add it to the prerequisites image's System32.
FROM mcr.microsoft.com/windows/server:ltsc2022 AS dlls

FROM ${NAMESPACE}/ue4-base-build-prerequisites:${PREREQS_TAG}
COPY --from=dlls C:\Windows\System32\qwave.dll C:\Windows\System32\
