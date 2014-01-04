package java.lang;

public class ArrayIndexOutOfBoundsException extends IndexOutOfBoundsException
{
	public ArrayIndexOutOfBoundsException()
	{
		super();
	}

	public ArrayIndexOutOfBoundsException( String message )
	{
		super( message );
	}

	public ArrayIndexOutOfBoundsException( Throwable cause )
	{
		super( cause );
	}

	public ArrayIndexOutOfBoundsException( String message, Throwable cause )
	{
		super( message, cause );
	}
	
	public ArrayIndexOutOfBoundsException( int index )
	{
		super( "Array index out of bounds: " + index );
	}
}