package java.lang;

public class Long
{
	public static final long MIN_VALUE= 0x8000000000000000L;
	public static final long MAX_VALUE= 0x7fffffffffffffffL;
	
	/** Only the base 10 implementation of parseInt() is implemented yet. */
	public static long parseLong( String str )
	{
		// TODO: Rewrite this using str.getChar() when it is implemented. It will save us to make this char[]-copy.
		char[] chars= new char[str.length()];
		str.getChars( 0, str.length(), chars, 0 );
		
		long value= 0;
		for( int i= chars.length-1; i >= 0; i-- )
		{
			// Handle sign, if present
			if( i == 0 && chars[0] == '-' )
			{
				value= -value;
				break;
			}
			
			long intermediateValue= chars[i]-'0';
			
			if( intermediateValue > 9 )
				throw new NumberFormatException( "Could not parse this string to a base-10 number: " + str );
				
			// Multiply the extracted number by 10 according to its position. (Significance)
			for( int j= chars.length-1-i; j > 0; j-- )
				intermediateValue*= 10;
				
			// And finally add the parts together.
			value+= intermediateValue;	
		}
		
		return value;
	}

	// TODO: Base 10 implementation only. Write multi-base implementation.
	private static final int MAX_LENGTH= 64+1; 
	public static String toString( long value )
	{
		if( value == 0 )
			return "0";

		char[] buffer= new char[MAX_LENGTH+1];
		boolean sign= value < 0;
		
		if( sign )
			value= -value;
		
		int i;
		for( i= 0; value != 0; i++ )
		{
			buffer[MAX_LENGTH-i]= (char) ((value % 10) + '0');
			value/= 10;
		}
		
		if( sign )
		{
			i++;
			buffer[MAX_LENGTH-i]= '-';
		}
		
		return new String( buffer );
	}
}