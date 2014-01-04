package java.lang;

public class String
{
	private char[] value;
	
	public String( char[] otherValue )
	{
		value= new char[otherValue.length];
		
		// TODO: Optimize this if System.arraycopy() has been implemented!
		for( int i= 0; i < otherValue.length; i++ )
			value[i]= otherValue[i];
	}
	
	public String( String str )
	{
		this.value= str.value;
	}
	
	public int length()
	{
		return value.length;
	}
	
	public void getChars( int start, int length, char[] destination, int destStart )
	{
		// TODO: Optimize using System.arraycopy() if it has been implemented.
		int destPos= destStart;
		for( int i= start; i < length; i++, destPos++ )
			destination[destPos]= value[i];
	}
	
	public String toString()
	{
		return this;
	}
}