package homeworkPP;

public class If extends Program {

	@Override
	public String eval() {
		String op1 = l.get(0).eval();
		String op2 = l.get(1).eval();
		String op3 = l.get(2).eval();
		return "if " + op1 + "\n  " + op2 + "\nelse\n  " + op3 + "\n " ;
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
