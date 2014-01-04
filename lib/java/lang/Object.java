package java.lang;

public class Object
{
	
	public native int hashCode();
	public native String getClassName();
	
	public String toString()
	{
		// TODO: Implement Integer.toHexString() and use it here appropriately instead of Integer.toString()
		return getClassName() + "@" + Integer.toString( hashCode() );
	}
	
	public boolean equals( Object obj )
	{
		return this == obj;
	}
}