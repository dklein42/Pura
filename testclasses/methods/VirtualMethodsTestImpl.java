public class VirtualMethodsTestImpl implements VirtualMethodsTestInterface
{
	public String toString()
	{
		return "Funzt!";
	}
	
	public int add( int a, int b )
	{
		return a + b;
	}
	
	public final int sub( int a, int b )
	{
		return a - b;
	}
	
	public int interfaceMethod()
	{
		return VALUE;
	}
}