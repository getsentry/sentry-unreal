ARG NAMESPACE
ARG PREREQS_TAG
FROM ${NAMESPACE}/ue4-base-build-prerequisites:${PREREQS_TAG}

# Switch to root to install additional packages
USER root

# Install Java 17 (required for UE 5.5 Android builds)
RUN apt-get update && apt-get install -y --no-install-recommends \
    openjdk-17-jdk \
    wget \
    unzip && \
    rm -rf /var/lib/apt/lists/*

# Set up environment variables for Android SDK/NDK
# Unreal Engine checks multiple environment variable names, so we set all of them
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
ENV ANDROID_HOME=/home/ue4/android-sdk
ENV ANDROID_SDK_ROOT=/home/ue4/android-sdk
ENV ANDROID_NDK=/home/ue4/android-sdk/ndk/25.1.8937393
ENV ANDROID_NDK_ROOT=/home/ue4/android-sdk/ndk/25.1.8937393
ENV NDKROOT=/home/ue4/android-sdk/ndk/25.1.8937393
ENV NDK_ROOT=/home/ue4/android-sdk/ndk/25.1.8937393
ENV PATH=${PATH}:${ANDROID_HOME}/cmdline-tools/latest/bin:${ANDROID_HOME}/platform-tools

# Switch to ue4 user for SDK installation (to ensure correct permissions)
USER ue4

# Download and install Android command-line tools
RUN mkdir -p ${ANDROID_HOME}/cmdline-tools && \
    cd ${ANDROID_HOME}/cmdline-tools && \
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip && \
    unzip -q commandlinetools-linux-9477386_latest.zip && \
    rm commandlinetools-linux-9477386_latest.zip && \
    mv cmdline-tools latest

# Accept licenses and install Android SDK components
# UE 5.5 requires: API Level 34, Build Tools 34.0.0, NDK r25b (25.1.8937393)
RUN yes | ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager --licenses && \
    ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager \
    "platform-tools" \
    "platforms;android-34" \
    "build-tools;34.0.0" \
    "ndk;25.1.8937393"
