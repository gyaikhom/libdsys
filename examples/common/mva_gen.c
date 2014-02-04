#include <stdio.h>
#define MAX 20

int main(int argc, char *argv[])
{
	FILE *out;
	char fname[128];
	float tolerance;
	int nclass, nqueue, i, j;

	out = fopen(argv[1], "w");
	printf("#class #queue tolerance");
	scanf("%d %d %f", &nclass, &nqueue, &tolerance);

	printf("%d %d %f\n", nclass, nqueue, tolerance);
 
	/* Header. */
	fprintf(out, "%d %d %f\n", nclass, nqueue, tolerance);

	/* Queue type. */
	for (i = 0; i < nqueue; i++) fprintf(out, "1 ");
	fprintf(out, "\n");

	/* Load intensity vector. */
	for (i = 0; i < nclass; i++) fprintf(out, "%d ", (random() % MAX) * 100);
	fprintf(out, "\n");

	/* Visitation vector. */
	for (i = 0; i < nclass; i++) fprintf(out, "%d ", random() % MAX);
	fprintf(out, "\n");

	/* Service demand array. */
	for (i = 0; i < nclass; i++) 
		for (j = 0; j < nqueue; j++) fprintf(out, "%d ", random() % MAX);
	fprintf(out, "\n");	

	fclose(out);
}
