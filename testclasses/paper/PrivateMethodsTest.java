public class PrivateMethodsTest
{
   private static class A
   {
      public void doTest()
      {
         privTest();
      }

      private void privTest()
      {
         System.out.println("This is class A!");
      }
   }

   private static class B extends A
   {
      private void privTest()
      {
         System.out.println("This is class B!");
      }
   }

   public static void main(String[] args)
   {
      B b= new B();
      A a= b; // automatic upcast to A

      // Both methods call the same method of A, 
      // but internally they call a different privTest()
      b.doTest();
      a.doTest();
   }
}
