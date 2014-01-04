public class StaticMethodsTest
{
	public static void simple()
	{
		return;
	}
	
	public static final int retVal()
	{
		return 0xFF;
	}
	
	public static int two( int a, int b )
	{
		int z= a + b;
		return three( a, b, 3 );
	}
	
	public static int three( int a, int b, int c )
	{
		int z= a + b + c;
		return -1;
	}
	
	public static void main( String[] args )
	{
		simple();
		int x= retVal();
		int y= two( 1, 2 );
		int z= x + y;
	}
}