package kr.selfcontrol.exeemulator;

public class dasm {
    // libNDKTest.so �� �ε��մϴ�
    static { System.loadLibrary( "dasm" ); }
 
    // native �Լ��� ����ϴ�
    public static native int fileLoad(String s);
    public static native String getPrintBuffer();
	public static native int isFinish();
    public static native String readNext();
    public static native int getInputType();
    public static native void inputChar(String s);
    public static native void inputFloat(float f);
    public static native void inputInt(int i);
    public static native void printBufferClear();


    public native void callBackTest();
    public void printStr(){
        System.out.println("print Test");
    }
}
