public class SimpleExceptionTest
{
	public static void exceptionThrower()
	{
		throw new IllegalArgumentException( "This is a fake exception! :-) And it works!" );
	}
	
	public static void main( String[] args )
	{
		exceptionThrower();
	}
}