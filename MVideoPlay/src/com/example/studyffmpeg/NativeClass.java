package com.example.studyffmpeg;

import android.util.Log;

import com.example.studyffmpeg.R.string;

public class NativeClass {
	//java中被访问的数据
	public String s ="aaaaa";
	
	//字符串的处理啊
	public native String StringFromJNI();
	public native String StringCopyFrom(String OrgString);
	
	//数组的传输
	public native int sumArray1(int[] arr);
	public native int sumArray2(int[] arr);
	public static native int[][] initInt2DArray(int size);
	
	//c++访问java中的数据
	public native void accessField();
	
	//c++中回调java中的方法
	public native void nativeMethod();
	
	//ffmpeg的处理
	public native void  ffmpegTest();

	
	private void callBack(){
		Log.i("in java","java中的函数被调用了！！！！");
	}
	
	static {
		//System.loadLibrary("ffmpeg-neon");
		System.loadLibrary("ffmpeg_test");
	}
}