package homeworkPP;

import java.util.LinkedList;
import java.util.List;

public class Main {

	/**
	 * IMPORTANT! Your solution will have to implement this method.
	 * @param exp - a string, which represents an expression (that
	 * follows the specification in the homework);
	 * @param c - the context (a one-to-one association between variables
	 * and values);
	 * @return - the result of the evaluation of the expression;
	 */
	public static Integer evalExpression(String exp, Context c){
		Elem e = Utils.parse(exp);
		ExpEvalVisitor eev = new ExpEvalVisitor(c);
		e.accept(eev);

		return new Integer(eev.st.get(0).trim());
	}

	/**
	 * IMPORTANT! Your solution will have to implement this method.
	 * @param program - a string, which represents a program (that
	 * follows the specification in the homework);
	 * @return - the result of the evaluation of the expression;
	 */
	public static Integer evalProgram(String program) {
		Elem e = Utils.parse(program);
		ProgEvalVisitor pev = new ProgEvalVisitor();
		e.accept(pev);
		return pev.result;
	}

	/**
	 * IMPORTANT! Your solution will have to implement this method.
	 * @param program - a string, which represents a program (that
	 * follows the specification in the homework);
	 * @return - whether the given program follow the syntax rules
	 * specified in the homework (always return a value and always
	 * use variables that are "in scope");
	 */

	public static Boolean checkCorrectness(String program) {
		Elem e = Utils.parse(program);
		CorrectnessVisitor cv = new CorrectnessVisitor();
		e.accept(cv);
		
		return cv.hasResult();
	}


	/**
	 *
	 * @param s - a string, that contains a list of programs, each
	 * program starting with a '[' and ending with a matching ']'.
	 * Programs are separated by the whitespace caracter;
	 * @return - array of strings, each element in the array representing
	 * a program;
	 * Example: "[* [+ 1 2] 3] [* 4 5]" -> "[* [+ 1 2] 3]" & "[* 4 5]";
	 */
	 public static String[] splitList(String s){
		List<String> l = new LinkedList<String>();
        int inside = 0;
        int start = 0, stop = 0;
        for (int i=0; i<s.length(); i++){
                if (s.charAt(i) == '['){
                    inside++;
                    stop++;
                    continue;
                }
                if (s.charAt(i) == ']'){
                    inside--;
                    stop++;
                    continue;
                }
                if (s.charAt(i) == ' ' && inside == 0){
                    l.add(s.substring(start,stop));
                    start = i+1; //starting after whitespace
                    stop = start;

                    continue;
                }
                stop++; //no special case encountered
        }
        if (stop > start) {
            l.add(s.substring(start, stop));
        }

        return l.toArray(new String[l.size()]);

	 }

	public static void main(String[] args) {
		/* Suggestion: use it for testing */
		/*[; [= x [+ [* 100 [+ 7 8]] 2]] [; [= y [+ 2 [* x 3]]] [return [+ 100 [+ [* x y] [* x 0]]]]]]*/
//		String s = "[; [= x 1] [; [= i 0] [; [while [< i 10] [; [= x [* x 2]] [= i [+ i 1]]]] [return x]]]] ";
//	(BUG)	String s = "[; [= x 10] [; [if [< x 20] [if [== x 10] [= x 1] [= x 2]] [= x 3]] [return x]]]";
		String s = "[; [= x 10] [; [if [< x 5] [= x 2] [if [< x 20] [= x 3] [= x 4]]] [return x]]]";
		//		Context c = new Context();
//		System.out.println(evalExpression(s, c));
//		String test = "x y";
//		String[] spl = splitList(test);
		
		Elem e = Utils.parse(s);
		System.out.println(e.eval());
		ProgEvalVisitor pev = new ProgEvalVisitor();
		e.accept(pev);
		System.out.println(pev.result);
		
//		CorrectnessVisitor cv = new CorrectnessVisitor();
//		Elem e = Utils.parse(s);
//		System.out.println(e.eval());
//		e.accept(cv);
//		System.out.println(cv.hasResult());
		
		
		
//		String[] test = splitList(s);
//		System.out.println(test[0]);
//		Elem e = Utils.parse(test[0]);
//		System.out.println(e.eval());
		
		
		
		
		
		
//		ShowVisitor v = new ShowVisitor();
//		e.accept(v);
//		System.out.println(v.st.get(0));
		
//		ExpEvalVisitor eev = new ExpEvalVisitor();
//		e.accept(eev);
//		System.out.println(eev.st.get(0));
		
		
		
		
		
		
		
		
		
		
//		Context c = new Context();
//		c.add("a", 10);
//		c.add("b", 15);
//		String str = "[* [+ 1 a] 2]";
//		System.out.println(evalExpression(str, c));
	}
	
}
