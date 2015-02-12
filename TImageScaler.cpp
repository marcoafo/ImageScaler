#include <cstdlib>
#include <cstdio>

#include "TImageScaler.h"

//---------------------------------------------------------------------

/*!
 * \brief Clear all containers and reset members.
 */
void TImageScaler::Clean()
{
    Width = 0;
    Height = 0;
    NumComp = 0;
    if(Data != NULL) delete [] Data;
    Data = NULL;
}

/*!
 * \brief Get a pixel value from the image.
 * \param Y Position in height.
 * \param X Position in width.
 * \param Component Component (channel) to be used.
 * \return The color in float value of the position/channel, or 0 if not found.
 */
float TImageScaler::GetPixel(uint Y, uint X, uint Component)
{
    if(X >= Width || Y >= Height) return 0;
    unsigned char c = Data[(Y * NumComp * Width) + (X * NumComp) + Component];
    return (float)c;
}

/*!
 * \brief Convert to unsigned char a float value of a pixel, making sure it's inside 0f to 255f.
 * \param Pixel Value to be converted.
 * \return An unsigned char from 0f to 255f, being the value casted.
 */
inline unsigned char Saturate(float Pixel)
{
    if(Pixel > 255.0f) return 255;
    if(Pixel < 0.0f) return 0;
    return (unsigned char)Pixel;
}

//---------------------------------------------------------------------

TImageScaler::TImageScaler()
{
    Width = 0;
    Height = 0;
    NumComp = 0;
    Data = NULL;
}

TImageScaler::~TImageScaler()
{
	Clean();
}

unsigned int TImageScaler::GetWidth() { return Width; }

unsigned int TImageScaler::GetHeight() { return Height; }

unsigned int TImageScaler::GetComponents() { return NumComp; }

void TImageScaler::SetWidth(unsigned int NewWidth) { this->Width = NewWidth; }

void TImageScaler::SetHeight(unsigned int NewHeight) { this->Height = NewHeight; }

void TImageScaler::Reallocate()
{
    if(Data != NULL) delete [] Data;
    Data = (unsigned char *)std::malloc(Width * Height * NumComp);
}

void TImageScaler::CopyToPixel(uint X, uint Y, uint Component, unsigned char Pixel)
{
    if(X >= Width || Y >= Height) return;
    Data[(Y * NumComp * Width) + (X * NumComp) + Component] = Pixel;
}

unsigned char TImageScaler::GetFromPixel(uint X, uint Y, uint Component)
{
    if(X >= Width || Y >= Height) return 0;
    unsigned char c = Data[(Y * NumComp * Width) + (X * NumComp) + Component];
    return c;
}

//---------------------------------------------------------------------

/*!
 * \brief Decompress and load image in memory.
 * \param FileName Path to file to be loaded.
 */
void TImageScaler::LoadHeader(const char* FileName)
{
    // try to open the file
	FILE* infile = fopen(FileName, "rb");
	if (!infile) return;
	// structs to read the jpeg
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    // let's set default stuff, create decompress, etc
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    // store headers value
    Width = cinfo.image_width;
    Height = cinfo.image_height;
    NumComp = cinfo.num_components;
    fclose(infile);
}

/*!
 * \brief Decompress and load image in memory.
 * \param FileName Path to file to be loaded.
 */
void TImageScaler::LoadData(const char* FileName,uint Width0, uint Width1, uint Height0, uint Height1)
{
    if(Data != NULL) delete Data;

    // try to open the file
	FILE* infile = fopen(FileName, "rb");
	if (!infile) return;
	// structs to read the jpeg
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    // let's set default stuff, create decompress, etc
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    // store headers value
    Width = cinfo.image_width;
    Height = cinfo.image_height;
    NumComp = cinfo.num_components;
    // start it, allocate memory
    jpeg_start_decompress(&cinfo);
    // get bitmap

    if(Width0 == 0 && Width1 == 0)
    {
        Width0 = 1;
        Width1 = Width;
    }
    if(Height0 == 0 && Height1 == 0)
    {
        Height0 = 1;
        Height1 = Height;
    }
    Width = Width1 - Width0 + 1;
    Height = Height1 - Height0 + 1;

    Data = (unsigned char *)std::malloc(Width * Height * NumComp);
    JSAMPROW row_pointer[1];  // pointer of line read by libjpeg
    row_pointer[0] = (unsigned char *)malloc(cinfo.output_width * cinfo.num_components);  // allocate memory for one line

    uint k = 0;
    for(uint j = 0; j < cinfo.image_height; j++)  // read all lines
    {
        jpeg_read_scanlines(&cinfo,row_pointer,1);  // scan the line, toss it in the vector
        if((j+1) < Height0 || (j+1) > Height1) continue;
        for(uint i = 0; i < cinfo.image_width; i++)
        {
            if((i+1) < Width0 || (i+1) > Width1) continue;
            for(int c = 0; c < cinfo.num_components; c++)  // read all points in line, toss in the bitmap vector
            {
                uint k1 = i * cinfo.num_components + c;
                Data[k] = row_pointer[0][k1];
                k++;

            }
        }
    }

    // close and destroy
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    fclose(infile);
}


/*!
 * \brief Compress image and save to JPEG file.
 * \param FileName Path to file to be saved.
 * \param Quality Quality of the image to be saved (from 0 to 100).
 */
void TImageScaler::Save(const char* FileName, int Quality)
{
    // try to open the file
    FILE* outfile = fopen(FileName, "wb");
	if (!outfile) return;
    // struct to compress image to jpeg
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    // initializate, create compress and set error messages
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    // set image parameters
    cinfo.image_width = Width;
    cinfo.image_height = Height;
    cinfo.input_components = NumComp;
    cinfo.in_color_space = JCS_RGB;  // just in case ;-)
    jpeg_set_defaults(&cinfo);
    // set quality and start compress
    jpeg_set_quality (&cinfo, Quality, true);
    jpeg_start_compress(&cinfo, true);
      // pointer to a row
    JSAMPROW row_pointer;  // pointer to a row
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer = (JSAMPROW)&Data[cinfo.next_scanline * cinfo.image_width * cinfo.num_components];  // just drop the line of bitmap directly, so I only need a pointer to the line
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    // close and destroy, we are finished
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
}

/*!
 * \brief Scale image to the desired side, using a bicubic method.
 * \param Size Desired new size of the greater side of the image in px.
 */
void TImageScaler::Scale(float RateWidth, float RateHeight)
{
    // sanity check
    if(Data == NULL) return;
    // determine width and height (I won't deal here with float and integer division and approximation, since the error for rounding is irrelevant
    int newWidth = RateWidth * Width;
    int newHeight = RateHeight * Height;
    // printf("%d x %d  =>  %d x %d \n",Width, Height,newWidth,newHeight);
    // create new container with the new width x height
    unsigned int newSize = newWidth * newHeight * NumComp;
    unsigned char *newData = (unsigned char *)std::malloc(newSize);
    // calcuate the tax rates for x and y
    float tx = (float)Width/newWidth;
    float ty = (float)Height/newHeight;
    // bicubic matrix
    float C[5] = {0};  // I hate this dennotation...
    // help with the raw vector
    const unsigned int rowStride = newWidth * NumComp;
    // let's go for it, pay attention in width and height (I have mistaked it
    for(int i = 0; i < newHeight; i++)
    {
        for(int j = 0; j < newWidth; j++)
        {
            //printf("%d %d     \r",i,j);
            // we will use float to avoid rouding and conversion problems
            float x = float(tx * j);
            float y = float(ty * i);
            float dx = tx * j - x;
            float dy = ty * i - y;
            // don't touch the channels
            for(unsigned k = 0; k < NumComp; k++)
            {
                //printf("%d %d %d\n",i,j,k);
                for(int jj = 0; jj <= 3; jj++)
                {
                    float d0, d2, d3, a0, a1, a2, a3;
                    a0 = GetPixel(y-1+jj,x,k);
                    d0 = GetPixel(y-1+jj,x-1,k) - a0;
                    d2 = GetPixel(y-1+jj,x+1,k) - a0;
                    d3 = GetPixel(y-1+jj,x+2,k) - a0;
                    a1 = -1.0f / 3.0f * d0 + d2 - 1.0f / 6.0f * d3;
                    a2 = 1.0f / 2.0f * d0 + 1.0f / 2.0f * d2;
                    a3 = -1.0f / 6.0f * d0 - 1.0f / 2.0f * d2 + 1.0f / 6.0f * d3;
                    C[jj] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;
                    d0 = C[0] - C[1];
                    d2 = C[2] - C[1];
                    d3 = C[3] - C[1];
                    a0 = C[1];
                    a1 = -1.0f / 3.0f * d0 + d2 -1.0f / 6.0f * d3;
                    a2 = 1.0f / 2.0f * d0 + 1.0f / 2.0f * d2;
                    a3 = -1.0f / 6.0f * d0 - 1.0f / 2.0f * d2 + 1.0f / 6.0f * d3;
                    newData[i * rowStride + j * NumComp + k] = Saturate(a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy);  // saturate or fail!
                }
            }
        }
    }
    // ok, now delete the old, assign new
    Width = newWidth;
    Height = newHeight;
    delete [] Data;
    Data = newData;
}

