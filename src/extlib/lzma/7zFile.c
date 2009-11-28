/* 7zFile.c -- File IO
2008-11-22 : Igor Pavlov : Public domain */

#include "7zFile.h"

#ifndef USE_WINDOWS_FILE

#include <errno.h>

#endif

#ifdef USE_WINDOWS_FILE

/*
   ReadFile and WriteFile functions in Windows have BUG:
   If you Read or Write 64MB or more (probably min_failure_size = 64MB - 32KB + 1)
   from/to Network file, it returns ERROR_NO_SYSTEM_RESOURCES
   (Insufficient system resources exist to complete the requested service).
   Probably in some version of Windows there are problems with other sizes:
   for 32 MB (maybe also for 16 MB).
   And message can be "Network connection was lost"
*/

#define kChunkSizeMax (1 << 22)

/** Unicode functions for Gens/GS. **/

#include <winnls.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#define MAKE_STFUNCPTR(f) static typeof(f) * p##f = NULL
static HMODULE hKernel32 = NULL;
MAKE_STFUNCPTR(CreateFileW);
MAKE_STFUNCPTR(MultiByteToWideChar);
static BOOL isUnicodeChecked = FALSE;
static BOOL isUnicodeAvailable = FALSE;

#define InitFuncPtr_lzma(hDll, fn) p##fn = (typeof(p##fn))GetProcAddress((hDll), #fn)

#endif

void File_Construct(CSzFile *p)
{
  #ifdef USE_WINDOWS_FILE
  p->handle = INVALID_HANDLE_VALUE;
  #else
  p->file = NULL;
  #endif
}

static WRes File_Open(CSzFile *p, const char *name, int writeMode)
{
#ifdef USE_WINDOWS_FILE
	// Check for Unicode.
	if (!isUnicodeChecked)
	{
		hKernel32 = LoadLibrary("kernel32.dll");
		if (hKernel32)
		{
			InitFuncPtr_lzma(hKernel32, CreateFileW);
			InitFuncPtr_lzma(hKernel32, MultiByteToWideChar);
			
			isUnicodeAvailable = (pCreateFileW && pMultiByteToWideChar);
			if (!isUnicodeAvailable)
			{
				// Unicode is not enabled.
				// Unload kernel32.dll and NULL out the pointers.
				FreeLibrary(hKernel32);
				hKernel32 = NULL;
				pCreateFileW = NULL;
				pMultiByteToWideChar = NULL;
			}
		}
		isUnicodeChecked = TRUE;
	}
	
	if (isUnicodeAvailable)
	{
		// Unicode is available.
		
		// Convert the name from UTF-8 to UTF-16.
		wchar_t *wname = NULL;
		int name_len = pMultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
		if (name_len > 0)
		{
			wname = (wchar_t*)malloc(name_len * sizeof(wchar_t));
			pMultiByteToWideChar(CP_UTF8, 0, name, -1, wname, name_len);
		}
		
		p->handle = pCreateFileW(wname,
				writeMode ? GENERIC_WRITE : GENERIC_READ,
				FILE_SHARE_READ, NULL,
				writeMode ? CREATE_ALWAYS : OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);
		
		free(wname);
	}
	else
	{
		// Unicode is not available. Use ANSI.
		p->handle = CreateFileA(name,
				writeMode ? GENERIC_WRITE : GENERIC_READ,
				FILE_SHARE_READ, NULL,
				writeMode ? CREATE_ALWAYS : OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);
	}
	
	return (p->handle != INVALID_HANDLE_VALUE) ? 0 : GetLastError();
#else
	p->file = fopen(name, writeMode ? "wb+" : "rb");
	return (p->file != 0) ? 0 : errno;
#endif
}

WRes InFile_Open(CSzFile *p, const char *name) { return File_Open(p, name, 0); }
WRes OutFile_Open(CSzFile *p, const char *name) { return File_Open(p, name, 1); }

WRes File_Close(CSzFile *p)
{
  #ifdef USE_WINDOWS_FILE
  if (p->handle != INVALID_HANDLE_VALUE)
  {
    if (!CloseHandle(p->handle))
      return GetLastError();
    p->handle = INVALID_HANDLE_VALUE;
  }
  #else
  if (p->file != NULL)
  {
    int res = fclose(p->file);
    if (res != 0)
      return res;
    p->file = NULL;
  }
  #endif
  return 0;
}

WRes File_Read(CSzFile *p, void *data, size_t *size)
{
  size_t originalSize = *size;
  if (originalSize == 0)
    return 0;

  #ifdef USE_WINDOWS_FILE

  *size = 0;
  do
  {
    DWORD curSize = (originalSize > kChunkSizeMax) ? kChunkSizeMax : (DWORD)originalSize;
    DWORD processed = 0;
    BOOL res = ReadFile(p->handle, data, curSize, &processed, NULL);
    data = (void *)((Byte *)data + processed);
    originalSize -= processed;
    *size += processed;
    if (!res)
      return GetLastError();
    if (processed == 0)
      break;
  }
  while (originalSize > 0);
  return 0;

  #else
  
  *size = fread(data, 1, originalSize, p->file);
  if (*size == originalSize)
    return 0;
  return ferror(p->file);
  
  #endif
}

WRes File_Write(CSzFile *p, const void *data, size_t *size)
{
  size_t originalSize = *size;
  if (originalSize == 0)
    return 0;
  
  #ifdef USE_WINDOWS_FILE

  *size = 0;
  do
  {
    DWORD curSize = (originalSize > kChunkSizeMax) ? kChunkSizeMax : (DWORD)originalSize;
    DWORD processed = 0;
    BOOL res = WriteFile(p->handle, data, curSize, &processed, NULL);
    data = (void *)((Byte *)data + processed);
    originalSize -= processed;
    *size += processed;
    if (!res)
      return GetLastError();
    if (processed == 0)
      break;
  }
  while (originalSize > 0);
  return 0;

  #else

  *size = fwrite(data, 1, originalSize, p->file);
  if (*size == originalSize)
    return 0;
  return ferror(p->file);
  
  #endif
}

WRes File_Seek(CSzFile *p, Int64 *pos, ESzSeek origin)
{
  #ifdef USE_WINDOWS_FILE

  LARGE_INTEGER value;
  DWORD moveMethod;
  value.LowPart = (DWORD)*pos;
  value.HighPart = (LONG)((UInt64)*pos >> 16 >> 16); /* for case when UInt64 is 32-bit only */
  switch (origin)
  {
    case SZ_SEEK_SET: moveMethod = FILE_BEGIN; break;
    case SZ_SEEK_CUR: moveMethod = FILE_CURRENT; break;
    case SZ_SEEK_END: moveMethod = FILE_END; break;
    default: return ERROR_INVALID_PARAMETER;
  }
  value.LowPart = SetFilePointer(p->handle, value.LowPart, &value.HighPart, moveMethod);
  if (value.LowPart == 0xFFFFFFFF)
  {
    WRes res = GetLastError();
    if (res != NO_ERROR)
      return res;
  }
  *pos = ((Int64)value.HighPart << 32) | value.LowPart;
  return 0;

  #else
  
  int moveMethod;
  int res;
  switch (origin)
  {
    case SZ_SEEK_SET: moveMethod = SEEK_SET; break;
    case SZ_SEEK_CUR: moveMethod = SEEK_CUR; break;
    case SZ_SEEK_END: moveMethod = SEEK_END; break;
    default: return 1;
  }
  res = fseek(p->file, (long)*pos, moveMethod);
  *pos = ftell(p->file);
  return res;
  
  #endif
}

WRes File_GetLength(CSzFile *p, UInt64 *length)
{
  #ifdef USE_WINDOWS_FILE
  
  DWORD sizeHigh;
  DWORD sizeLow = GetFileSize(p->handle, &sizeHigh);
  if (sizeLow == 0xFFFFFFFF)
  {
    DWORD res = GetLastError();
    if (res != NO_ERROR)
      return res;
  }
  *length = (((UInt64)sizeHigh) << 32) + sizeLow;
  return 0;
  
  #else
  
  long pos = ftell(p->file);
  int res = fseek(p->file, 0, SEEK_END);
  *length = ftell(p->file);
  fseek(p->file, pos, SEEK_SET);
  return res;
  
  #endif
}


/* ---------- FileSeqInStream ---------- */

static SRes FileSeqInStream_Read(void *pp, void *buf, size_t *size)
{
  CFileSeqInStream *p = (CFileSeqInStream *)pp;
  return File_Read(&p->file, buf, size) == 0 ? SZ_OK : SZ_ERROR_READ;
}

void FileSeqInStream_CreateVTable(CFileSeqInStream *p)
{
  p->s.Read = FileSeqInStream_Read;
}


/* ---------- FileInStream ---------- */

static SRes FileInStream_Read(void *pp, void *buf, size_t *size)
{
  CFileInStream *p = (CFileInStream *)pp;
  return (File_Read(&p->file, buf, size) == 0) ? SZ_OK : SZ_ERROR_READ;
}

static SRes FileInStream_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
  CFileInStream *p = (CFileInStream *)pp;
  return File_Seek(&p->file, pos, origin);
}

void FileInStream_CreateVTable(CFileInStream *p)
{
  p->s.Read = FileInStream_Read;
  p->s.Seek = FileInStream_Seek;
}


/* ---------- FileOutStream ---------- */

static size_t FileOutStream_Write(void *pp, const void *data, size_t size)
{
  CFileOutStream *p = (CFileOutStream *)pp;
  File_Write(&p->file, data, &size);
  return size;
}

void FileOutStream_CreateVTable(CFileOutStream *p)
{
  p->s.Write = FileOutStream_Write;
}
