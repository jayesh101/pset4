#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    /* remember filenames and enlargement factor,
        converting value of n to a single integer */
    int efactor = atoi (argv[1]); 
    char *infile = argv[2];
    char *outfile = argv[3];
    
    if (efactor < 1 || efactor > 100)
    {
        fprintf(stderr, "Error! range of n must be = [1,100]\n ");
        return 2;
    }
    

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 3;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 4;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 5;
    }
    
    // preserving old dimensions 
    LONG biWidth_inp = bi.biWidth;
    LONG biHeight_inp = bi.biHeight;
    
    // new dimensions
    bi.biWidth = efactor * bi.biWidth;
    bi.biHeight = efactor * bi.biHeight;
     
    
    // determine padding for scanlines for input and output file
    int padding_inp = (4 - (biWidth_inp * sizeof(RGBTRIPLE)) % 4) % 4;
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    bi.biSizeImage = ((bi.biWidth * sizeof (RGBTRIPLE)) + padding) * abs(bi.biHeight);
    bf.bfSize = bi.biSizeImage + sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER);
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // allocate memory for temp to hold scanline
    RGBTRIPLE *temp = malloc(sizeof(RGBTRIPLE) * (bi.biWidth));

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(biHeight_inp); i < biHeight; i++)
    {
        // sets counter to 0 after each row
        int counter = 0;
        
        // iterate over pixels in scanline
        for (int j = 0; j < biWidth_inp; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
             // for writing pixel to temp n times
            for(int inc = 0; inc < efactor; inc++)
            {
                *(temp + counter) = triple;
                counter++;
            }
        }
        
         // skip over padding, if any
        fseek(inptr, padding_inp, SEEK_CUR);
        
        // write RGB triple to outfile
        for(int inc = 0; inc < efactor; inc++)
        {
            // write RGB triple to outfile
            fwrite(temp, sizeof(RGBTRIPLE), bi.biWidth, outptr);
           
            // then add padding back (to demonstrate how)
            for (int k = 0; k < padding; k++)
            {
                fputc(0x00, outptr);
            }
        }
    } 
    
    // free memory from temp
    free(temp);

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
