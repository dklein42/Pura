public class VirtualMethodsTest
{
	public static void main( String[] args )
	{
		VirtualMethodsTestImpl test= new VirtualMethodsTestImpl();
		Object obj= test;
		VirtualMethodsTestInterface interf= test;
		
		System.out.println( "Impl reference: " + test );
		System.out.println( "Object reference: " + obj );
		System.out.println( "Object method via Impl ref: " + test.hashCode() );
		System.out.println( "Object method via Object ref: " + obj.hashCode() );
		System.out.println( "Direct new Object: " + new Object() );
		System.out.println();
		System.out.println( "Interface method via impl ref: " + test.interfaceMethod() );
		System.out.println( "Interface method vir interface ref: " + interf.interfaceMethod() );
	}
}