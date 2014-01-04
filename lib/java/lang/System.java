package java.lang;

public class System
{
	public static java.io.PrintStream out= new java.io.PrintStream();
	public static java.io.PrintStream err= out;
	
	public static native long currentTimeMillis();
}