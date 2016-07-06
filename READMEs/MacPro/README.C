=============================================================================
C language notes
=============================================================================
Last updated on 11/17/14; 12/9/14; 1/5/15; 5/28/15
=============================================================================



*.  %llu = C print format for unsigned int 64 = unsigned long long:

e.g.

log_verbose("d = %d  seg_id = %llu  lane_info_id = %d  lane_id = %d \n", 
		  d, xdm_id->seg_id, xdm_id->lane_info_idx, xdm_id->lane_idx);

*. sprintf syntax to specify zero-padding of integers so that they contain
exactly 4 digits:

 sprintf(image_chip_filename, "chip_%s_%04d.jpg", curr_annot_ebbox->label, image_chip_counter++);


*. printf syntax for specifying two digits to display after decimal point
in floating point numbers:

  printf("%s : elapsed runtime =  %.2f sec = %.2f mins = %.2f hours \n", banner, elapsed_time_in_secs,
	 elapsed_time_in_mins, elapsed_time_in_hours);

*.  In order to print pointer addresses, chant

	printf("pointer = %p \n", ptr);

*.  %d and %i may be used interchangeably as integer format specifiers for printf.

*.  Recall following typedefs in p_image.h:


		typedef p_image p_image_yv12[3];
		typedef p_image p_image_rgb[3];

When passing p_image_yv12 and p_image_rgb arrays as method arguments,
always treat them as p_image pointers:


void p_refine_coarse_regions_via_flood_filling(int display_output_flag,
					       p_image *im_yv12, p_image *init_rgb_image, p_image *rgb_image,
					       double global_mu_threshold, double local_mu_threshold,
					       double global_sigma_threshold, double local_sigma_threshold,
					       g_pos2_list_t* seeds, p_image *refined_segmentation_mask)

*.  Use strstr to test for one string being contained inside another as a substring:

const char * strstr ( const char * str1, const char * str2 );
      char * strstr (       char * str1, const char * str2 );

Returns a pointer to the first occurrence of str2 in str1, or a null pointer if str2 is not part of str1.


*.  Use strlen to get string length

size_t strlen ( const char * str );

The length of a C string is determined by the terminating null-character: A
C string is as long as the number of characters between the beginning of
the string and the terminating null character (without including the
terminating null character itself).


*.  Use sprintf to set character strings:

	 sprintf(test_string,"/foo/bar/blah/ick");

*.  Use sprintf() to copy one Cstring into another:

	sprintf(char* dst, "%s", char* src);

*.  Can use strcat() to concatenate one Cstring onto another:

	char * strcat ( char * destination, const char * source )

Appends a copy of the source string to the destination string. The
terminating null character in destination is overwritten by the first
character of source, and a null-character is included at the end of the new
string formed by the concatenation of both in destination.

destination and source shall not overlap.


*.  Use strcmp to compare two character strings:

	int strcmp ( const char * str1, const char * str2 );

Returns an integral value indicating the relationship between the strings:

A zero value indicates that both strings are equal.

A value greater than zero indicates that the first character that does not
match has a greater value in str1 than in str2.

A value less than zero indicates the opposite.


*.  Use sprintf to concatenate two strings and assign the result to a 3rd string:

  char quantized_color_name[MAX_PATH], curr_hue[MAX_PATH];
  sprintf(curr_hue,"red");
  sprintf(quantized_color_name, "%s%s" , "dark", curr_hue);
  log_verbose("quantized_color_name = %s \n", quantized_color_name);

*.  Any C function which is only called from within a .c file in which it
is defined should be declared as "static".  Such static functions are
similar in spirit to C++ private methods.  Static functions should NOT be
incorporated into any header files.

*.  In order to initialize all elements within an array declared on the
stack to have value val, chant something like

   int neighbor_values[9] = { [0 ... 8] = val};
  

*.  Can execute a unix system call from within a C program by chanting

	char unix_cmd[MAX_PATH];
	sprintf(unix_cmd, "ls -l %s \n", output_subsubdir);
	system(unix_cmd);

*.  In late Dec 2014, Manfred taught us that we should 
avoid unnecessary methods such as 

int cstr_char_to_ascii(char *input_char)
{
  return (int)(input_char[0]);
}

char cstr_ascii_to_char(int i)
{
  return (char)(i);
}

Instead, just perform casting directly, e.g.

	curr_char = cstr_ascii_to_char(48 + curr_digit) 	--->

	curr_char = (char)(48 + curr_digit);
