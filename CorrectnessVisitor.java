package homeworkPP;

import java.util.LinkedList;
import java.util.List;
import java.util.NoSuchElementException;

public class CorrectnessVisitor implements Visitor {
	
	public List<String> st = new LinkedList<String>();
	private Context c = new Context();
	private boolean result = true;
	private boolean has_return = false;
	private int type = -1;
	
	public CorrectnessVisitor() {
		st.add("");
	}
	
	public boolean hasResult(){
		if (result == true && has_return == true)
			return true;
		else 
			return false;
	}
	
	public void add_last(String s){
		st.set(st.size()-1, st.get(st.size()-1) + s + " ");
	}
	
	private void push_context(){
		st.add("");
	}
	
	private String pop_context (){
		return st.remove(st.size()-1);
	}
	
	@Override
	public void visit(Value v) {
		add_last(v.eval());
	}

	@Override
	public void initVisit(Plus p) {
		push_context();
	}

	@Override
	public void endVisit(Plus p) {
		String s = pop_context();
		s = s.trim();
		String[] splitted = s.split("\\s+");
		boolean ok = inContext(splitted, 0);
		if (ok == false){
			result = false;
		}
		add_last(splitted[0] + "+" + splitted[1]);
	}

	@Override
	public void initVisit(Or o) {
		push_context();
	}

	@Override
	public void endVisit(Or o) {
		String s = pop_context();
		s = s.trim();
		String[] splitted = s.split("\\s+");
		boolean ok = inContext(splitted, 0);
		if (ok == false){
			result = false;
		}
		add_last(splitted[0]+"*"+splitted[1]);
	}

	@Override
	public void initVisit(Semicolon s) {
		push_context();
	}

	@Override
	public void endVisit(Semicolon s) {
		
	}

	@Override
	public void initVisit(Equal e) {
		push_context();
	}

	@Override
	public void endVisit(Equal e) {
		String s = pop_context();
		boolean ok = true;
		s = s.trim();
		String[] splitted = s.split("[\\s+, \\+, \\*]");
		String key = splitted[0];
		ok = inContext(splitted, 1);
		if (ok == false){
			result = false;
		}
		else {
			if (c.isInContext(key) == false)
				c.add(key, type); // adaug o variabila in contextul curent
		}
		add_last(splitted[0] + "=" + splitted[1]);
	}

	@Override
	public void initVisit(Return r) {
		push_context();
	}
	
	@Override
	public void endVisit(Return r){
		has_return = true;
		String s = pop_context();
		add_last(" return " + s);
	}


	@Override
	public void initVisit(If i) {
		type = -2;
		push_context();
	}

	@Override
	public void endVisit(If i) {
		type = -1;
		String s = pop_context();
		c.remove(-2);
		add_last("if" + s);

	}

	@Override
	public void initVisit(While w) {
		type = -2;
		push_context();
	}

	@Override
	public void endVisit(While w) {
		type = -1;
		String s = pop_context();
		c.remove(-2);
		add_last("while " + s);
	}

	@Override
	public void initVisit(EqualEqual ee) {
		push_context();
	}

	@Override
	public void endVisit(EqualEqual ee) {
		String s = pop_context();
		s = s.trim();
		String[] splitted = s.split("\\s+");
		boolean ok = inContext(splitted, 0);
		if (ok == false){
			result = false;
		}
		add_last(splitted[0] + "==" + splitted[1]);
	}

	@Override
	public void initVisit(Less l) {
		push_context();
	}

	@Override
	public void endVisit(Less l) {
		String s = pop_context();
		s = s.trim();
		String[] splitted = s.split("\\s+");
		boolean ok = inContext(splitted, 0);
		if (ok == false){
			result = false;
		}
		add_last(splitted[0] + "<" + splitted[1]);		
	}
	/* metoda pentru a verifica daca un element e in context
	 * returneaza false daca nu e in context*/
	private boolean inContext(String[] splitted, int start){
		boolean ok = true;
		for (int i = start; i < splitted.length; i++){
			if (isInteger(splitted[i]) == false){
				try {
					c.valueOf(splitted[i]);
				}
				catch (NoSuchElementException nee){
					ok = false;
//					nee.printStackTrace();
				}
			}
		}
		return ok;
	}
	
	static boolean isInteger(String s){
		try {
			Integer.parseInt(s);
		} catch (NumberFormatException e){
			return false;
		} catch (NullPointerException e){
			return false;
		}
		return true;
	}
}


