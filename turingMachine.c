# include <stdio.h>
# include <stdlib.h>

/* structura pentru a salva o tranzitie */
typedef struct transition 
{
	int currentS;
	int currentC;
	int newS;
	int newC;
	char pos;
} Ttranzitie, *Ttrans;
/* structura pentru masina turing */
typedef struct turing
{
	int currentState;
	int currentChar;
	int cursor;
	int holdState;

} TuringMachine;
/* functie pentru parsarea datelor din fisierul de intrare*/
void parseInputFile (FILE *f, int* noTrans, Ttrans transitions )
{
	int s, sf, i, j = -1, index = 0;
	int stateBuffer;
	char charBuffer, posBuffer; 
	/* citesc datele in buffere pentru a nu salva spatii goale in caz ca gasesc -1 */
	fscanf(f, "%d %d", &s, &sf);
	(*noTrans) = (s - sf) * 3;

	for (i = 0; i < (s - sf) * 3; i++)
	{
		
		fscanf(f, "%d", &stateBuffer);
		/* verific intai daca exista tranzitie */
		if (stateBuffer == -1) 
		{
			if (i == (*noTrans))
			{	
				break;
			}
			if (index*(-1) % 3 == 0) /* index contorizeaza tipul caracterului curent 0, 1, # */
				j++; /* j contorizeaza numarul starii curente; la fiecare 3 caractere schimba starea */
			(*noTrans)--;
			i--;
			index++;

		}
		else 
		{
			fscanf(f, " %c %c", &charBuffer, &posBuffer);

			transitions[i].newS = stateBuffer;
			transitions[i].pos = posBuffer;

			if (charBuffer == 48)
				transitions[i].newC = 0;
			if (charBuffer == 49)
				transitions[i].newC = 1;
			if (charBuffer == 35)
				transitions[i].newC = 2;

			if (index % 3 == 0) /* din 3 in 3 creste pe j, adica trece la starea urmatoare */
				j++;
			transitions[i].currentS = j;

			transitions[i].currentC = index % 3;
			index++;
		}	
	}	
}
/* functie pentru a citi si parsa cuvantul; mapeaza pentru fiecare # cifra 2 */
int* parseWord (FILE* f, int *dimBand)
{
	char c;
	int i = 0;
	int *w = (int*) malloc (100 * sizeof (int));
	if (!w)
		return;
	do
	{
		fscanf (f, "%c", &c);
		if (c == 48) // am citit 0
			w[i++] = 0;
		if (c == 49) // am citit 1
			w[i++] = 1;
		if (c == 35) // am citit #
			w[i++] = 2;
	} while (c != '\n');
		
	(*dimBand) = i - 1;

	return w;
}
/* afiseaza ce se afla pe banda */
void printBand (int* w, int n)
{
	int i;
	for (i = 0; i <= n; i++)
		printf("%d", w[i]);
	printf("\n");

}
/* executa un pas din Masina Turing*/
int doOneStep (TuringMachine** m, Ttrans tranzitii, int* w, int noTrans)
{
	int i;
	if ((*m)->holdState == 1)
		return 2; /* daca ajung in stare Hold, returnez codul 2 */
	for (i = 0; i < noTrans; i++) /* la fiecare pas, caut in vectorul de tranzitii daca exista tranzitie */
	{
		if ((*m)->currentState == tranzitii[i].currentS && w[(*m)->cursor] == tranzitii[i].currentC)
		{	
			w[(*m)->cursor] = tranzitii[i].newC; /* modific caracterul indicat de cursor */
			(*m)->currentState = tranzitii[i].newS; /* modific starea indicata de cursor */
			if (tranzitii[i].pos == 'L') /* mut cursorul la stanga dreapta sau pe loc */
			{
				(*m)->cursor--;
				(*m)->holdState = 0;
			}			
			if (tranzitii[i].pos == 'R')
			{
				(*m)->cursor++;
				(*m)->holdState = 0;
			}
			if (tranzitii[i].pos == 'H')
				(*m)->holdState = 1;
			return 1; /* daca execut tranzitie, returnez codul 1 */
		}

	}
	return 0; /* codul 0 in caz ca nu exista tranzitie */

}
/* 0- nu exista tranzitie; 1 - exista tranzitie si o executa; 2- a ajuns la final */
int playTM (TuringMachine** m, Ttrans tranzitii, int* w, int noTrans)
{ 
	int ok = 0; 
	do {
		ok = doOneStep(m, tranzitii, w, noTrans);
		if (ok == 2)
			break;
	} while (ok==1);

	return ok;
}
/* formatez output in fisierul de iesire*/
void formatOutput (int* w, int dimBand, FILE* g, int rez)
{
	int i;
	if (rez == 0)
		{
			fprintf(g, "Eroare!");
			return;
		}
	for (i = 0; i <= dimBand; i++)
	{
		if (w[i] == 2)
			fprintf(g, "#");
		else
			fprintf(g, "%d", w[i]);
	}
}

int main ()
{
	int s, sf, dimBand = 0, noTrans = 0, i;
	int* w;
	TuringMachine* m = (TuringMachine*)malloc(sizeof (TuringMachine));

	FILE* f = fopen ("date.in", "rt");
	FILE* g = fopen("date.out", "wt");
	if (!f){
		printf ("Error opening the input file!\n");
		return -1;
	}

	w = parseWord(f, &dimBand);

	Ttrans tranzitii = (Ttrans) malloc (noTrans * sizeof(Ttranzitie));
	parseInputFile(f, &noTrans, tranzitii);
	
	m->currentState = 0;
	m->cursor = 1;

	int rez = playTM(&m, tranzitii, w, noTrans);

	formatOutput(w, dimBand, g, rez);

	free(m);

	fclose(f);
	fclose(g);

	return 0;
}
