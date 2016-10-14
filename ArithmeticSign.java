package homeworkPP;

import java.util.LinkedList;
import java.util.List;

public abstract class ArithmeticSign implements Elem {
	protected List<Elem> l = new LinkedList<Elem>();
	
	public ArithmeticSign(){ }
		
	public void add (Elem e){
		l.add(e);
	}
}
