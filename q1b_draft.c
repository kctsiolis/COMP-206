#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int H = 0;
int L = 0;

int h_t = 0;
int num_rows = 0;
int relative_rows = 0;
int m = 2;
int spaces = 0;
int stars = 0;

void print_row(int row, int h_t, int l) {
	if (l == 1) {
		spaces += h_t - 1;
		stars += 1;

		for (int i = 0; i < h_t; i++) {
			for (int j = 0; j < spaces-i; j++) {
				printf(" ");
			}
			for (int k = 0; k < stars + 2*i; k++) {
				printf("*");
			}
			for (int j = 0; j < spaces-i; j++) {
				printf(" ");
			}
			printf("\n");
		}
		relative_rows = num_rows/2;
		m = 2;
		spaces = 0;
		stars = 0;
	}
	else {
		if (row < relative_rows) {
			spaces += H/(pow(2,m));
			m++;
			print_row(row, h_t/2, l-1);
		}
		else {
			print_row(row-h_t/2, h_t/2, l-1);
			printf(" ");
			print_row(row-h_t/2, h_t/2, l-1);
		}
		relative_rows = relative_rows/2;
	}

}

int main(int argc, const char *argv[]) {
	//DO THE CHECKINGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG
	H = atoi(argv[1]);
	L = atoi(argv[2]);
	h_t = H/2+1;
	num_rows = pow(2,L-1);
	relative_rows = num_rows/2;

	for (int i = 0; i < num_rows; i++) {
		print_row(i, h_t, L);
	}
}

