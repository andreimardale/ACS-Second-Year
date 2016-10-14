package homeworkPP;

import java.util.LinkedList;
import java.util.List;
import java.util.NoSuchElementException;

public class ProgEvalVisitor implements Visitor {
	public List<String> st = new LinkedList<>();
	Integer result = new Integer(0);
	Context c = new Context();
	ExpEvalVisitor eev = new ExpEvalVisitor(c);
	private boolean blockContext = false;;
	
	private void add_last(String s){
		st.set(st.size() - 1, st.get(st.size() - 1) + s + " ");
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
		s = s.trim();
		add_last("[+ " + s + "]");
	}

	@Override
	public void initVisit(Or o) {
		push_context();
	}

	@Override
	public void endVisit(Or o) {
		String s = pop_context();
		add_last("[* " + s + "]");
	}

	@Override
	public void initVisit(Semicolon s) {
		push_context();
	}

	@Override
	public void endVisit(Semicolon s) {
		String str = pop_context();
		if (st.size() == 0)
			push_context();
		add_last(str);
	}

	@Override
	public void initVisit(Equal e) {
		push_context();
	}

	@Override
	public void endVisit(Equal e) {
		String s = pop_context();
		int i = 0;
		while (s.charAt(i) != ' ')
			i++;
		String appended = s.substring(i).trim();
		if (appended.charAt(0) != '['){
			appended = "[" + appended + "]";
		}
		Elem el = Utils.parse(appended);
		el.accept(eev);
		Integer res = new Integer(eev.st.remove(0).trim());
		if (blockContext == false)
			c.add(s.substring(0, i).trim(), res);
//		if (blockContext == true && c.isInContext(s.substring(0, i).trim())){
//			c.add(s.substring(0, i).trim(), res);
//		}
		add_last("["+s.substring(0, i).trim() + "= " + res + "]");
	}

	@Override
	public void initVisit(Return r) {
		push_context();
	}
	
	@Override
	public void endVisit(Return r){
		String s = pop_context();
		if (s.startsWith("[") == false){
			s = s.trim();
			result = c.valueOf(s);
		}
		else {
			Elem el = Utils.parse(s);
			ExpEvalVisitor eval = new ExpEvalVisitor(c);
			el.accept(eval);
			result = new Integer(eval.st.remove(0).trim());
			
		}
		
	}

	@Override
	public void initVisit(If i) {
		blockContext = true;
		push_context();
	}

	@Override
	public void endVisit(If i) {
		String s = pop_context();
		blockContext = false;
		String[] splitted = Main.splitList(s);
		
		String then = splitted[1];
		String els = splitted[2];
		
		String cnd = i.l.get(0).eval();
		String thn = i.l.get(1).eval();
		String el = i.l.get(2).eval();
		
		String toAdd = "[if " + cnd + " " + thn+ " " + el +"]";
		
		String condition = splitted[0].substring(3, splitted[0].length()-1);
		String[] conditionParts = Main.splitList(condition.trim());
		Integer res1;
		Elem el1 = Utils.parse(conditionParts[0]);
		el1.accept(eev);
		try{
			res1 = c.valueOf(eev.st.get(0).trim());
		}
		catch (NoSuchElementException e){
			res1 = new Integer (eev.st.get(0).trim());
		}
		finally {
			eev.st.remove(0);
		}
		
		
		Integer res2;
		Elem el2 = Utils.parse(conditionParts[1]);
		el2.accept(eev);
		try{
			res2 = c.valueOf(eev.st.get(0).trim());
		}
		catch (NoSuchElementException e){
			res2 = new Integer(eev.st.get(0).trim());
		}
		finally {
			eev.st.remove(0);
		}
		if (res1 <= res2){
			String[] sp = then.split("\\=");
			sp[0] = sp[0].substring(1);
			sp[1] = sp[1].trim();
			sp[1] = sp[1].substring(0, sp[1].length()-1);
			c.add(sp[0], Integer.valueOf(sp[1]));
		}
		else {
			String[] sp = els.split("\\=");
			sp[0] = sp[0].substring(1);
			sp[1] = sp[1].trim();
			sp[1] = sp[1].substring(0, sp[1].length()-1);
			c.add(sp[0], Integer.valueOf(sp[1]));
		}
		add_last(toAdd);
	}

	@Override
	public void initVisit(While w) {
		blockContext = true;
		push_context();
	}

	@Override
	public void endVisit(While w) {
		pop_context();
		blockContext = false;
		String op = w.l.get(1).eval();
		
		String[] operations = op.split("\n");
		
		String cond = w.l.get(0).eval();
		String toAdd = "[while " + cond + op + "]";
		cond = cond.substring(3, cond.length() - 1);
		String[] conditionParts = Main.splitList(cond.trim());
		Integer res1;
		
		Elem el1 = Utils.parse(conditionParts[0]);
		el1.accept(eev);
		try{
			res1 = c.valueOf(eev.st.get(0).trim());
		}
		catch (NoSuchElementException e){
			res1 = new Integer (eev.st.get(0).trim());
		}
		finally {
			eev.st.remove(0);
		}
		
		
		Integer res2;
		Elem el2 = Utils.parse(conditionParts[1]);
		el2.accept(eev);
		try{
			res2 = c.valueOf(eev.st.get(0).trim());
		}
		catch (NoSuchElementException e){
			res2 = new Integer(eev.st.get(0).trim());
		}
		finally {
			eev.st.remove(0);
		}
		while (res1 + 1 <= res2){
			Integer rez;
			for (int i = 0; i < operations.length; i++){
				String opr = operations[i].substring(3, operations[i].length() - 1);
				String[] oprParts = Main.splitList(opr.trim());
				Elem ele = Utils.parse(oprParts[1]);
				ele.accept(eev);
				
				try{
					rez = c.valueOf(eev.st.get(0).trim());
				}
				catch (NoSuchElementException e){
					rez = new Integer(eev.st.get(0).trim());
				}
				finally {
					eev.st.remove(0);
				}
				c.add(oprParts[0], rez);
			}
			res1 = res1 + 1;
		}
		add_last(toAdd);
	}

	@Override
	public void initVisit(EqualEqual ee) {
		push_context();
	}

	@Override
	public void endVisit(EqualEqual ee) {
		String s = pop_context();
		add_last("[== " + s + "]");
	}

	@Override
	public void initVisit(Less l) {
		push_context();
	}

	@Override
	public void endVisit(Less l) {
		String s = pop_context();
		add_last("[< " + s + "]");
	}

}
