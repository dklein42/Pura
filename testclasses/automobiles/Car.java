public abstract class Car implements Vehicle
{
	private Person passenger;
	
	public boolean enter( Person p )
	{
		if( passenger != null )
			return false;
			
		passenger= p;
		return true;
	}
	
	public Person leave()
	{
		//if( passenger == null )
		//	throw new NoPassengerException();
			
		Person p= passenger;
		passenger= null;
		return p;
	}
	
	public Person getPassenger()
	{
		return passenger;
	}
	
	public abstract void startMotor();
	public abstract void switchOffMotor();
	
	public abstract void accelerate();
	public abstract void brake();
	
	public abstract void turnLeft();
	public abstract void turnRight();
	
	public void honk()
	{
		System.out.println( "Hooooonk!" );
	}	
}