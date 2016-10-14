package homeworkPP;

import java.util.LinkedList;
import java.util.List;
import java.util.NoSuchElementException;

public class ExpEvalVisitor implements Visitor {
	public List<String> st = new LinkedList<String>();
	private Context c;
	
	public ExpEvalVisitor(Context c) {
		st.add("");
		this.c = c;
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
		if (st.size() == 0)
			push_context();
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
		Integer sum = getResult(splitted, '+');
		if (st.size() == 0)
			push_context();
		add_last(sum.toString());
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
		Integer prod = getResult(splitted, '*');
		if (st.size() == 0)
			push_context();
		add_last(prod.toString()); 
	}
	
	
	private Integer getResult(String[] splitted, char op){
		Integer result = 0, op1 = 0, op2 = 0;
		if (isInteger(splitted[0]) == true && isInteger(splitted[1]) == true){
			op1 = Integer.valueOf(splitted[0]);
			op2 = Integer.valueOf(splitted[1]);
		}
		if (isInteger(splitted[0]) == false && isInteger(splitted[1]) == true){
			try {
				op1 = c.valueOf(splitted[0]);
				op2 = Integer.valueOf(splitted[1]);
			}catch (NoSuchElementException e){
				e.printStackTrace();
			}
		}
		if (isInteger(splitted[1]) == false && isInteger(splitted[0]) == true){
			try {
				op2 = c.valueOf(splitted[1]);
				op1 = Integer.valueOf(splitted[0]);
			}catch (NoSuchElementException e){
				e.printStackTrace();
			}
		}
		if (isInteger(splitted[0]) == false && isInteger(splitted[1]) == false) {
			try {
				op2 = c.valueOf(splitted[1]);
				op1 = c.valueOf(splitted[0]);
			}catch (NoSuchElementException e){
				e.printStackTrace();
			}
		
		}
		if (op == '+')
			result = op1 + op2;
		if (op == '*')
			result = op1 * op2;
		return result;
		
		
	}
	
	private static boolean isInteger(String s){
		try {
			Integer.parseInt(s);
		} catch (NumberFormatException e){
			return false;
		} catch (NullPointerException e){
			return false;
		}
		return true;
	}

	@Override
	public void initVisit(Semicolon s) { }

	@Override
	public void endVisit(Semicolon s) { }

	@Override
	public void initVisit(Equal e) { }

	@Override
	public void endVisit(Equal e) { }

	@Override
	public void initVisit(Return r) { }
	
	public void endVisit(Return r) {}

	@Override
	public void initVisit(If i) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void endVisit(If i) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void initVisit(While w) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void endVisit(While w) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void initVisit(EqualEqual ee) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void endVisit(EqualEqual ee) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void initVisit(Less l) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void endVisit(Less l) {
		// TODO Auto-generated method stub
	}
}
