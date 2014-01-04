public class B extends A
{
	private void privTest()
	{
		System.out.println( "Private Method!" );
	}
	
   public void doTest()
   {
      System.out.println("This is class B!");
   }

	public void test()
	{
		privTest();
		doTest();
		super.doTest();
	}

	public static void main(String[] args)
	{
		B b= new B();
		b.test();
	}
}
