package ru.corristo.eclipse.powerdetect;

public class PowerSourceDetector {
	
	static {
		System.loadLibrary("powersrc");
	}
	
	public static native void initialize();
	
	public static native void destroy();
}
