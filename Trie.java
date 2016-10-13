package implementation;

import java.util.ArrayList;
import java.util.Iterator;

import trie.AbstractTrie;
import trie.TrieElement;

/**
 * Clasa principala pentru implementarea Trie-ului. Contine un element TrieNode,
 * adica radacina Trie-ului. Implementeaza AbstractTrie
 * 
 * @author andrei
 *
 */
public class Trie implements AbstractTrie 
{
	private TrieNode root;
	
	/**
	 * Constructor fara parametrii. Instantiaza radacina.
	 */
	public Trie ()
	{
		root = new TrieNode();
	}

	/**
	 * Adauga un element in Trie. Verifica de ce tip este elementul, ii genereaza
	 * char array-ul specific. Pastreaza si cuvantul initial in "realWord".
	 * Apeleaza metoda add definita in clasa TrieNode. Adauga elementul incepand de
	 * radacina.
	 */
	@Override
	public void add(TrieElement element) 
	{	
		char[] word = null;
		String realWord = null;
		int type = 0;
		
		if (element instanceof IgnCaseElement)
		{
			word = ((IgnCaseElement)element).toCharArray();
			realWord = ((IgnCaseElement)element).getString();
			type = 0;
		}
		if (element instanceof IgnCharElement)
		{
			word = ((IgnCharElement)element).toCharArray();
			realWord = ((IgnCharElement)element).getElement();
			type = 1;
		}
		root.addElement(word, realWord, type);
}

	/**
	 * Pe baza unui element, cauta in Trie nodul unde se gaseste acel element.
	 * @param currentNode Nodul de unde se incepe cautarea, e mereu root.
	 * @param element elementul cautat in Trie
	 * @return Un nod
	 */
	public TrieNode getNode (TrieNode currentNode, TrieElement element)
	{
		char[] word = null;
		currentNode = root;
		TrieNode child = null ;
		int position;
		
		if (element instanceof IgnCaseElement)
			word = ((IgnCaseElement)element).toCharArray();

		if (element instanceof IgnCharElement)
			word = ((IgnCharElement)element).toCharArray();
		
		
		for (char c : word)
		{
			if (element instanceof IgnCaseElement)
				position = IgnCaseElement.getPosition(c);
			else
				position = IgnCharElement.getPosition(c);
			
			TrieNode[] children = currentNode.getChildren();
			child = children[position];
			if (child == null)
				return null;
			if (child.isLeaf() == true && c != word[word.length - 1])
			{
				return null;
			}
			currentNode = child;			
		}
		return child;
		
	}

	/**
	 * Metoda ce identifica numarul de aparitii al unui element in Trie.
	 * Cauta folosind metoda getNode nodul respectiv si intoarce rezultatul
	 * metodei getCount() specifica acelui nod.
	 */
	@Override
	public int count(TrieElement element) 
	{
		TrieNode child = getNode(root, element);
		if (child == null)
			return 0;
		if (child.getCount() < 0)
			return 0;
		return child.getCount();
	}

	/**
	 * Sterge un element din arbore. Cauta nodul unde se gaseste elementul de sters.
	 * Daca nu exista, nu iese din metoda fara sa faca nimic.
	 * Decrementeaza numarul de aparitii al acelui element in Trie.
	 * Daca este frunza si a ajuns numaratoarea la 0, incep sa elim nodul din Trie.
	 * Elimin noduri pana ajung la un nod ce are si cai catre alte cuvinte, altele 
	 * decat cel de eliminat. Practic, pana ajung la un prefix comun pentru mai multe
	 * cuvinte.
	 */
	@Override
	public void remove(TrieElement element) 
	{
		TrieNode foundNode = getNode(root, element);
		if (foundNode == null)
			return;
		TrieNode upperNode = null;
		foundNode.decrCount();
		if (foundNode.isLeaf() && foundNode.getCount() == 0)
		{
			upperNode = foundNode.getParent();
			while (upperNode.getNoChildren() == 1 && upperNode.isWord() == false)
			{
				foundNode = null;
				foundNode = upperNode;
				upperNode = foundNode.getParent();
			}
			
			foundNode.setWord(false);
			foundNode.setElement(null);
			foundNode = null;
		}
		else	
			if ((foundNode.isLeaf() == false) && (foundNode.getCount() == 0))
			{
				foundNode.setWord(false);
				foundNode.setElement(null);
			}
	}

	/**
	 * Caut nodul de unde se gaseste prefixul de cautat. Caz special: in cazul
	 * al doilea de Trie, daca caut dupa un prefix ce a fost "curatat" de tot,
	 * pana nu mai are niciun caracter, afisez tot Trie-ul.
	 * Daca nu exista prefix-ul in Trie, intorc un vector de zero elemente, 
	 * pentru a afisa o linie goala. Altfel, apelez metoda findWords() definita
	 * in clasa TrieNode. Aceasta imi va aduce toate cuvintele de la Nodul curent,
	 * in jos. Folosesc un iterator pentru a translata din ArrayList in TrieElement[]
	 */
	@Override
	public TrieElement[] getSortedElements(TrieElement prefix) 
	{
		TrieNode startingNode = getNode(root, prefix);
		
		if (prefix instanceof IgnCharElement)
		{
			char [] word = ((IgnCharElement)prefix).toCharArray();
			if (word.length == 0)
				startingNode = root;
					
		}
		
		TrieElement[] retrievedWords;
		
		if (startingNode == null)
		{
			retrievedWords = new TrieElement[0];
			return retrievedWords;
		}
		ArrayList<TrieElement> list = startingNode.findWords();
		
		retrievedWords = new TrieElement[list.size()];
		
		Iterator<TrieElement> li = list.iterator();
		int i = 0;
		
		while (li.hasNext()) {
			retrievedWords[i++] = (TrieElement) li.next();
		}
		return retrievedWords;
	}

}
