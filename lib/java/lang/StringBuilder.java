package java.lang;

public class StringBuilder
{
	private char[] data;
	private int position;
	
	public StringBuilder()
	{
		this( 16 );
	}
	
	public StringBuilder( int capacity )
	{
		data= new char[capacity];
	}
	
	private void expandCapacity( int minimumCapacity )
	{
		// use factor two by default, or the given number if higher
		int newCapacity= data.length * 2;
		
		if( newCapacity < minimumCapacity )
			newCapacity= minimumCapacity;
		
		char[] newData= new char[newCapacity];
		
		// TODO: Use System.arraycopy() when it is implemented.
		for( int i= 0; i < data.length; i++ )
			newData[i]= data[i];
			
		data= newData;
	}
	
	public StringBuilder append( String str )
	{
		// if required, expand array appropriately
		if( str.length() + position > data.length )
			expandCapacity( str.length() + position );
		
		str.getChars( 0, str.length(), data, position );
		position+= str.length();
		
		return this;
	}
	
	public StringBuilder append( char ch )
	{
		data[position++]= ch;
		return this;
	}

	public StringBuilder append( int i )
	{
		String str= Integer.toString( i );
		return append( str );
	}
	
	public StringBuilder append( long l )
	{
		String str= Long.toString( l );
		return append( str );
	}

	public StringBuilder append( Object obj )
	{
		return append( obj.toString() );
	}
	
	public StringBuilder append( boolean bool )
	{
		return append( bool ? "true" : "false" );
	}

	public int length()
	{
		return position;
	}
	
	public int capacity()
	{
		return data.length;
	}
	
	public String toString()
	{
		return new String( data );
	}
}