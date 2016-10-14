package homeworkPP;

import java.util.LinkedList;
import java.util.List;

public abstract class Program implements Elem {
	protected List <Elem> l = new LinkedList<Elem>();
	
	public Program(){}
	
	public void add(Elem e){
		l.add(e);
	}
}
