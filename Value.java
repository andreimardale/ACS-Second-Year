package homeworkPP;

public class Value implements Elem {
	private String s;
	
	public Value(String s) {
		this.s = s;
	}
	
	@Override
	public String eval() {
		return s;
	}

	@Override
	public void accept(Visitor v) {
		v.visit(this);
	}

}
