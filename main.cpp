

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "TImageScaler.h"
#include "time.h"

//---------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // validate usage
    if(argc != 5)
    {
        std::cout << "Usage: " << std::endl << "Scaler.exe [Source Image] [Destination Image] [Size to Scale] [Quality to Compress]" << std::endl << std::endl;
        return 1;
    }
    // parse parameters
    std::string ImageIn, ImageOut;
    int SizeScale, Quality;
    ImageIn = argv[1];
    ImageOut = argv[2];
    SizeScale = std::atoi(argv[3]);
    Quality = std::atoi(argv[4]);
    // start timer
    int times = clock();
    // import header from source
    TImageScaler image;
    image.LoadHeader(ImageIn.c_str());
    const int width = image.GetWidth();
    const int height = image.GetHeight();
    int newWidth, newHeight;
    if(width > height)
    {
        newWidth = SizeScale;
        newHeight = height * newWidth / width;
    }
    else
    {
        newHeight = SizeScale;
        newWidth = width * newHeight / height;
    }
    float tx = (float)newWidth / width;
    float ty = (float)newHeight / height;
    std::printf("scaling from %d x %d to %d x %d...\n",width,height,newWidth,newHeight);
    // lets chunck
    int nX = 0;
    int nY = 0;
    const int maxSize = 1000;  // throttle the memory alloc
    int stepX = maxSize;
    int stepY = maxSize;
    // set divisons
    nX = width / stepX;
    while(nX < 2)
    {
        stepX++;
        nX = width / stepX;
    }

    nY = height / stepY;
    while(nY < 2)
    {
        stepY++;
        nY = height / stepY;
    }
    // reallocate memory to the new size
    image.SetWidth(newWidth);
    image.SetHeight(newHeight);
    image.Reallocate();
    unsigned int x0,y0,x1,y1;
    x0 = 1;
    x1 = stepX;
    int offsetX = 0;
    // chunck, resize, add resized chuncks to the new image
    for(int i = 1; i <= nX; i++)
    {
        if(i == nX) x1 = width;
        y0 = 1;
        y1 = stepY;
        int offsetY = 0;
        int stepoffsetX = 0;
        for(int j = 1; j <= nY; j++)
        {
            if(j == nY) y1 = height;
            std::printf("%d/%d,%d/%d = (%d,%d) - (%d,%d)        \r",i,nX,j,nY,x0,y0,x1,y1);
            TImageScaler chunk;
            chunk.LoadData(ImageIn.c_str(),x0,x1,y0,y1);
            //char nameOut[255];
            //std::sprintf(nameOut,"i=%d,j=%d - uncompress.jpeg",i,j);
            //chunk.Save(nameOut,90);
            chunk.Scale(tx,ty);
            for(unsigned int x = 0; x <= chunk.GetWidth(); x++)
            {
                for(unsigned int y = 0; y <= chunk.GetHeight(); y++)
                {
                    for(unsigned int c = 0; c < image.GetComponents(); c++)
                    {
                        unsigned char p = chunk.GetFromPixel(x,y,c);
                        image.CopyToPixel(x + offsetX,y + offsetY,c,p);
                    }
                }
            }
            offsetY += chunk.GetHeight();
            stepoffsetX = chunk.GetWidth();
            //std::sprintf(nameOut,"i=%d,j=%d - compress.jpeg",i,j);
            //chunk.Save(nameOut,90);
            y0 += stepY;
            y1 += stepY;
        }
        x0 += stepX;
        x1 += stepX;
        offsetX += stepoffsetX;
    }
    // save final image and exit
    image.Save(ImageOut.c_str(),Quality);
    int timed = clock();
    std::printf("Done!\nTime to execute: %ld ms\n\n",(timed-times)/(CLOCKS_PER_SEC/1000));
    std::cin.ignore();
    return 0;
}
