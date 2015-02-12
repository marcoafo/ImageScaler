#ifndef TImageScaler_H
#define TImageScaler_H

#include "libjpeg/jpeglib.h"
#include <setjmp.h>
#include <vector>
#include <map>

//---------------------------------------------------------------------

/*!
 * \brief Read, scale and write image.
 *
 * Uses only bicubic method, as described in http://www.paulinternet.nl/?page=bicubic .
 * Image is stored in pure bitmap in class, but the calculation is just inside
 * the function. Could be improved, but I have only like two hours left to deliver. :D
 *
 * Also, the error handling is poorly made, and used with stdio. I could have done
 * a better job in that.
 */
class TImageScaler
{
private:
    typedef unsigned char* points;
    typedef unsigned int uint;

	uint Width;    /*!< Width of image, as exported by libjpeg. */
	uint Height;   /*!< Height of image, as exported by libjpeg. */
	uint NumComp;  /*!< Number of colorset components. */
	points Data;   /*!< Number of colorset components. */

	void Clean();
	float GetPixel(uint X, uint Y, uint Component);

public:
	TImageScaler();
	virtual ~TImageScaler();

	unsigned int GetWidth();
	unsigned int GetHeight();
	unsigned int GetComponents();
	void SetWidth(unsigned int NewWidth);
	void SetHeight(unsigned int NewHeight);
	void Reallocate();
	void CopyToPixel(uint X, uint Y, uint Component, unsigned char Pixel);
	unsigned char GetFromPixel(uint X, uint Y, uint Component);

    void LoadHeader(const char* FileName);
	void LoadData(const char* FileName,uint Width0 = 0, uint Width1 = 0, uint Height0 = 0, uint Height1 = 0);
	void Save(const char* FileName, int Quality);
	void Scale(float RateWidth, float RateHeight);

};

//---------------------------------------------------------------------

#endif
