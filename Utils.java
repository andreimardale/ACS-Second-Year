package homeworkPP;


public class Utils {
	
	public static Elem parse (String s){
		s = s.trim();
		if (s.startsWith("[")){
			return parseList(s.substring(1, s.length()));
		}
		else {
			String str = s.substring(0).trim();
			int i = 0;
			if (CorrectnessVisitor.isInteger(str) == true){
				return new Value (str);
			}
			
			while (i < str.length()){
				
				if (str.charAt(i) == ']' || str.charAt(i) == ' ')
					break;
				i++;
			}
			return new Value (s.substring(0,i));
		}
			
	}
	
	private static int getEnd (String s){
		int start = 1;
		int i = 0;
		while (start > 0){
			if (s.charAt(i) == '[')
				start ++;
			if (s.charAt(i) == ']')
				start--;
			i++;
		}
		return i;
	}
	
	private static Elem parseList (String s){
		if (s.startsWith("*")){
			Or ori = new Or();
			String piece = s.substring(2, getEnd(s) - 1);
			String splitted[] = Main.splitList(piece);
			
			ori.add(parse(splitted[0]));
			ori.add(parse(splitted[1]));
			
			return ori;
		}
		if (s.startsWith("+")){
			Plus plus = new Plus();
			
			String piece = s.substring(2, getEnd(s) - 1);
			String splitted[] = Main.splitList(piece);
			
			plus.add(parse(splitted[0]));
			plus.add(parse(splitted[1]));
			
			return plus;
		}
		if (s.startsWith(";")){
			Semicolon sc = new Semicolon();
			
			String piece = s.substring(2, getEnd(s)-1);
			String splitted[] = Main.splitList(piece);
			
			sc.add(parse(splitted[0]));
			sc.add(parse(splitted[1]));
			
			return sc;
		}
		if (s.startsWith("== ")){
			EqualEqual ee = new EqualEqual();
			
			String piece = s.substring(3, getEnd(s)-1);
			String[] splitted = Main.splitList(piece);
			
			
			ee.add(parse(splitted[0]));
			ee.add(parse(splitted[1]));
			
			return ee;
		}
		if (s.startsWith("=")){
			Equal eq = new Equal();
			
			String piece = s.substring(2, getEnd(s) - 1);
			String splitted[] = Main.splitList(piece);
			
			eq.add(parse(splitted[0]));
			eq.add(parse(splitted[1]));
			
			return eq;
		}
		
		if (s.startsWith("return")){
			Return r = new Return();
			
			r.add(parse(s.substring(6)));
			return r;
		}
		
		if (s.startsWith("<")){
			Less l = new Less();
			
			String piece = s.substring(2, getEnd(s) - 1);
			String splitted[] = Main.splitList(piece);
			
			l.add(parse(splitted[0]));
			l.add(parse(splitted[1]));
			
			return l;
		}
		
		if (s.startsWith("while")){
			While w = new While();
			
			String piece = s.substring(6, getEnd(s) - 1);
			String splitted[] = Main.splitList(piece);
			
			w.add(parse(splitted[0]));
			w.add(parse(splitted[1]));
			
			return w;
		}
		
		if (s.startsWith("if")){
			If i = new If();
			
			String piece = s.substring(3, getEnd(s) - 1);
			String splitted[] = Main.splitList(piece);
			
			i.add(parse(splitted[0]));
			i.add(parse(splitted[1]));
			i.add(parse(splitted[2]));
			
			return i;
		}
		
		
		String str = s.substring(0);
		int i = 0;
		while (i < str.length()){
			i++;
			if (str.charAt(i) == ']' || str.charAt(i) == ' ')
				break;
		}
		return new Value (s.substring(0,i));
			
	}
}
