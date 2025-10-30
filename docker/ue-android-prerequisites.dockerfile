ARG NAMESPACE
ARG PREREQS_TAG
FROM ${NAMESPACE}/ue4-base-build-prerequisites:${PREREQS_TAG}

# ARG must be re-declared after FROM to be available in this build stage
ARG UNREAL_VERSION

# Switch to root to install additional packages
USER root

RUN echo "Installing Android prerequisites for UE ${UNREAL_VERSION}"

# Determine Java, NDK, and SDK versions based on UE version
RUN JAVA_VERSION="" && \
    ANDROID_SDK_VERSION="" && \
    ANDROID_BUILD_TOOLS="" && \
    NDK_VERSION="" && \
    case "${UNREAL_VERSION}" in \
        5.6) \
            JAVA_VERSION="21" && \
            ANDROID_SDK_VERSION="34" && \
            ANDROID_BUILD_TOOLS="34.0.0" && \
            NDK_VERSION="27.2.12479018" \
            ;; \
        5.5) \
            JAVA_VERSION="17" && \
            ANDROID_SDK_VERSION="34" && \
            ANDROID_BUILD_TOOLS="34.0.0" && \
            NDK_VERSION="25.1.8937393" \
            ;; \
        5.4|5.3) \
            JAVA_VERSION="17" && \
            ANDROID_SDK_VERSION="33" && \
            ANDROID_BUILD_TOOLS="33.0.1" && \
            NDK_VERSION="25.1.8937393" \
            ;; \
        5.2|5.1) \
            JAVA_VERSION="11" && \
            ANDROID_SDK_VERSION="32" && \
            ANDROID_BUILD_TOOLS="32.0.0" && \
            NDK_VERSION="25.1.8937393" \
            ;; \
        4.27) \
            JAVA_VERSION="11" && \
            ANDROID_SDK_VERSION="29" && \
            ANDROID_BUILD_TOOLS="30.0.3" && \
            NDK_VERSION="21.1.6352462" \
            ;; \
        *) \
            echo "ERROR: Unsupported UE version ${UNREAL_VERSION}" && \
            exit 1 \
            ;; \
    esac && \
    apt-get update && apt-get install -y --no-install-recommends \
        openjdk-${JAVA_VERSION}-jdk \
        wget \
        unzip && \
    rm -rf /var/lib/apt/lists/* && \
    echo "${JAVA_VERSION}" > /tmp/java_version && \
    echo "${ANDROID_SDK_VERSION}" > /tmp/sdk_version && \
    echo "${ANDROID_BUILD_TOOLS}" > /tmp/build_tools_version && \
    echo "${NDK_VERSION}" > /tmp/ndk_version

# Set up environment variables for Android SDK/NDK
# Note: NDK path will be set after installation with the correct version
ENV ANDROID_HOME=/home/ue4/android-sdk
ENV ANDROID_SDK_ROOT=/home/ue4/android-sdk
ENV PATH=${PATH}:${ANDROID_HOME}/cmdline-tools/latest/bin:${ANDROID_HOME}/platform-tools

# Determine Java home based on installed version
RUN JAVA_VERSION=$(cat /tmp/java_version) && \
    echo "export JAVA_HOME=/usr/lib/jvm/java-${JAVA_VERSION}-openjdk-amd64" >> /etc/environment

# Switch to ue4 user for SDK installation (to ensure correct permissions)
USER ue4

# Download and install Android command-line tools
RUN mkdir -p ${ANDROID_HOME}/cmdline-tools && \
    cd ${ANDROID_HOME}/cmdline-tools && \
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip && \
    unzip -q commandlinetools-linux-9477386_latest.zip && \
    rm commandlinetools-linux-9477386_latest.zip && \
    mv cmdline-tools latest

# Accept licenses and install Android SDK components based on UE version
RUN ANDROID_SDK_VERSION=$(cat /tmp/sdk_version) && \
    ANDROID_BUILD_TOOLS=$(cat /tmp/build_tools_version) && \
    NDK_VERSION=$(cat /tmp/ndk_version) && \
    yes | ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager --licenses && \
    ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager \
        "platform-tools" \
        "platforms;android-${ANDROID_SDK_VERSION}" \
        "build-tools;${ANDROID_BUILD_TOOLS}" \
        "ndk;${NDK_VERSION}"

# Set NDK environment variables now that we know the exact version
RUN NDK_VERSION=$(cat /tmp/ndk_version) && \
    echo "export ANDROID_NDK=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> ~/.bashrc && \
    echo "export ANDROID_NDK_ROOT=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> ~/.bashrc && \
    echo "export NDKROOT=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> ~/.bashrc && \
    echo "export NDK_ROOT=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> ~/.bashrc

# Set NDK environment variables for the image
USER root
RUN NDK_VERSION=$(cat /tmp/ndk_version) && \
    echo "export ANDROID_NDK=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> /etc/environment && \
    echo "export ANDROID_NDK_ROOT=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> /etc/environment && \
    echo "export NDKROOT=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> /etc/environment && \
    echo "export NDK_ROOT=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> /etc/environment
USER ue4

RUN echo "Android prerequisites installation completed for UE ${UNREAL_VERSION}"
