package homeworkPP;

import junit.framework.TestCase;

public class PublicTests extends TestCase {

	private Context context;
	private String e1, e2, e3;
	private Integer eRes1, eRes2, eRes3;
	
	private String cp1, cp2, cp3;
	private Boolean cpRes1, cpRes2, cpRes3;
	
	private String prog1, prog2;
	private Integer progRes1, progRes2;
	
	
	/* This method is called before any tests are run (sort of a context initialization method) */
	@Override	
	protected void setUp() {
		
		/* Set the actual context of the evaluation */
		context = new Context();
		context.add("x", 5);
		context.add("y", 4);
		context.add("z", 10);
		
		
		/* ---- Expression evaluation ---- */
		e1 = "[+ [* x 2] [+ 3 [* 2 1]]]";
		eRes1 = 15;
		
		e2 = "[* [* 3 x] [+ 1 [+ x 1]]]";
		eRes2 = 105;
		
		e3 = "[+ [+ [* z z] [* 2 z]] 1]";
		eRes3 = 121;
		
		
		/* ---- Program correctness ---- */
		cp1 = "[; [= y [+ 1 x]] [return y]]";	
		cpRes1 = false;		// x is out of scope
		
		cp2 = "[; [; [; [= x 2] [= y 3]] [= z [+ x y]]] [return z]]";
		cpRes2 = true; 		// the program is correct
		
		cp3 = "[; [= y 3] [= x 2]]";
		cpRes3 = false; 	// no return 
		
		
		/* ---- Program evaluation ---- */
		/* Initialization for the correct programs that only need to be evaluated */
		prog1 = "[; [= x 10] [; [= y 5] [; [if [== 15 [+ x y]] [= x [* x y]] [= x [+ x y]]] [return x]]]]";
		progRes1 = 50;
		
		prog2 = "[; [= i 0] [; [= x 10] [; [if [== i 0] [while [< x 15] [= x [+ x 1]]] [= i [+ i 1]]] [return [+ x x]]]]]";
		progRes2 = 30;		
		
	}
	
	/* ---- Test expression evaluation ---- */
	public void testExpEvaluation1(){
		Integer computedResult = Main.evalExpression(e1, context);
		assertEquals(Integer.valueOf(computedResult), Integer.valueOf(eRes1));
	}
	
	public void testExpEvaluation2(){
		Integer computedResult = Main.evalExpression(e2, context);
		assertEquals(Integer.valueOf(computedResult), Integer.valueOf(eRes2));
	}
	
	public void testExpEvaluation3(){
		Integer computedResult = Main.evalExpression(e3, context);
		assertEquals(Integer.valueOf(computedResult), Integer.valueOf(eRes3));
	}
	
	
	/* ---- Test program correctness ---- */
	// Here, we're able to use: Main.checkCorrectness(String program) -> Boolean
	// Check for variable out of scope & missing return.
	public void testProgCorrectness1(){	
		Boolean computedResult = Main.checkCorrectness(cp1);
		assertEquals(computedResult, cpRes1);
	}
	
	public void testProgCorrectness2(){
		Boolean computedResult = Main.checkCorrectness(cp2);
		//System.out.println(Main.evalProgram(cp2));
		assertEquals(computedResult, cpRes2);
	}
	
	public void testProgCorrectness3(){
		Boolean computedResult = Main.checkCorrectness(cp3);
		//System.out.println(Main.evalProgram(cp3));
		assertEquals(computedResult, cpRes3);
	}
	
	
	/* ---- Test program evaluation ---- */
	// Here, we're able to use: Main.evalProgram(String program) -> Integer
	// Evaluate the program (it will be a correct program).
	public void testProgEvaluation1(){
		Integer computedResult = Main.evalProgram(prog1);
		assertEquals(Integer.valueOf(computedResult), progRes1);
	}
	
	public void testProgEvaluation2(){
		Integer computedResult = Main.evalProgram(prog2);
		assertEquals(Integer.valueOf(computedResult), progRes2);
	}		
}