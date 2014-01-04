public class InstanceVar
{
	private int testVar;
	
	private void test()
	{
		testVar++;
	}
	
	public static void main(String[] args)
	{
		new InstanceVar().test();
	}
}