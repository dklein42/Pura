public class Test2 {
	public static void main(String[] args) {
		B b= new B();
		b.foo("1", "1");
	}
	
	public void foo( Object obj, String str ) {
		System.out.println("os");
	}
}

class B extends Test2 {
	public void foo( Object o1, Object o2 ) {
		System.out.println("oo");
	}
}