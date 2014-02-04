#include <stdio.h>

int main(void) {
    FILE *ifile, *ofile;
    int i;
    long size;
    float time;

    ifile = fopen("mc.dat", "r");
    ofile = fopen("mc1.dat", "w");
    for (i = 0; i < 21; i++) {
        fscanf(ifile, "%ld %f", &size, &time);
        fprintf(ofile, "%d %ld %f\n", i, size, time);
    }
    fclose(ifile);
    fclose(ofile);

    ifile = fopen("nmc.dat", "r");
    ofile = fopen("nmc1.dat", "w");
    for (i = 0; i < 21; i++) {
        fscanf(ifile, "%ld %f", &size, &time);
        fprintf(ofile, "%d %ld %f\n", i, size, time);
    }
    fclose(ifile);
    fclose(ofile);

    ifile = fopen("blk.dat", "r");
    ofile = fopen("blk1.dat", "w");
    for (i = 0; i < 21; i++) {
        fscanf(ifile, "%ld %f", &size, &time);
        fprintf(ofile, "%d %ld %f\n", i, size, time);
    }
    fclose(ifile);
    fclose(ofile);

    ifile = fopen("nblk.dat", "r");
    ofile = fopen("nblk1.dat", "w");
    for (i = 0; i < 21; i++) {
        fscanf(ifile, "%ld %f", &size, &time);
        fprintf(ofile, "%d %ld %f\n", i, size, time);
    }
    fclose(ifile);
    fclose(ofile);
    return 0;
}
