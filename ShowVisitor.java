package homeworkPP;

import java.util.LinkedList;
import java.util.List;

public class ShowVisitor implements Visitor {
	public List <String> st = new LinkedList<String>(); 
	
	public ShowVisitor(){
		st.add("");
	}
	
	private void add_last(String s){
		st.set(st.size() - 1, st.get(st.size()-1) + s + " ");
	}
	
	private void push_context(){
		st.add("");
	}
	private String pop_context(){
		return st.remove(st.size() - 1);
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
		add_last("+ " + s);
	}

	@Override
	public void initVisit(Or o) {
		push_context();
	}

	@Override
	public void endVisit(Or o) {
		String s = pop_context();
		add_last("X " + s);
	}

	@Override
	public void initVisit(Semicolon s) {
		push_context();
	}

	@Override
	public void endVisit(Semicolon s) {
		String str = pop_context();
		add_last("; " + str);
	}

	@Override
	public void initVisit(Equal e) {
		push_context();
	}

	@Override
	public void endVisit(Equal e) {
		String s = pop_context();
		add_last("= " + s);
	}

	@Override
	public void initVisit(Return r) {
		push_context();
	}
	
	@Override
	public void endVisit(Return r){
		String s = pop_context();
		add_last("return " + s);
	}
	
	

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
