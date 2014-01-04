package java.io;

public class PrintStream
{
	public native void print( java.lang.String str );
	public native void print( int i );
	public native void print( long l );
	public native void print( float f );
	public native void print( double d );

	public void print( Object obj )
	{
		print( obj.toString() );
	}
	
	public void print( boolean bool )
	{
		print( bool ? "true" : "false" );
	}

	public void println( java.lang.String str )
	{
		print( str );
		print( "\n" );
	}
	
	public void println( int i )
	{
		print( i );
		print( "\n" );
	}
	
	public void println( long l )
	{
		print( l );
		print( "\n" );
	}
	
	public void println( float f )
	{
		print( f );
		print( "\n" );
	}
	
	public void println( double d )
	{
		print( d );
		print( "\n" );
	}

	public void println( Object obj )
	{
		println( obj.toString() );
	}
	
	public void println( boolean bool )
	{
		print( bool ? "true" : "false" );
		print( "\n" );
	}
	
	public void println()
	{
		print( "\n" );
	}
}