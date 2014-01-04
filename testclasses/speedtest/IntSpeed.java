public class IntSpeed
{
	public static void main( String[] args )
	{
		long start= System.currentTimeMillis();
		
		for( int i= 100000000; i > 0; i-- )
			;
			
		long end= System.currentTimeMillis();
		System.out.println( "Time taken: " + (end-start) );
	}
}