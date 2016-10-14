package homeworkPP;

public class Plus extends ArithmeticSign {
	

	@Override
	public String eval() {
		String op1 = l.get(0).eval();
		String op2 = l.get(1).eval();
		
		return "[+ " + op1 + " " + op2 + "]";
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
