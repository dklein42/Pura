public class VirtualMethodCall
{
	private static class A
	{
		public void test() {}
	}

	private static class B extends A
	{
		public void test() {}
	}

	private static class C extends B {}
	
	public static void main(String[] args)
	{
		A a= new A();
		B b= new B();
		C c= new C();
		
		a.test();
		b.test();
		c.test();
	}
}