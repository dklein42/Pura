public class ArrayTests
{
	public static void main( String[]  args )
	{
		byte[][][] ar= new byte[100][100][100];

		for( int i= 0; i < ar.length; i++ )
			for( int j= 0; j< ar[0].length; j++ )
				for( int k= 0; k< ar[0][0].length; k++ )
				ar[i][j][k]= (byte)k;

		for( int i= 0; i < ar.length; i++ )
			for( int j= 0; j < ar[0].length; j++ )
				for( int k= 0; k< ar[0][0].length; k++ )
				{
					System.out.print( ar[i][j][k] );
					System.out.print( " " );
				}
				
		System.out.println();
		
		//////////////////

		System.out.println( new byte[0] );
		System.out.println( new int[5] );
		System.out.println( new int[1][2] );
		System.out.println( new int[1][2][3] );

		//////////////

		int[] ara= new int[0];
		System.out.println( ara );
		
		System.out.println( new byte[0].hashCode() );
		System.out.println( new int[0].toString() );

	}
}