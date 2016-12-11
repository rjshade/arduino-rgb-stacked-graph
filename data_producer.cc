#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main()
{
  // Random seed data for graph.
  int data[] = {rand()%8,rand()%8,rand()%8, rand()%8,rand()%8};

  while(true) {
    FILE *file = fopen("/dev/cu.usbmodem2436491","w");
    for(int j = 0 ; j < 5 ; j++)
    {
      data[j] = data[j] + (1 - rand() % 3);
      if (data[j] < 0) { data[j] = 0; }
      if (data[j] > 8) { data[j] = 8; }
      fprintf(file,"%d",data[j]); //Writing to the file
      fprintf(file,"%c",','); //To separate digits
    }
    fprintf(file,"%c",';');
    fclose(file);
    sleep(1);
  }
}
