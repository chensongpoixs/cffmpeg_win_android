#!/bin/bash
export ANDROID_NDK_HOME=/usr/local/android-ndk-r14b
export PLATFORM_VERSION=android-9

function build_ffmpeg
{
	echo "start build ffmpeg $ARCH"
	./configure --target-os=linux \
	--prefix=$PREFIX \
	--disable-symver \
	--enable-avresample \
	--enable-small \
	--enable-jni \
	--arch=$ARCH \
	--enable-shared \
	--disable-static \
	--disable-yasm \
	--disable-ffmpeg \
	--disable-ffplay \
	--disable-ffprobe \
	--disable-ffserver \
	--disable-doc \
	--enable-cross-compile \
	--cross-prefix=$CROSS_COMPILE \
	--sysroot=$PLATFORM \
	--extra-cflags="-fpic"
	make clean
	make
	make install

	echo "build finished $ARCH"
}
#exec 1>chen_build_stdout.txt
#exec 2>chen_build_stdout.txt
#arm
export ARCH=arm
export CPU=arm
export PREFIX=$(pwd)/android/$CPU
export TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64
export CROSS_COMPILE=$TOOLCHAIN/bin/arm-linux-androideabi-
export PLATFORM=$ANDROID_NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH
build_ffmpeg

#x86
#ARCH=x86
#CPU=x86
#PREFIX=$(pwd)/android/$CPU
#TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/x86-4.9/prebuilt/linux-x86_64
#CROSS_COMPILE=$TOOLCHAIN/bin/i686-linux-android-
#PLATFORM=$ANDROID_NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH
#build_ffmpeg

#x86_64
#ARCH=x86_64
#CPU=x86_64
#PREFIX=$(pwd)/android/$CPU
#TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/x86_64-4.9/prebuilt/linux-x86_64
#CROSS_COMPILE=$TOOLCHAIN/bin/x86_64-linux-android-
#PLATFORM=$ANDROID_NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH
#build_ffmpeg

 
#aarch64
#ARCH=aarch64
#CPU=aarch64
#PREFIX=$(pwd)/android/$CPU
#TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
#CROSS_COMPILE=$TOOLCHAIN/bin/aarch64-linux-android-
#PLATFORM=$ANDROID_NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH
#build_ffmpeg

#mips
#ARCH=mips
#CPU=mips
#PREFIX=$(pwd)/android/$CPU
#TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/mipsel-linux-android-4.9/prebuilt/linux-x86_64
#CROSS_COMPILE=$TOOLCHAIN/bin/mipsel-linux-android-
#PLATFORM=$ANDROID_NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH
#build_ffmpeg


#mips64el
#ARCH=mips64el
#CPU=mips64el
#PREFIX=$(pwd)/android/$CPU
#TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/mips64el-linux-android-4.9/prebuilt/linux-x86_64
#CROSS_COMPILE=$TOOLCHAIN/bin/mips64el-linux-android-
#PLATFORM=$ANDROID_NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH
#build_ffmpeg
