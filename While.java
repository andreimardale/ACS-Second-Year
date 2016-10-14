package homeworkPP;

public class While extends Program {

	@Override
	public String eval() {
		String op1 = l.get(0).eval();
		String op2 = l.get(1).eval();
		return "while" + op1 + "\n{\n\t" + op2 + "\n}"; 
	}

	@Override
	public void accept(Visitor v) {
		v.initVisit(this);
		for (Elem e : l){
			e.accept(v);
		}
		v.endVisit(this);
	}

}
