package java.lang;

public class IndexOutOfBoundsException extends RuntimeException
{
	public IndexOutOfBoundsException()
	{
		super();
	}

	public IndexOutOfBoundsException( String message )
	{
		super( message );
	}

	public IndexOutOfBoundsException( Throwable cause )
	{
		super( cause );
	}

	public IndexOutOfBoundsException( String message, Throwable cause )
	{
		super( message, cause );
	}
}