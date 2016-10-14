package homeworkPP;

public interface Visitor {
	
	public void visit (Value v);
	
	public void initVisit(Plus p);
	public void endVisit(Plus p);
	
	public void initVisit(Or o);
	public void endVisit (Or o);
	
	public void initVisit (Semicolon s);
	public void endVisit  (Semicolon s);
	
	public void initVisit (Equal e);
	public void endVisit  (Equal e);
	
	public void initVisit (Return r);
	public void endVisit (Return r);
	
	public void initVisit(If i);
	public void endVisit (If i);
	
	public void initVisit(While w);
	public void endVisit (While w);
	
	public void initVisit(EqualEqual ee);
	public void endVisit (EqualEqual ee);
	
	public void initVisit(Less l);
	public void endVisit (Less l);
}
