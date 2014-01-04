package java.lang;

public final class StackTraceElement
{
	// CAUTION: Do not change instance variable positions! (The JVM accesses them natively by position.)
	private String declaringClass;
	private String methodName;
	private String fileName;
	private int lineNumber;
	
	public String getClassName()
	{
		return declaringClass;
	}
	
	public String getMethodName()
	{
		return methodName;
	}
	
	public String getFileName()
	{
		return fileName;
	}
	
	public int getLineNumber()
	{
		return lineNumber;
	}
	
	public String toString()
	{
		StringBuilder sb= new StringBuilder();
		
		sb.append( declaringClass );
		sb.append( "." );
		sb.append( methodName );

		if( isNativeMethod() )
		{
			sb.append( "(Native Method)" );
		}
		else
		{
			if( fileName != null && lineNumber > 0 )
				sb.append( "(" ).append( fileName ).append( ":" ).append( lineNumber ).append( ")" );
			else if( fileName != null )
				sb.append( "(" ).append( fileName ).append( ")" );
			else
				sb.append( "(Unknown Source)" );
		}
		
		return sb.toString();
	}
	
	public boolean isNativeMethod()
	{
		return lineNumber == -2;
	}
}