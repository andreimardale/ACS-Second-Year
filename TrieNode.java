package implementation;

import java.util.ArrayList;

import trie.TrieElement;
/**
 * Clasa ce implementeaza tipul unui Nod. Contine variabile:
 * <p> parent => nodul parinte, folosit pentru remove </p>
 * <p> children => vector de noduri copil </p>
 * <p> isWord => flag 1 - daca am cuvant in acel nod, 0 altfel </p>
 * <p> character => caracterul salvat intr-un anume nod </p>
 * <p> count => numarul de aparitii al unui cuvant intr-un nod </p>
 * <p> element => TrieElementul efectiv </p>
 * Clasa contine getteri si setteri pentru majoritatea campurilor, 
 * pentru a respecta principiul incapsularii.
 * @author andrei
 */
public class TrieNode
{
	private TrieNode parent;
	private TrieNode[] children;
	private boolean isWord;
	private char character;
	private int count = 0;
	private TrieElement element;
	
	/**
	 * Constructor fara parametrii. Initializeaza un vector de copii, de maxim 68 elemente
	 * Count e resetat la 0, isWord e 0.
	 */
	public TrieNode()
	{
		children = new TrieNode[68];
		isWord = false;
		count = 0;
	}
	
	/**
	 * Constructor cu parametrii. Seteaza caracterul la "|", pt a avea un cod ascii cat mai mare
	 * Idee folositoarea pentru comparatii.
	 * @param character
	 * @param type
	 */
	public TrieNode(char character, int type)
	{
		this();
		this.character = character;
		if (type == 0)
			this.element = new IgnCaseElement("|");
		if (type == 1)
			this.element = new IgnCharElement("|"); //caracter cu ascii mare
		
	}
	
	public TrieNode[] getChildren() 
	{
		return children;
	}
	
	public void setElement(TrieElement element)
	{
		this.element = element;
	}
	
	public TrieElement getElement()
	{
		return this.element;
	}
	
	public int getNoChildren()
	{
		return children.length;
	}
	
	public boolean isWord() {
		return isWord;
	}
	
	public void setWord(boolean isWord) {
		this.isWord = isWord;
	}
	
	public boolean isLeaf()
	{
		boolean result = true;
		for (TrieNode child:children)
		{
			if (child != null)
			{
				result = false;
				break;
			}
		}
		return result;
	}


	public char getCharacter() {
		return character;
	}


	public int getCount() {
		return count;
	}
	
	public void decrCount(){
		this.count = this.count - 1;
	}
	
	public TrieNode getParent() 
	{
		return parent;
	}

/**
 * Metoda ce adauga un element in Trie. In primul rand, determina pozitia in vector.
 * Daca nu exista nod acolo, creeaza nodul. Daca am mai mult de un caracter ramas
 * neadaugat, repeta recursiv toti pasii pana aici, pentru cuvantul ce incepe
 * la pozita 1.
 * Cand ajung la ultima litera, setez flagul de isWord, si compar cu cuvantul ce se
 * gaseste deja in acel nod, pentru a determina care e mai mic lexicografic.
 * Astfel, mereu salvez cel mai mic cuvant.
 * @param word Array obtinut in urma metodei toCharArray
 * @param realWord Cuvantul initial
 * @param type tipul elementului de adaugat
 */
	public void addElement(char [] word, String realWord, int type)
	{
		int position;
		String insertWord = new String (word);
		
		if (type == 0)
			{
				position = IgnCaseElement.getPosition(insertWord.charAt(0));
			}
		else
			position = IgnCharElement.getPosition(insertWord.charAt(0));
		
		if (children[position] == null)
		{
			children[position] = new TrieNode(insertWord.charAt(0), type);
			children[position].parent = this;
		}
	
		if (insertWord.length() > 1)
		{	
			children[position].addElement((insertWord.substring(1)).toCharArray(), realWord, type);
		}
		else
		{
			children[position].isWord = true;
			
			if (type == 0)
			{
				String oldWord = "|";
				if (children[position].element != null)
				{
					oldWord = ((IgnCaseElement)(children[position].element)).getString();
				}
				
				if (realWord.compareTo(oldWord) < 0)
				{
					oldWord = realWord;
				}
				children[position].element = new IgnCaseElement(oldWord);

			}
			else
			{
				String oldWord = "|";
				if (children[position].element != null)
					oldWord = ((IgnCharElement)(children[position].element)).getElement();

				if ( realWord.compareTo(oldWord) < 0)
				{
					oldWord = realWord;
				}
				children[position].element = new IgnCharElement(oldWord);
			}
			children[position].count++;
		}
	}
	
	
	public ArrayList<TrieElement> findWords()
	{
		
		ArrayList<TrieElement> list = new ArrayList<TrieElement>();
		if (isWord == true)
		{
			list.add(element);
		}
		if (! isLeaf())
		{
			for (int i = 0; i < children.length; i++)
			{
				if (children[i] != null)
				{
					list.addAll(children[i].findWords());
				}
			}
		}
		return list;
		
	}

}
