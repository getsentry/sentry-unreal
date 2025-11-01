ARG NAMESPACE
ARG PREREQS_TAG
FROM ${NAMESPACE}/ue4-base-build-prerequisites:${PREREQS_TAG}

# ARG must be re-declared after FROM to be available in this build stage
ARG UNREAL_VERSION

# Switch to root to install additional packages
USER root

# Determine versions and install packages
# Store versions in a shell script that can be sourced
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
    echo "JAVA_VERSION=${JAVA_VERSION}" > /tmp/android_env.sh && \
    echo "ANDROID_SDK_VERSION=${ANDROID_SDK_VERSION}" >> /tmp/android_env.sh && \
    echo "ANDROID_BUILD_TOOLS=${ANDROID_BUILD_TOOLS}" >> /tmp/android_env.sh && \
    echo "NDK_VERSION=${NDK_VERSION}" >> /tmp/android_env.sh && \
    echo "JAVA_HOME=/usr/lib/jvm/java-${JAVA_VERSION}-openjdk-amd64" >> /tmp/android_env.sh

# Set JAVA_HOME as ENV (read from the temp file in a way Docker can use)
RUN export $(cat /tmp/android_env.sh | xargs) && \
    mkdir -p /etc/profile.d && \
    echo "export JAVA_HOME=${JAVA_HOME}" > /etc/profile.d/android.sh && \
    echo "export ANDROID_HOME=/home/ue4/android-sdk" >> /etc/profile.d/android.sh && \
    echo "export ANDROID_SDK_ROOT=/home/ue4/android-sdk" >> /etc/profile.d/android.sh

# Set Android SDK environment variables
ENV ANDROID_HOME=/home/ue4/android-sdk
ENV ANDROID_SDK_ROOT=/home/ue4/android-sdk
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

# Accept licenses and install Android SDK components based on UE version
RUN . /tmp/android_env.sh && \
    yes | ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager --licenses && \
    ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager \
        "platform-tools" \
        "platforms;android-${ANDROID_SDK_VERSION}" \
        "build-tools;${ANDROID_BUILD_TOOLS}" \
        "ndk;${NDK_VERSION}" && \
    echo "NDK_PATH=/home/ue4/android-sdk/ndk/${NDK_VERSION}" >> /tmp/android_env.sh

# Create environment setup script that will be sourced
RUN . /tmp/android_env.sh && \
    echo "#!/bin/bash" > /etc/profile.d/android-env.sh && \
    echo "export JAVA_HOME=${JAVA_HOME}" >> /etc/profile.d/android-env.sh && \
    echo "export ANDROID_HOME=/home/ue4/android-sdk" >> /etc/profile.d/android-env.sh && \
    echo "export ANDROID_SDK_ROOT=/home/ue4/android-sdk" >> /etc/profile.d/android-env.sh && \
    echo "export ANDROID_NDK=${NDK_PATH}" >> /etc/profile.d/android-env.sh && \
    echo "export ANDROID_NDK_ROOT=${NDK_PATH}" >> /etc/profile.d/android-env.sh && \
    echo "export NDKROOT=${NDK_PATH}" >> /etc/profile.d/android-env.sh && \
    echo "export NDK_ROOT=${NDK_PATH}" >> /etc/profile.d/android-env.sh && \
    chmod +x /etc/profile.d/android-env.sh

# Now set these as Docker ENV variables by sourcing and echoing to a Dockerfile snippet
# This is a workaround since we can't use ENV with runtime values
USER root
RUN . /tmp/android_env.sh && \
    echo "ENV JAVA_HOME=${JAVA_HOME}" > /tmp/final_env.dockerfile && \
    echo "ENV ANDROID_NDK=${NDK_PATH}" >> /tmp/final_env.dockerfile && \
    echo "ENV ANDROID_NDK_ROOT=${NDK_PATH}" >> /tmp/final_env.dockerfile && \
    echo "ENV NDKROOT=${NDK_PATH}" >> /tmp/final_env.dockerfile && \
    echo "ENV NDK_ROOT=${NDK_PATH}" >> /tmp/final_env.dockerfile

# Source the android environment in bashrc for ue4 user
USER ue4
RUN echo ". /etc/profile.d/android-env.sh" >> ~/.bashrc
