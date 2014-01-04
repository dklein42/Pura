public class Micra extends Car
{
	public void startMotor()
	{
		System.out.println( "-=Engaging motor=-" );
	}
	
	public void switchOffMotor()
	{
		System.out.println( "-=Motor is off=-" );
	}
	
	public void accelerate()
	{
		System.out.println( "Happy accelerating" );
	}
	
	public void brake()
	{
		System.out.println( "braking a bit" );
		
	}
	
	public void turnLeft()
	{
		System.out.println( "hard left" );
		
	}
	
	public void turnRight()
	{
		System.out.println( "hard right" );
		
	}

	public void honk()
	{
		System.out.println( "Beep! Beep!" );
	}
}