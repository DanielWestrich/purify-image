/* 
 * Title: purify.c
 * By: Daniel Westrich, Spring 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <pnmrdr.h>
#include <stdbool.h>
#include <stack.h>
#include "bit2.h"
#include "uarray2.h"

/*
 *ensures the file is valid, readable, and can be used to create a 
 *valid Pnmrdr_T (of type pbm)
 */
Pnmrdr_T check_file_validity(FILE *file);
/*
 *creates, allocates, and initalizes the Bit2_T with data read from 
 *Pnmrdr_T img
 */
Bit2_T pbmread(Pnmrdr_T img);
/*
 *called by pbmread in a Bit2_map function, fills the Bit2 with 
 *data read in from the img
 */
void fill_bitmap(int row, int col, Bit2_T bitmap, int b, void *cl);

/*
 *uses the bitmap to build a stack of black edge pixels
 */
void build_stack(Bit2_T bitmap);

/*
 *edits the passed Bit2_T to remove black edge pixels using the stack of 
 *coordinates of black edge pixels
 */
void unblack(Bit2_T bitmap, Stack_T black_pixel_stack, int width, 
	     int height);

/*
 *initializes a UArray_T of boolean values to false
 */
void initialize_uarray2(UArray2_T marked, int width, int height);

/*
 *writes the edited bitmap (with black edge pixels removed) to a file
 */
void pbmwrite(FILE *outputfp, Bit2_T bitmap);

const Except_T EXCESSIVE_ARGS = {"Too many arguments. Useage: " 
				 "./unblackedges " "filename"};
const Except_T NULL_FILE = {"Could not open file"};
const Except_T NULL_IMG = {"Could not open img"};
const Except_T WRONG_IMG_TYPE = {"File type other than .pbm passed, pass a "
				 "file of type .pbm to use."};
const Except_T ALLOCATION_FAILED = {"Allocation failed"};

struct Index_Pair {
	int row;
	int col;
};

int main(int argc, char *argv[])
{
	Pnmrdr_T open_img;
	Bit2_T bitmap = NULL;
	if (argc > 2) {
	        RAISE(EXCESSIVE_ARGS);
	}

	if (argc == 2){
		FILE *file = fopen(argv[1], "r");
		if (file == NULL) {
		        RAISE(NULL_FILE);
		}
		open_img = check_file_validity(file);
		bitmap = pbmread(open_img);
		build_stack(bitmap);
		pbmwrite(stdout, bitmap);
		fclose(file);
	}
	if (argc < 2){
	        open_img = check_file_validity(stdin);
		bitmap = pbmread(open_img);
		build_stack(bitmap);
		pbmwrite(stdout, bitmap);
	}

	Bit2_free(&bitmap);
	
	exit(EXIT_SUCCESS);
}

/*
 *ensures the file is valid, readable, and can be used to create a 
 *valid Pnmrdr_T (of type pbm)
 */
Pnmrdr_T check_file_validity(FILE *file)
{
	Pnmrdr_T img;
	if (file == NULL) {
	        RAISE(NULL_FILE);
	}

	img = Pnmrdr_new(file);

	if (img == NULL) {
		RAISE(NULL_IMG);
	}

	if (Pnmrdr_data(img).type != Pnmrdr_bit){
	        RAISE(WRONG_IMG_TYPE);
	}

       	return img;
}

/*
 *creates, allocates, and initalizes the Bit2_T with data read from 
 *Pnmrdr_T img
 */
Bit2_T pbmread(Pnmrdr_T img)
{
	int img_width, img_height;
	img_width = Pnmrdr_data(img).width;
	img_height = Pnmrdr_data(img).height;

	Bit2_T bitmap = Bit2_new(img_width, img_height);
	if (bitmap == NULL) {
		RAISE(ALLOCATION_FAILED);
	}
	
	Bit2_map_row_major(bitmap, fill_bitmap, &img);

	Pnmrdr_free(&img);

	return bitmap;
}

/*
 *called by pbmread in a Bit2_map function, fills the Bit2 with 
 *data read in from the img
 */
void fill_bitmap(int row, int col, Bit2_T bitmap, int b, void *cl)
{	
	(void) b;
	Pnmrdr_T *temp_ptr = cl;
	
	Bit2_put(bitmap, row, col, Pnmrdr_get(*temp_ptr));
}

/*
 *edits the passed Bit2_T to remove black edge pixels
 */

void build_stack(Bit2_T bitmap)
{
	Stack_T black_pixel_stack = Stack_new();
	if (black_pixel_stack == NULL) {
		RAISE(ALLOCATION_FAILED);
	}
        int i, top_row, col, bottom_row, left_col, row, right_col;
	int width = Bit2_width(bitmap);
	int height = Bit2_height(bitmap);
	/*builds a stack with structs that hold indices of the black edge
	 *pixels
 	 */
	for (i = 0; i < height; i++) {
		top_row = 0;
		col = i;
		bottom_row = width - 1; 
	        if (Bit2_get(bitmap, top_row, col) == 1) {
			struct Index_Pair *ind_pair = 
					  malloc(sizeof(ind_pair));
			if(ind_pair == NULL) { 
				RAISE(ALLOCATION_FAILED);
			}
			ind_pair->row = top_row;
			ind_pair->col = col;
			Stack_push(black_pixel_stack, ind_pair);
		}
		if (Bit2_get(bitmap, bottom_row, col) == 1) {
		        struct Index_Pair *ind_pair = 
					  malloc(sizeof(ind_pair));
			if(ind_pair == NULL) { 
				RAISE(ALLOCATION_FAILED);
			}
			ind_pair->row = bottom_row;
			ind_pair->col = col;
			Stack_push(black_pixel_stack, ind_pair);
		}
	}
	for (i = 1; i < width - 1; i++) {
		left_col = 0;
		row = i;
		right_col = height - 1;
		if (Bit2_get(bitmap, row, left_col) == 1) {
		        struct Index_Pair *ind_pair = 
					  malloc(sizeof(ind_pair));
			if(ind_pair == NULL) { 
				RAISE(ALLOCATION_FAILED);
			}
			ind_pair->row = row;
			ind_pair->col = left_col;
			Stack_push(black_pixel_stack, ind_pair);
		}
		if (Bit2_get(bitmap, row, right_col) == 1) {
		        struct Index_Pair *ind_pair = 
					  malloc(sizeof(ind_pair));
			if(ind_pair == NULL) { 
				RAISE(ALLOCATION_FAILED);
			}
			ind_pair->row = row;
			ind_pair->col = right_col;
			Stack_push(black_pixel_stack, ind_pair);
		}
	}
	unblack(bitmap, black_pixel_stack, height, width);
}

/*
 *edits the passed Bit2_T to remove black edge pixels using the stack of 
 *coordinates of black edge pixels
 */
void unblack(Bit2_T bitmap, Stack_T black_pixel_stack, int width, int height)
{
        struct Index_Pair *ind_pair;
	while(Stack_empty(black_pixel_stack) == false) {
		ind_pair = Stack_pop(black_pixel_stack);
		int row = ind_pair->row;
		int col = ind_pair->col;
		/*each loop checks the current index's neighbors for more 
		 *adjacent black pixels, adding them to the stack if 
		 *necessary
		 */
		if (row - 1 >= 0){
			if(Bit2_get(bitmap, row - 1, col) == 1) {
				struct Index_Pair *up_neighbor = 
					malloc(sizeof(up_neighbor));
				if (up_neighbor == NULL) {
					RAISE(ALLOCATION_FAILED);
				}
				up_neighbor->row = row - 1;
				up_neighbor->col = col;
				Stack_push(black_pixel_stack, up_neighbor);
			}
		}
		if (col + 1 < width) {
			if(Bit2_get(bitmap, row, col + 1) == 1) {
				struct Index_Pair *right_neighbor = 
					malloc(sizeof(right_neighbor));
				if (right_neighbor == NULL) {
					RAISE(ALLOCATION_FAILED);
				}
				right_neighbor->row = row;
				right_neighbor->col = col + 1;
				Stack_push(black_pixel_stack, 
					   right_neighbor);
			}

		}
		if (row + 1 < height) {
			if(Bit2_get(bitmap, row + 1, col) == 1) {
				struct Index_Pair *down_neighbor = 
					malloc(sizeof(down_neighbor));
				if (down_neighbor == NULL) {
					RAISE(ALLOCATION_FAILED);
				}
				down_neighbor->row = row + 1;
				down_neighbor->col = col;
				Stack_push(black_pixel_stack, down_neighbor);
			}

		}
		if (col - 1 >= 0) {
			if(Bit2_get(bitmap, row, col - 1) == 1) {
				struct Index_Pair *left_neighbor = 
					malloc(sizeof(left_neighbor));
				if (left_neighbor == NULL) {
					RAISE(ALLOCATION_FAILED);
				}
				left_neighbor->row = row;
				left_neighbor->col = col - 1;
				Stack_push(black_pixel_stack, 
					   left_neighbor);
			}

		} 
		Bit2_put(bitmap, row, col, 0);
		free(ind_pair);
	}
	Stack_free(&black_pixel_stack);	
}

/*
 *writes the edited bitmap (with black edge pixels removed) to a file
 */
void pbmwrite(FILE *outputfp, Bit2_T bitmap)
{
        int width = Bit2_width(bitmap);
	int height = Bit2_height(bitmap);
	int row, col, bit;
	fprintf(outputfp, "P1\n");
	fprintf(outputfp, "%d %d\n", width, height);
	for (row = 0; row < height; row++) { 
		for (col = 0; col < width; col++) {
			bit = Bit2_get(bitmap, col, row);
			fprintf(outputfp, "%d", bit);
		}
		fputc('\n', outputfp);
	}
}

/*
 *This function initializes each element of a boolean UArray2 to false
 */
void initialize_uarray2(UArray2_T marked, int width, int height)
{
	int row;
	int col;
	bool* elem_marked = NULL;
       
	for (col = 0; col < width; col++) {
		for (row = 0; row < height; row++) {
			elem_marked = UArray2_at(marked, row, col);
			*elem_marked = false;
		}
	}
}
