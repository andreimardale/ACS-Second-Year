package homeworkPP;

public class Return extends Program {
	
	@Override
	public void accept(Visitor v) {
		v.initVisit(this);
		for (Elem e : l){
			e.accept(v);
		}
		v.endVisit(this);
	}

	@Override
	public String eval() {
		String op = l.get(0).eval();
		return "return " + op;
	}

}
