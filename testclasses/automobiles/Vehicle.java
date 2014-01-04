public interface Vehicle extends Drivable
{
	public boolean enter( Person p );
	public Person leave();
	public Person getPassenger();
}