public class B extends A
{
	public int b= 99;
	public static int d= 4;
	
	public static void main( String[] args )
	{
		A a= new A();
		B b= new B();
		
		int z= a.a;
		z= a.b;
		z= a.c;
		
		z= b.a;
		z= b.b;
		z= b.c;
		z= b.d;
		
	}
}