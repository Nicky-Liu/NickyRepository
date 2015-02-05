package com.example.studyffmpeg;

public class FfmpegCls {
	public native void ffmpegTest();
	
	static {
		System.loadLibrary("ffmpeg-neon");
		System.loadLibrary("ffmpeg_test");
	}
}
