/**
 * fibonacci.java: Lösung der Finonacci-Folge mit Hilfe einer Schleife
 * Autor: Daniel Klein 2k-12-17
 * 
 * Änderungen:
 * - 2007-02-26: TextIO-Abhängigkeiten entfernt. Eingabe nun über Kommandozeilenargumente. 
 * 
 * HINWEIS: In der Aufgabenstellung zu diesem Praktikum wird für die
 * Fibonacci-Folge von 0 und 1 so definiert: fib(0)= 0 und fib(1)= 1
 * Hingegen wird in Robert Sedgewicks 'Algorithmen in C++' auf S.76
 * fib(0) UND fib(1) = 1 definiert. Um der Aufgabenstellung gerecht
 * zu werden ist sie hier jedoch als fib(0)= 0 implementiert.
 */
public class FibonacciIter
{
	// Berechnet die Fibonacci-Folge mit Hilfe einer Schleife
	// Im Falle eines Überlaufs wird -1 zurückgegeben
	public static long fib(int n)
	{
		long a= 0;
		long b= 1;
		long c= 0;
		
		// Überläufe abfangen. Als Fehlercode -1 zurückgeben
		if( n > 92 )
			return -1;
		
		// Fest definierte Werte 0 und 1 -> Siehe HINWEIS
		if( n == 0 )
			return 0;
		if( n == 1 )
			return 1;
		
		for( int i= 2; i <= n; i++ )
		{
			c= a+b;
			a= b;
			b= c;
		}
		
		return c;		
	}	
	
	public static void main (String[] args)
	{
		int input;
		long output;
		
		System.out.println( "Berechnung der Fibonacci-Folge mit Hilfe einer Schleife" );

		// Fehlendes Argument abfangen.
		if( args.length == 0 )
		{
			System.out.println( "Benutzung: java FibonacciIter <nicht negative Ganzzahl>" );
			return;
		}
		
		input= Integer.parseInt( args[0] );

		// Falsches Argument abfangen
		if( input < 0 )
		{
			System.out.println( "Benutzung: java FibonacciIter <nicht negative Ganzzahl>" );
			return;
		}

		long startTime= System.currentTimeMillis();
		
		output= fib( input );

		long endTime= System.currentTimeMillis();
		
		// Zahl ausserhalb des darstellbaren Bereichs?
		if( output == -1 )
		{
			System.out.println( "- ERROR: Die eingegebene Zahl loeste einen Ueberlauf aus und kann nicht berechnet werden!" );
			return;
		}		
		
		System.out.println( "Das Ergebnis ist: "+output );
		System.out.println( "Laufzeit: " + (endTime - startTime) );
	}
}
