package java.lang;

import java.io.PrintStream;

public class Throwable
{
	private String detailMessage;
	private Throwable cause= this;
	private StackTraceElement[] stackTrace;
	
	public Throwable()
	{
		getOurStackTrace();		
	}
	
	public Throwable( String message )
	{
		getOurStackTrace();		
		detailMessage= message;
	}
	
	public Throwable( Throwable cause )
	{
		getOurStackTrace();		
		this.cause= cause;
	}
	
	public Throwable( String message, Throwable cause )
	{
		getOurStackTrace();		
		detailMessage= message;
		this.cause= cause;
	}
	
	public String getMessage()
	{
		return detailMessage;
	}
	
	public Throwable getCause()
	{
		return cause == this ? null : cause;
	}
	
	public Throwable initCause( Throwable cause )
	{
		if( this.cause != this )
			throw new IllegalStateException( "Can't overwrite cause." );
		if( cause == this )
			throw new IllegalArgumentException( "Self-causation not permitted." );
			
		this.cause= cause;
		return this;
	}
	
	public void printStackTrace()
	{
		printStackTrace(System.err);
	}
	
	// TODO: Do this nicer like in the Sun JDK.
	public void printStackTrace( PrintStream s )
	{
		s.println( this );
		
		for( int i= 0; i < stackTrace.length; i++ )
			s.println( "\tat " + stackTrace[i] );
			
		Throwable ourCause= getCause();
		if( ourCause != null )
		{
			s.print( "Caused by: " );
			ourCause.printStackTrace( s );
		}
	}
	
	public StackTraceElement[] getStackTrace()
	{
		// TODO: We should return a copy of the stack trace elements array here. It would be nice to have the clone()
		// Method for arrays available for this. ;-)
		return stackTrace;
	}
	
	private native int getStackTraceDepth();
	private native StackTraceElement getStackTraceElement( int index );
	
	private StackTraceElement[] getOurStackTrace()
	{
		if( stackTrace != null)
			return stackTrace;
			
		int depth= getStackTraceDepth();
		stackTrace= new StackTraceElement[depth];
		
		for( int i= 0; i < depth; i++ )
			stackTrace[i]= getStackTraceElement( i );
			
		return stackTrace;
	}
	
	public String toString()
	{
		StringBuilder sb= new StringBuilder();
		return sb.append( getClassName() ).append( ": " ).append( detailMessage ).toString(); 
	}
}