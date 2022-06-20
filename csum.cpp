// -------------------------------------------------
// CRC32 CheckSum (32 for check sum calculation, you can build it for 32 or 64 bits) 
// Standard calculation of CRC32, same values as in PKZIP (for example)
// Windows.h is used only to scan folder for filenames, the rest is plain C
// Usage: csum *.c *.cpp
// Well known CRC32, this code partially based on ZLIB code written by Mark Adler 
// -------------------------------------------------
#include "stdio.h"
#include "stdlib.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

typedef unsigned char UINT8;
typedef unsigned int  UINT32;

class CDirCsum {
private:  // data:
   FILE*    m_fpOut;
   UINT32   m_CrcTable[256];
   UINT32   m_nTotalCrc;
   UINT32   m_nTotalLng;
   UINT32   m_nTotalFiles;

public:
   CDirCsum(FILE* fpOut);
   ~CDirCsum();
   int  scanName(const char* pName);
   void total();

private:
   void   makeCrcTable();
   UINT32 csumFile(const char* pFileName, const char* pPath);
};

int main(int argc, char** argv) {
   if (argc < 2) {
      printf("Usage: csum name[s]\n");
		return 1;
	}

   CDirCsum crcObj(stdout);
   printf("CRC32        Length  Filename:\n"
          "------------------------------\n");

   int j = 1;
   for (; j < argc; ++j) {
      crcObj.scanName( argv[j] );
   }
   crcObj.total();
	return 0;
}  /* end main() */

CDirCsum::CDirCsum(FILE* fpOut) {
   m_fpOut = fpOut;
   m_nTotalCrc = 0;
   m_nTotalLng = 0;
   m_nTotalFiles = 0;
   makeCrcTable();
}

CDirCsum::~CDirCsum() {
}

// scan for last delimiter
static void getPath(char* pPath, const char* pFull) {
  const char* pPos = pFull + strlen(pFull);
  while (pPos > pFull) {
     if (*pPos == '\\' || *pPos == '/')
         break;
     --pPos;
  }

  pPath[0] = 0;
  if (pPos > pFull) { // delimiter found
      size_t nLng = pPos - pFull + 1;
      memcpy(pPath, pFull, nLng);
      pPath[nLng] = 0;
  }
}

// name argument wildcard-extend, Win32:
int CDirCsum::scanName(const char* pName) {
   WIN32_FIND_DATA wfd;
   HANDLE          hFile;
   DWORD           err;

   hFile = ::FindFirstFile(pName, &wfd);
   if (hFile == INVALID_HANDLE_VALUE) {
       err = ::GetLastError();
       fprintf(m_fpOut, "Can't open file, err=%08X\n",
                    err);
       return 1;
   }

   char  pPath[_MAX_PATH + 16];
   getPath(pPath, pName);
   do {  // first/next file info:
     if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue;
     csumFile(wfd.cFileName, pPath);
   } while ( FindNextFile((HANDLE)hFile, &wfd) );


   FindClose(hFile);
   return 0;
} /* end scanName() */


UINT32 CDirCsum::csumFile(const char* pFileName, const char* pPath) {
   int nByte;
   char pFullName[_MAX_PATH + 16];
   strcpy(pFullName, pPath);
   strcat(pFullName, pFileName);

   FILE* fpIn = fopen(pFullName, "rb");
   if (fpIn == NULL)  {
      fprintf(m_fpOut, "       file %s open error\n",
              pFullName);
      return 0;
   }
   UINT32 crc = 0xFFFFFFFFL;
   UINT32 nLng = 0;
   while ( (nByte = fgetc(fpIn)) != EOF ) {
      nByte ^= crc & 0xFF;
      crc >>= 8;
      crc ^= m_CrcTable[nByte];
      ++nLng;
   }

   fclose(fpIn);
   crc ^= 0xFFFFFFFFL;
   fprintf(m_fpOut, "%08X %9d %s\n" ,
            crc, nLng, pFullName);

   // for the "total":
   m_nTotalCrc ^= crc;
   m_nTotalLng += nLng;
   m_nTotalFiles++;
   return crc;
}

void CDirCsum::total() {
   fprintf(m_fpOut, "------------------------------\n"
                    "%08X %9d bytes in %d file[s]\n" ,
                  m_nTotalCrc, m_nTotalLng, m_nTotalFiles);
}

// ------------------------------------------------------------------------
// Generate a table for a byte-wise 32-bit CRC calculation on the polynomial:
//   x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.
// Polynomials over GF(2) are represented in binary, one bit per coefficient,
// with the lowest powers in the most significant bit.
// Then adding polynomials is just exclusive-or, and multiplying
// a polynomial by x is a right shift by one.
//  If we call the above polynomial p, and represent a byte as the
// polynomial q, also with the lowest power in the most significant bit
// (so the byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is
// (q*x^32) mod p, where a mod b means the remainder after dividing a by b.
// This calculation is done using the shift-register method of multiplying
// and taking the remainder.  The register is initialized to zero, and for
// each incoming bit, x^32 is added mod p to the register if the bit is a
// one (where x^32 mod p is p+x^32 = x^26+...+1), and the register is
// multiplied mod p by  x (which is shifting right by one and adding
// x^32 mod p if the bit shifted out is a one).  We start with the highest
// power (least significant bit) of q and repeat for all eight bits of q.
//  The table is simply the CRC of all possible eight bit values.  This
// is all the information needed to generate CRC's on data a byte at a time
// for all combinations of CRC register values and incoming bytes.
// ------------------------------------------------------------------------
void CDirCsum::makeCrcTable() {  // [256]
  UINT32 c;
  int n, k;
  UINT32 poly = 0L;  /* polynomial exclusive-or pattern */
  /* terms of polynomial defining this crc (except x^32): */
  static const UINT8 p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

  /* make exclusive-or pattern from polynomial (0xedb88320L) */
  for (n = 0; n < sizeof(p)/sizeof(UINT8); n++)
      poly |= 1L << (31 - p[n]);

  for (n = 0; n < 256; n++)  {
    c = (UINT32)n;
    for (k = 0; k < 8; k++)
         c = c & 1 ? poly ^ (c >> 1) : c >> 1;
    m_CrcTable[n] = c;   // [0 - 255]
  }
}

/* eof */
