package implementation;

import trie.TrieElement;

/**
 * Clasa pentru elemente specifice tipului de Trie ce ignora caractere speciale.
 * Implementeaza interfata TrieElement. Contine un camp de tip string,
 * adica cuvantul original.
 * @author andrei
 *
 */
public class IgnCharElement implements TrieElement
{
	private String element;
	
	/**
	 * Constructor cu parametrii pentru {@link IgnCharElement}
	 * @param element Stringul din cadrul elementului
	 */
	public IgnCharElement(String element) 
	{
		this.element = element;
	}
	
	/**
	 * Constructor fara parametrii
	 */
	public IgnCharElement() {}
	
	/**
	 * Metoda ce mapeaza pentru un caracter pozitia in vectorul de Noduri al unui nod.
	 * Folosit pentru deplasarea in Trie, pentru a sti pe ce pozitie cobor in Trie.
	 * Maparea este urmatoarea:
	 * <p> pozitia 0 => '!' </p>
	 * <p> pozitia 1 - 10 => cifre </p>
	 * <p> pozitia 11 => '?' </p>
	 * <p> pozitia 12 - .. => litere mari </p>
	 * <p> ultimele poztii => litere mari </p>
	 * @param c Caracterul pentru care se vrea pozitia in vector
	 * @return position - Un intreg ce reprezinta pozitia in vector 
	 */
	public static int getPosition(char c)
	{
		int position;
		if (c == 33)
		{
			position = 0;
			return position;
		}
		if (c == 63)
		{
			position = 11;
			return position;
		}
		if (c >= 48 && c <= 57) 
			position = c - '0' + 1;
		else
			if (c >= 65 && c <= 90)
				position = c - 'A' + 12;
			else
				position = c - 71 + 12;
		return position;
		
	}
	
	/**
	 * Getter pentru cuvantul din cadrul Elementului
	 * @return cuvant
	 */
	public String getElement() 
	{
		return element;
	}
	
	public String toString()
	{
		return this.element;
	}

	/**
	 * Implementarea metodei toCharArray. Scoate caracterele indicate din cuvant
	 * @return Intoarce un vector de caractere
	 */
	@Override
	public char[] toCharArray() 
	{
		String result = element.replaceAll("[-()_]","");
		char[] rez = result.toCharArray();
		return rez;
	}

}
