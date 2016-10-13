package implementation;

import trie.TrieElement;

/**
 * Clasa pentru elemente specifice tipului de Trie ce ignora Case-ul.
 * Implementeaza interfata TrieElement. Contine un camp de tip string,
 *  adica cuvantul original.
 * @author andrei
 * 
 */
public class IgnCaseElement implements TrieElement
{
	private String word;
	
	/**
	 * Constructor fara parametrii.
	 */
	public IgnCaseElement() {
	}

	/**
	 * Constructor cu parametrii.
	 * @param word Cuvantul aferent unui TrieElement
	 */
	public IgnCaseElement(String word)
	{
		this.word = word;
	}
	
	/**
	 * Metoda ce mapeaza pentru un caracter pozitia in vectorul de Noduri al unui nod.
	 * Folosit pentru deplasarea in Trie, pentru a sti pe ce pozitie cobor in Trie.
	 * Maparea este urmatoarea:
	 * <p>pozitii de la 0 - 4 => caractere speciale; </p>
	 * <p>pozitii de la 5 - 15 => cifre; </p> 
	 * <p>pozitia 16 => _ </p>
	 * <p>pozitii 17 - final => literele mici </p>
	 * 
	 * @param c Caracterul pentru care se vrea pozitia in vector
	 * @return position Un intreg ce reprezinta pozitia
	 */
	public static int getPosition(char c)
	{	
		int position;
		if (c >= 48 && c <= 57)
			position = c - '0' + 5;
			
		else
		{
			switch (c)
			{
			case '!':	position = 0;
						break;
			case '(':	position = 1;
						break;
			case ')':	position = 2;
						break;
			case '-':	position = 3;
						break;
			case '?':	position = 4;
						break;
			case '_':	position = 16;
						break;
			default:	position = c - 'a' + 18;
						break;
			}
		}
		return position;
	}
	
	/**
	 * Getter pentru cuvantul din cadrul elementului
	 * @return word - String 
	 */
	
	public String getString ()
	{
		return this.word;
	}

	/**
	 * Functia ce trece toate literele mari in litere mici, pentru a ignora Case-ul
	 * @return vector de caractere
	 */
	@Override
	public char[] toCharArray() 
	{
		return word.toLowerCase().toCharArray();
	}
	
	
	public String toString()
	{
		return this.word;
	}

}
