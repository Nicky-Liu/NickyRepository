#!/bin/sh
export PLATFORM=$NDK/platforms/android-8/arch-arm/
export PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86
LOCAL_ARM_NEON=true
CPU=armv7-a
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -marm -mcpu=cortex-a8"
export PREFIX=./android/$CPU
build_one(){
./configure --target-os=linux \
--prefix=$PREFIX \
--enable-cross-compile \
--arch=arm \
--enable-nonfree \
--enable-asm \
--cpu=cortex-a8 \
--enable-neon \
--cc=$PREBUILT/bin/arm-linux-androideabi-gcc \
--cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
--nm=$PREBUILT/bin/arm-linux-androideabi-nm \
--sysroot=$PLATFORM \
--extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 $OPTIMIZE_CFLAGS " \
--disable-shared \
--enable-static \
--extra-ldflags="-Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -nostdlib -lc -lm -ldl -llog" \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-ffserver \
--disable-encoders \
--enable-avformat \
--disable-optimizations \
--disable-doc \
--enable-pthreads \
--disable-yasm \
--enable-zlib \
--enable-pic \
--enable-small
  
#make clean
make  -j4 install

$PREBUILT/bin/arm-linux-androideabi-ar d libavcodec/libavcodec.a inverse.o

$PREBUILT/bin/arm-linux-androideabi-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg-neon.so -shared -nostdlib  -z noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg-neon.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a  libavfilter/libavfilter.a libswresample/libswresample.a libswscale/libswscale.a libavdevice/libavdevice.a -lc -lm -lz -ldl -llog  --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/arm-linux-androideabi/4.4.3/libgcc.a
}

build_one
