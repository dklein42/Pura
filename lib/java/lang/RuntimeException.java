package java.lang;

public class RuntimeException extends Exception
{
	public RuntimeException()
	{
		super();
	}

	public RuntimeException( String message )
	{
		super( message );
	}

	public RuntimeException( Throwable cause )
	{
		super( cause );
	}

	public RuntimeException( String message, Throwable cause )
	{
		super( message, cause );
	}
}