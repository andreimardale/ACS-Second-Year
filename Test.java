package implementation;

import test.Command;
import test.TestReader;
import test.TestWriter;
import trie.TrieElement;
/**
 * Clasa ce testeaza functionalitatea
 * @author andrei
 *
 */
public class Test {
	/**
	 * Modul de rularea in care ignora caractere speciale.
	 * @param secondTrie Trie-ul pe care se lucreaza
	 * @param secondCom Comenzile aplicate
	 * @param words Cuvintele inserate
	 * @param writer Writer-ul pentru fisier
	 */
	public static void playIgnChar(Trie secondTrie, Command[] secondCom, String[] words, TestWriter writer)
	{
		IgnCharElement[] igCharElts = new IgnCharElement[words.length];
		int i = 0;
		
		for (String string : words)
		{
			igCharElts[i] = new IgnCharElement(string);
			i++;
		}
		for (IgnCharElement element : igCharElts)
		{
			secondTrie.add(element);
		}
		for (i = 0; i < secondCom.length; i++)
		{
			int type = secondCom[i].getType();
			String word = secondCom[i].getWord();
			
			if (type == 0)
			{
				secondTrie.add(new IgnCharElement(word));
			}
			if (type == 2)
			{
				writer.printCount(secondTrie.count(new IgnCharElement(word)));
			}
			if (type == 1)
			{
				secondTrie.remove(new IgnCharElement(word));
			}
			if (type == 3)
			{
				TrieElement[] result = secondTrie.getSortedElements(new IgnCharElement(word));

				writer.printSortedWords(result);
			}
		}
		writer.close();		
	}
	
	public static void playIgnCase(Trie firstTrie, Command[] firstCom, String[] words, TestWriter writer)
	{
		IgnCaseElement[] elements = new IgnCaseElement[words.length];
		int i = 0;
		
		for (String string : words)
		{
			elements[i] = new IgnCaseElement(string);
			i++;
		}
		
		for (IgnCaseElement element:elements)
		{
			firstTrie.add(element);
		}
		for (i = 0; i < firstCom.length; i++)
		{
			int type = firstCom[i].getType();
			String word = firstCom[i].getWord();
			
			if (type == 0)
			{
				firstTrie.add(new IgnCaseElement(word));
			}
			if (type == 2)
			{
				writer.printCount(firstTrie.count(new IgnCaseElement(word)));
			}
			if (type == 1)
			{
				firstTrie.remove(new IgnCaseElement(word));
			}
			if (type == 3)
			{
				TrieElement[] result = firstTrie.getSortedElements(new IgnCaseElement(word));
				
				writer.printSortedWords(result);
			}	
		}
	}

	public static void main(String[] args) 
	{
		TestReader reader = new TestReader("trie.in");
		TestWriter writer = new TestWriter("trie.out");
		
		String[] words = reader.getWords();
		
		Command[] firstCom = reader.getFirstCommands();
		Command[] secondCom = reader.getSecondCommands();
		
		
		
		Trie firstTrie = new Trie();
		Trie secondTrie = new Trie();
		
		playIgnCase(firstTrie, firstCom, words, writer);
		playIgnChar(secondTrie, secondCom, words, writer);

	}
}
