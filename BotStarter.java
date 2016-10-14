// // Copyright 2016 theaigames.com (developers@theaigames.com)

//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at

//        http://www.apache.org/licenses/LICENSE-2.0

//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
//    For the full copyright and license information, please view the LICENSE
//    file that was distributed with this source code.

package bot;
import java.util.ArrayList;

/**
 * BotStarter class
 * 
 * Magic happens here. You should edit this file, or more specifically
 * the makeTurn() method to make your bot do more than random moves.
 * 
 * @author Jim van Eeden <jim@starapple.nl>
 */

public class BotStarter {
	static int count = 0;
	/**
	 * clasa auxiliara pentru a reprezenta outputul algoritmului minmax
	 *
	 */
	class Best {
		int bestScore;
		int x;
		int y;
		
		public Best( int bestScore, int x, int y) {
			this.x = x;
			this.y = y;
			this.bestScore = bestScore;
		}
	}
	
    /**
     * Makes a turn. Edit this method to make your bot smarter.
     * Currently does only random moves.
     *
     * @return The column where the turn was made.
     */
	public Move makeTurn(Field field) {
		Best b = minimax(field, 7, BotParser.mBotId, Integer.MIN_VALUE, Integer.MAX_VALUE);
		Move m = new Move(b.x, b.y);
		return m;
	}
	
	public static int winner(int[][] board, int player) {
		if (board[0][0] == player && board[0][0] == board[1][1] && board[0][0] == board[2][2]) 
			return player;
		
		if (board[0][2] == player && board[0][2] == board[1][1] && board[0][2] == board[2][0]) 
			return player;
		
		for (int i = 0; i < 3; i++) {
			if (board[i][0] == player && board[i][0] == board[i][1] && board[i][0] == board[i][2])
				return player;
			if (board[0][i] == player && board[0][i] == board[1][i] && board[0][i] == board[2][i])
				return player;
		}
		return 0;
	}
	
	public static int getScore (Field f, int player) {
		int[][] macroB = new int[3][3];
		int[][] miniB = new int[3][3];
		int opponent = (player == 1) ? 2 : 1;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				miniB = getSubMatrix (f.getmBoard(), 3 * i, 3 * j);
				macroB[i][j] = winner(miniB, player);
				if (macroB[i][j] == 0) 
					macroB[i][j] = winner(miniB, opponent);
			}
		}
		

		if (winner(macroB, player) == player)
			return 1000000 + freeCells(f.getmBoard());
		else if (winner(macroB, opponent) == opponent)
			return -1000000 - freeCells(f.getmBoard());
		if (f.isFull())
			return 0;
		int value = evaluate(macroB, player) * 100;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				value += evaluate(getSubMatrix (f.getmBoard(), 3 * i, 3 * j), player);
			}
		}
		return value;
	}	
	
	public static int freeCells(int[][] macroB) {
		int free = 0;
		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 9; j++)
				if (macroB[i][j] == 0)
					free++;
		return free;
	}
	
	public static int evaluate(int[][] board, int player) {
		int value = 0;
		int opponent = (player == 1) ? 2 : 1;
		
		int[][] winPosition = { 
				  { 0, 1, 2 },
				  { 3, 4, 5 },
				  { 6, 7, 8 },
				  { 0, 3, 6 },
				  { 1, 4, 7 },
				  { 2, 5, 8 },
				  { 0, 4, 8 },
				  { 2, 4, 6 }
				};
		int[][] score = {
				  {     0,   -10,  -100, -1000 },
				  {    10,     0,     0,     0 },
				  {   100,     0,     0,     0 },
				  {  1000,     0,     0,     0 }
				};
		for (int i = 0; i < 8; i++) {
			int pointsP = 0;
			int pointsO = 0;
			for (int j = 0; j < 3; j++) {
				int side = board[winPosition[i][j] / 3][winPosition[i][j] % 3];
				if (side == player)
					pointsP++;
				if (side == opponent)
					pointsO++;
			}
			value += score[pointsP][pointsO];
		}
		return value;
	}
	
	public static int[][] getSubMatrix (int[][] board, int l, int c){
		int[][] subMatrix =  new int[3][3];
		for (int i = 0; i < 3; i++) 
			for (int j = 0; j < 3; j++) 
				subMatrix[i][j] = board[l + i][c + j];
		return subMatrix;
	}
	
	public static boolean hasWon (int[][] board){
		int win1 = 0;
		win1 = winner(board, BotParser.mBotId);
		int win2 = 0;
		win2 = winner(board, swapId(BotParser.mBotId));
		if (win1 == 0 && win2 == 0)
			return false;
		return true;
	}
	/**
	 * Metoda pentru a inversa un ID. Folosita in algoritmul recursiv minimax
	 * pentru a intra in recursivitate pe randul oponentului
	 * @param id Daca primeste 1, intoarce 2 si viceversa.
	 * @return
	 */
	private static int swapId(int id){
		if (id == 1)
			return 2;
		if (id == 2)
			return 1;
		return 0;
	}
	/**
	 * Metoda care verifica daca mai e spatiu disponibil intr-o matrice mica,
	 * de 3 x 3.
	 * @param m Matricea de verificat
	 * @return true daca mai e loc, false altfel
	 */
	private boolean littleFull (int [][] m){
		for (int x = 0; x < 3; x++) {
			  for (int y = 0; y < 3; y++) {
				  if (m[x][y] > 0) {
					  return false;
				  }
			  }
		}
		return true;
	}
	/**
	 * Metoda pentru a seta o stare in macroboard.Modificarile se fac in felul urmator :
	 * id_meu = 1 ; id_lui = 2; full = 3; inca_Merge = 0
	 * @param f Field-ul de setat
	 * @param row indicele liniei ce urmeaza sa fie modificata
	 * @param column indicele coloanei ce urmeaza sa fie modificata
	 * 
	 */
	public void setMacro (Field f, int row, int column){
		
		int [][] smallMatrix = findSubMatrix(f.getmBoard(), row, column);
		int win = 0;
		win = winner(smallMatrix, BotParser.mBotId); // id_meu daca am castigat sau 0 - altceva
		if (win != BotParser.mBotId){ // daca nu am castigat
			int opp_id = swapId(BotParser.mBotId);
			win = winner (smallMatrix, opp_id); // id_lui daca a castigat, 0 altceva 
			if (win != opp_id && littleFull(smallMatrix)){
				win = 3; //remiza
			}
		}
		f.setMacroboard(row/3, column/3, win);
		
		if (f.getValueOfMacro(row%3, column%3) > 0){
			for (int i = 0; i < 3; i ++){
				for (int j = 0; j < 3; j++){
					if (f.getValueOfMacro(i, j) == 0){
						f.setMacroboard(i, j, -1);
					}
				}
			}
			return;
		}
		else {
			for (int i = 0; i < 3; i ++){
				for (int j = 0; j < 3; j++){
					if (f.getValueOfMacro(i, j) == -1){
						f.setMacroboard(i, j, 0);
					}
				}
			}
			f.setMacroboard(row%3, column%3, -1);
		}
	}
	
	public int[][] findSubMatrix (int[][] board, int l, int c){
		int i, j;
		i = l - l%3;
		j = c - c%3;
		return getSubMatrix(board, i, j);
	}
	/**
	 * Implementarea efectiva a algoritmului minimax. 
	 * @param f Starea jocului
	 * @param level Cat de mult sa intre in recursivitate
	 * @param bot_id ID-ul persoanei al carei rand este
	 * @param alfa coeficient alfa pentru taiera alfa-beta
	 * @param beta coeficient beta pentru taierea alfa-beta
	 * @return un Best -> o clasa ce incapsuleaza scorul cel mai bun si pozitia
	 * pe care s-a obtinut acel scor
	 */
	public Best minimax(Field f, int level, int color, int alfa, int beta){
		count++; // pentru a masura numarul de intrari in recursiviate
		int score;
		int bestColumn = -1, bestRow = -1;
		int bestValue;
		ArrayList<Move> children = f.getAvailableMoves(); // toate mutarile posibile din pct curent
		
		/* daca s-a ajuns la nivelul maxim setat de recursivitate 
		 * sau daca nu mai sunt mutari
		 * sau daca am castigat 
		 * atunci calculam scorul obtinut in acea pozitie si intoarcem mutarea respectiva*/
		if (level == 0 || children.isEmpty() || hasWon(f.getmMacroboard()) == true){
			score = color * getScore (f, color);
			return new Best(score, bestRow, bestColumn);
		}
		else {
			/* obtinem toate mutarile posibile */
			bestValue = Integer.MIN_VALUE;
			for (Move m : children){
				int backupBoard = f.getValueOfBoard(m.mX, m.mY); // copie pentru o viitoare restaurare
				int backupMacro = f.getValueOfMacro(m.mX % 3, m.mY % 3);
				f.setMove(m.mX, m.mY, color); // setam mutarea curenta
				setMacro(f, m.mX, m.mY); // actualizam si starea tabelei mari
				
				Best bst = minimax (f, level-1, -color, -beta, -alfa);
				int v = -bst.bestScore;
				if (v > bestValue){
					bestValue = v;
					bestRow = m.mX;
					bestColumn = m.mY;
				}
				
				int localAlfa = Math.max(alfa, v);
				if (localAlfa >= beta )
					break;

				f.setMove(m.mX, m.mY, backupBoard); // revin la setarile de dinainte de recursie
				f.setMacroboard(m.mX % 3, m.mY % 3, backupMacro);
				/* ma opresc daca alfa e mai mare ca beta*/
			}
		}
		/* daca a fost tura mea, intorc alfa row col, altfel intorc beta, row, col*/
		return new Best( bestValue, bestRow, bestColumn);
	}

	public static void afisare(int [][]a, int [][]b){
		for(int i = 0;i < 3;i++){
			for(int j = 0;j < 3;j ++){
				System.out.print(a[i][j] + " ");
				if(j == 2 || j == 5)
					System.out.print(" ");
			}
			System.out.println();
			if(i == 2 || i == 5)
				System.out.println();
		}
		
		for(int i = 0;i < 9;i++){
			for(int j = 0;j < 9;j ++){
				System.out.print(b[i][j] + " ");
				if(j == 2 || j == 5)
					System.out.print(" ");
			}
			System.out.println();
			if(i == 2 || i == 5)
				System.out.println();
		}
	}

	public static void main(String[] args) {
//		BotParser parser = new BotParser(new BotStarter());
//		parser.run();
		BotStarter bs = new BotStarter();
		int[][] small = { {-1, -1, -1},{-1, -1, -1},{-1, -1, -1}};
		int[][] big = new int [9][9];
		
		for (int i = 0; i < 9; i++){
			for (int j = 0; j < 9; j++){
				big[i][j] = 0;
			}
		}
		afisare(small, big);
		Field f = new Field(small, big);
		BotParser.mBotId = 1;
		int my_id = 1;
		
		long startTime = System.currentTimeMillis();
		Best b = bs.minimax(f, 6, my_id, Integer.MIN_VALUE, Integer.MAX_VALUE);
		long endTime = System.currentTimeMillis();
		long time = endTime - startTime;
		System.out.println("TIME: " + time + " milisecunde");
		System.out.println("BEST: " + b.bestScore + " " + b.x + " " +  b.y);
		System.out.println("NR INSTRUCTIUNI: " + count);
	}
}
