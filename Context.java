package homeworkPP;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NoSuchElementException;

public class Context {
	private Map<String, Integer> hm = new HashMap<>();
  
    public void add (String v, Integer i){
    	hm.put(v, i);
    }
    //removes all elements from the context with value = value
    public void remove (Integer value){
    	for(Iterator<Entry<String, Integer>> it = hm.entrySet().iterator(); it.hasNext(); ) {
    	      Entry<String, Integer> entry = it.next();
    	      if(entry.getValue() == value) {
    	        it.remove();
    	      }
    	    }
    }
    
    // Treat undefined variable problem using exceptions
    public Integer valueOf(String v) {
    	Integer i;
    	i = hm.get(v);
    	if (i == null){
    		throw new NoSuchElementException();
    	}
    	return i;
    }
    
    public boolean isInContext (String s){
    	Integer i = hm.get(s);
    	if (i == null)
    		return false;
    	return true;
    }
}