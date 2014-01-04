//
//  Test.java -- Test class for the Pura JVM.
// 
//  Created by Daniel Klein on 03.08.06.

public class Test implements MyInterface
{
	public int iadd( int a, int b )
	{
		return a + b;
	}
	
	public void simpleCall()
	{
		
	}
	
	public static void main( String[] args )
	{
		MyInterface myInt= new Test();
		int res= myInt.iadd(2, MY_VALUE);
		//System.out.println( res );
	}
}
