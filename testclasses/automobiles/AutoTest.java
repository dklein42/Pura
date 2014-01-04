public class AutoTest
{
	private static void drive( Drivable d )
	{
		System.out.println( "Test drive..." );
		
		d.startMotor();
		d.accelerate();
		d.turnLeft();
		d.turnRight();
		d.brake();
		d.switchOffMotor();
		
		System.out.println( "Test drive done.\n" );
	}
	
	public static void main( String[] args )
	{
		Person p= new Person( "Daniel" );
		Golf g= new Golf();
		Micra m= new Micra();
		Vehicle[] vehicles= new Vehicle[2];
		vehicles[0]= g;
		vehicles[1]= m;
		
		// Some more instanceof tests.
		System.out.println( "int array: " + (new int[0] instanceof int[]) );
		System.out.println( "Object array: " + (new Object[0] instanceof Object[]) );
		System.out.println( "Vehicle array: " + (vehicles instanceof Vehicle[]) );
		System.out.println( "int array array: " + (new int[0][0] instanceof int[][]) );
		
		if( vehicles[1] instanceof Drivable )
			System.out.println( "Vehicle is a Drivable" );
		else
			System.out.println( "Vehicle is not a Drivable" );

		if( vehicles[1] instanceof Car )
			System.out.println( "Vehicle is a Car" );
		else
			System.out.println( "Vehicle is not a Car" );

		if( vehicles[1] instanceof Micra )
			System.out.println( "Vehicle is a Micra" );
		else
			System.out.println( "Vehicle is not a Micra" );
		
		vehicles[1].enter( p );
		System.out.println( "Passenger of " + vehicles[1] + " is " + vehicles[1].getPassenger() );
		drive( vehicles[1] );
		vehicles[1].leave();
		
		vehicles[0].enter( p );
		System.out.println( "Passenger of " + vehicles[0] + " is " + vehicles[0].getPassenger() );
		drive( vehicles[0] );
		vehicles[0].leave();
		
		System.out.print( "Honking " + vehicles[0] + ": " );
		((Car)vehicles[0]).honk();
		
		System.out.print( "Honking " + vehicles[1] + ": " );
		((Car)vehicles[1]).honk();
	}
}