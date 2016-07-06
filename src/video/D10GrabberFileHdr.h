// ========================================================================
// Header for D10GrabberFileHdr type 
// ========================================================================
// Last updated on 8/12/05
// ========================================================================

#ifndef D10GRABBERFILEHDR_H
#define D10GRABBERFILEHDR_H

typedef union tD10GrabberFileHdr
{

// The following byte array ensures this raw Group 99 video image
// header structure consumes 4096 bytes:

      char byHdr[4096];

      struct {
            short		major_version;
            short		minor_version;
            short		bits_per_pixel;
            char		operat[20];
            short		chipnum;
            int			ext_hdr_len;
            char		filler[28];
            char		start_time[23];
            char		end_time[23];
            unsigned short		row_start;
            unsigned short 		row_length;
            unsigned short		col_start;
            unsigned short		col_length;
            unsigned int	frames_requested;
            unsigned int	frames_good;
            short		big_little;
            short		frame_type;	   // TCMP3 type = 3
            unsigned long		dwBytesPerImgDiv10;		
		// Bytes per image (NOT including extra info)
            unsigned short		wBytesPerImgExtraDiv10;	
		// Number of extra bytes following each image

// Note: Total number of bytes stored for each image within raw video
// file = dwBytesPerImgDiv10 + wBytesPerImgExtraDiv10.

      };
};

#endif // D10GRABBERFILEHDR.h
