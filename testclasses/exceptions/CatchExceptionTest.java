public class CatchExceptionTest
{
	private static void test1()
	{
		try
		{
			throw new IllegalArgumentException( "This exception should get caught." );
		}
		catch( IllegalArgumentException e )
		{
			System.out.println( "Exception caught!" );
			e.printStackTrace();
		}
	}
	
	private static void test2()
	{
		try
		{
			throw new IllegalArgumentException( "This exception should also get caught." );
		}
		catch( Exception e )
		{
			System.out.println( "Another exception caught!" );
			e.printStackTrace();
		}
	}
	
	private static void test3()
	{
		throw new IllegalArgumentException( "This exception should get caught." );
	}
	
	private static void test4()
	{
		try
		{
			throw new RuntimeException( "This is an uncaught chained exception." );
		}
		catch( Exception e )
		{
			throw new IllegalArgumentException( "Rethrowing exception", (Throwable)e );
		}
	}
	
	public static void main( String[] args )
	{
		test1();
		test2();
		
		try
		{
			test3();
		}
		catch( Exception e )
		{
			System.out.println( "The final exception caught!" );
			e.printStackTrace();
		}
		
		test4();
	}
}