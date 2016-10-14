# include <stdio.h>
# include <stdlib.h>

typedef struct edge 
{
	int u;
	int v;
} TEdge, *TEd ;

TEdge* parseInput (FILE* f, int* noEdge, int* noVert)
{
	int i, V, E;
	fscanf(f, "%d %d", &V, &E);
	(*noEdge) = E;
	(*noVert) = V;
	TEd edges = (TEd) malloc (E * sizeof(TEdge));

	for (i = 0; i < E; i++)
	{
		fscanf(f, "%d %d", &edges[i].u, &edges[i].v);
	}

	return edges;
}

void decode (int* clause, int dim, int noVert, FILE* g)
{
	int i;
	for (i = 0; i < dim; i++)
	{
		if (clause[i] == 0)
			continue;
		else if (clause[i] == -1)
			fprintf(g, "(");
		else if (clause[i] == -2)
			fprintf(g, ")");
		else if (clause[i] == -3)
			fprintf(g, "^");
		else if (clause[i] == -4)
			fprintf(g, "V");
		else if (clause[i] == -5)
			fprintf(g, "~");
		else
		{
			int first = clause[i] % 10; //j
			int second = clause[i] / 10; // i
			fprintf(g, "x%d", (second-1)*noVert+first);
			// printf("%c ", codes[second-1][first-1]);
		}
	}
}

/* (-> -1   )-> -2  ^ -> -3     V->-4     */
int clause1 (int noVert, int* clause)
{
	int i, j, k = 0;
	for (i = 1; i <= noVert; i++ )
	{
		clause[k++] = -1;
		for (j = 1; j <= noVert; j++)
		{
			if (j != noVert)
			{
				clause[k++] = i*10 + j;
				clause[k++] = -4;
			}
			else
			{
				clause[k++] = i*10 + j;
			}	
		}
		if (i != noVert)	
		{
			clause[k++] = -2;
			clause[k++] = -3;
		}
		else
		{
			clause[k++] = -2;
		}
	}
	clause[k++] = -3;
	return k;
}

int clause2(int noVert, int* clause)
{
	int i, j, k = 0;
	for (i = 1; i <= noVert; i++ )
	{
		clause[k++] = -1;
		for (j = 1; j <= noVert; j++)
		{
			if (j != noVert)
			{
				clause[k++] = j*10 + i;
				clause[k++] = -4;
			}
			else
			{
				clause[k++] = j*10 + i;
			}	
		}
		if (i != noVert)	
		{
			clause[k++] = -2;
			clause[k++] = -3;
		}
		else
		{
			clause[k++] = -2;
		}
	}
	clause[k++] = -3;
	return k;
}
/* (-> -1   )-> -2  ^ -> -3     V->-4  ~->-5   */
/* Nu vreau 2 noduri pe aceeasi poziti -> !(xij si xik)*/
int clause3(int noVert, int* clause)
{
	int i, j, k, l = 0;
	for (i = 1; i <= noVert; i++)
	{
		for (j = 1; j <= noVert; j++)
		{
			for (k = j; k <= noVert; k++)
			{
				if (k == j)
					continue;
				clause[l++] = -1;
				clause[l++] = -5;
				clause[l++] = i*10+j;
				clause[l++] = -4;
				clause[l++] = -5;
				clause[l++] = i*10+k;
				clause[l++] = -2;
				clause[l++] = -3;
			}
		}
	}
	return l;
}
/* Nu vreau acelasi nod pe mai multe pozitii*/
int clause4 (int noVert, int* clause)
{
	int i, j, k, l = 0;
	for (j = 1; j <= noVert; j++)
	{
		for (i = 1; i <= noVert; i++)
		{
			for (k = i; k <= noVert; k++)
			{
				if (k == i)
					continue;
				clause[l++] = -1;
				clause[l++] = -5;
				clause[l++] = i*10+j;
				clause[l++] = -4;
				clause[l++] = -5;
				clause[l++] = k*10+j;
				clause[l++] = -2;
				clause[l++] = -3;
			}
		}
	}
	return l;
}

int included (TEdge* tArray, int j, int k, int noEdge)
{
	int index;
	for (index = 0; index < noEdge; index++)
	{
		if ((tArray[index].u == j && tArray[index].v == k) || (tArray[index].u == k && tArray[index].v == j))
			return 1;
	}

	return 0;
}

/* (-> -1   )-> -2  ^ -> -3     V->-4  ~->-5   */
int clause5 (int noVert, int* clause, TEdge* edge, int noEdge)
{
	int i, j, k, l = 0;
	for (i = 1; i <= noVert - 1; i++)
	{
		for (j = 1; j <= noVert; j++)
		{
			for (k = 1; k <= noVert; k++)
			{
				// if (k == j)
				// 	continue;
				if (included (edge, j, k, noEdge) == 0)
				{
					// printf("Elementul introdus este %d%d %d%d \n", i,j,(i+1),k );
					clause[l++] = -1;
					clause[l++] = -5;
					clause[l++] = i*10+j;
					clause[l++] = -4;
					clause[l++] = -5;
					clause[l++] = (i+1)*10+k;
					clause[l++] = -2;
					clause[l++] = -3;
				}
			}
		}
	}
	clause[l-1] = 0;
	return l;
}

int main ()
{
	FILE* f = fopen("test.in", "rt");
	FILE* g = fopen("test.out", "wt");
	if (!f)
	{
		printf("Eroare la deschiderea fisierului!");
		return -1;
	}

	int noVert, noEdge, i, j;
	
	TEd edges = parseInput(f, &noEdge, &noVert);
	
	int dimension = 4* (noVert * noVert * noVert);
	int* clause = (int*) malloc (dimension * sizeof (int));
	int* clauseii = (int*) malloc (dimension * sizeof (int));
	int* clauseiii = (int*) malloc (dimension * sizeof (int));
	int* clauseiv = (int*) malloc (dimension * sizeof (int));
	int* clausev = (int*) malloc (dimension * sizeof (int));

	int k = clause1(noVert, clause);
	decode(clause, k, noVert, g);
	
	int k2 = clause2(noVert, clauseii);
	decode(clauseii, k2, noVert, g);

	int k3 = clause3(noVert, clauseiii);
	decode(clauseiii, k3, noVert, g);
	
	int k4 = clause4 (noVert, clauseiv);
	decode(clauseiv, k4, noVert, g);
	
	int k5 = clause5(noVert, clausev, edges, noEdge);
	decode(clausev, k5, noVert, g);
	
	return 0;

}