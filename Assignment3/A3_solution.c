/* FILE: A3_solutions.c is where you will code your answers for Assignment 3.
 * 
 * Each of the functions below can be considered a start for you. They have 
 * the correct specification and are set up correctly with the header file to
 * be run by the tester programs.  
 *
 * You should leave all of the code as is, especially making sure not to change
 * any return types, function name, or argument lists, as this will break
 * the automated testing. 
 *
 * Your code should only go within the sections surrounded by
 * comments like "REPLACE EVERTHING FROM HERE... TO HERE.
 *
 * The assignment document and the header A3_solutions.h should help
 * to find out how to complete and test the functions. Good luck!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "A3_provided_functions.h"

unsigned char*
bmp_open( char* bmp_filename,        unsigned int *width, 
          unsigned int *height,      unsigned int *bits_per_pixel, 
          unsigned int *padding,     unsigned int *data_size, 
          unsigned int *data_offset                                  )
{ //Opening a BMP file and storing the header fields in variables

  unsigned char *img_data=NULL;
  
  FILE *bmpfile = fopen(bmp_filename, "rb");

  if (bmpfile == NULL) {
	  printf("I was unable to open the file.\n");
	  return NULL;
  }

  char b,m;
  fread(&b,1,1,bmpfile);
  fread(&m,1,1,bmpfile);

  unsigned int overallFileSize;
  fread(&overallFileSize,1,sizeof(unsigned int), bmpfile);
  rewind(bmpfile);

  img_data = (unsigned char*)malloc(sizeof(unsigned char) * overallFileSize);
  if (img_data == NULL) {
	  printf("I was unable to allocate memory for the image data.");
	  return NULL;
  } 

  if (fread(img_data, 1, overallFileSize, bmpfile) != overallFileSize) {
	  printf("I was unable to read the requested %d bytes.\n", overallFileSize);
	  return NULL;
  }
  
  *width = *(unsigned int*)(img_data+18);
  
  *height = *(unsigned int*)(img_data+22);

  unsigned short* new_bits_ptr = (unsigned short*)(img_data+28);
  *bits_per_pixel = (unsigned int) *new_bits_ptr;

  *data_offset = *(unsigned int*)(img_data+10);

  *data_size = *(unsigned int*)(img_data+2);

  *padding = (4 - ((*width) * ((*bits_per_pixel)/8)) % 4) % 4;

  return img_data;  
}

void 
bmp_close( unsigned char **img_data )
{
  //Freeing the memory allocated for the image data

  free(*img_data);
  *img_data = NULL;
}

unsigned char***  
bmp_scale( unsigned char*** pixel_array, unsigned char* header_data, unsigned int header_size,
           unsigned int* width, unsigned int* height, unsigned int num_colors,
           float scale )
{ //Scaling a BMP image (based on the "scale" parameter of this function
  unsigned char*** new_pixel_array = NULL; 
 
  //Modifying height, width and file size, and putting this in header data
  *height = (int) (*height)*scale;
  *(unsigned int*)(header_data+22) = *height;

  *width = (int) (*width)*scale;
  *(unsigned int*)(header_data+18) = *width;

  int padding = (4 - ((*width) * num_colors) % 4) % 4;
  int overallFileSize = header_size + ((*width)*num_colors + padding)*(*height);
  *(unsigned int*)(header_data+2) = overallFileSize;

  //Allocating memory for new 3D array
  new_pixel_array = (unsigned char***)malloc(sizeof(unsigned char**) * (*height));
  if (new_pixel_array == NULL) {
	  printf("Error. Failed to allocate memory for new pixel array.");
	  return NULL;
  }

  for (int row=0; row < *height; row++) {
	  new_pixel_array[row] = (unsigned char**)malloc(sizeof(unsigned char*) * (*width));
	  for (int col=0; col <* width; col++) {
		  new_pixel_array[row][col] = (unsigned char*)malloc(sizeof(unsigned char) * (num_colors));
          }
  }

  //Filling the new scaled array
  for (int row = 0; row < *height; row++) {
	  for (int col = 0; col < *width; col++) {
		  int scaled_row = (int) row/scale;
		  int scaled_col = (int) col/scale;
		  for (int color = 0; color < num_colors; color++) {
			  new_pixel_array[row][col][color] = pixel_array[scaled_row][scaled_col][color];
		  }
	  }
  }

  return new_pixel_array;
}         

int 
bmp_collage( char* background_image_filename,     char* foreground_image_filename, 
             char* output_collage_image_filename, int row_offset,                  
             int col_offset,                      float scale )
{ //Superimposes one BMP picture on top of another based on the "alpha" values at each pixel
  
  unsigned char* fg_header_data;
  unsigned int fg_header_size, fg_width, fg_height, fg_num_colors;
  unsigned char*** fg_initial_pixel_array = NULL;
  unsigned char*** fg_pixel_array = NULL;

  fg_initial_pixel_array = bmp_to_3D_array(foreground_image_filename, &fg_header_data, &fg_header_size, &fg_width, &fg_height, &fg_num_colors);

  fg_pixel_array = bmp_scale(fg_initial_pixel_array, fg_header_data, fg_header_size, &fg_width, &fg_height, fg_num_colors, scale);

  unsigned char* bg_header_data;
  unsigned int bg_header_size, bg_width, bg_height, bg_num_colors;
  unsigned char*** bg_pixel_array = NULL;

  bg_pixel_array = bmp_to_3D_array(background_image_filename, &bg_header_data, &bg_header_size, &bg_width, &bg_height, &bg_num_colors);

  if (fg_pixel_array == NULL || bg_pixel_array == NULL) {
	  printf("Error: Problem processing the input images.");
	  return -1;
  }

  if (fg_num_colors !=4 || bg_num_colors !=4) {
	  printf("ERROR: At least one of the input images does not have 32 bits per pixel as required.");
	  return -1;
  }

  if (fg_height > bg_height || fg_width > bg_width) {
	  printf("ERROR: The scaled foreground image is bigger than the background image.");
	  return -1;
  }

  if (fg_height + row_offset > bg_height || fg_width + col_offset > bg_width) {
	  printf("ERROR: The offsets do not allow for the foreground image to be entirely contained within the background.");
	  return -1;
  }
  
  int fg_row = -1;
  int fg_col = -1;

  for (int row = 0; row < bg_height; row++) {
	if (row >= row_offset && fg_row != fg_height-1) {
		fg_row++;
	}
	else if (fg_row == fg_height-1) {
		break;
	}
  	for (int col = 0; col < bg_width; col++) {
		if (col >= col_offset && fg_col != fg_width-1) {
			fg_col++;
		}
		else if (fg_col == fg_width-1) {
			fg_col = -1;
			break;
		}
  		for (int color = 0; color < bg_num_colors; color++) {
			if (fg_row >= 0 && fg_col >= 0) {
			       if (fg_pixel_array[fg_row][fg_col][0] != 0)
					bg_pixel_array[row][col][color] = fg_pixel_array[fg_row][fg_col][color];
			}
		}
	}
  }

  bmp_from_3D_array(output_collage_image_filename, bg_header_data, bg_header_size, bg_pixel_array, bg_width, bg_height, bg_num_colors);




  // TO HERE! 
  return 0;
}              

