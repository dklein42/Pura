//
//  Test.java -- Test class for the Pura JVM.
// 
//  Created by Daniel Klein on 03.08.06.

public class Test
{
	public static void main( String[] args )
	{
		Test t1= new Test();
		Object t2= new String( "Babe" );
		
		System.out.println( "One object: " + t1 );
		System.out.println( "Another object: " + t2 );
		System.out.println( "And a last one: " + new Object() );
		
		System.out.println( "Equals test 1: " + t1.equals(t2) );
		System.out.println( "Equals test 2: " + t1.equals(t1) );

		System.out.println( "Equality operator test 1: " + (t1 == t2) );
		System.out.println( "Equality operator test 2: " + (t1 == t1) );
	}
}
