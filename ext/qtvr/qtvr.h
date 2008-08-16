#include <QuickTime.h>
#include <Endian.h>
#include <FixMath.h>
#include <stdio.h>
#include <string.h>
#if TARGET_OS_MAC
# include <fp.h>
#else /* !TARGET_OS_MAC */
# include <math.h>
#endif /* !TARGET_OS_MAC */
#include "ruby.h"

//
// MACROS
//
#ifndef RARRAY_LEN
#define RARRAY_LEN(arr)  RARRAY(arr)->len
#define RSTRING_LEN(str) RSTRING(str)->len
#define RSTRING_PTR(str) RSTRING(str)->ptr
#endif

#if DEBUG
static char gDebugString[256];

#define DebugAssert(condition)																		\
		if (condition)		NULL;																			\
	else 																											\
	{																												\
		sprintf(gDebugString,"File: %s, Line: %d", __FILE__, __LINE__);			\
		DebugStr(c2pstr(gDebugString));															\
	}
#else
#define DebugAssert(condition)		NULL
#endif


enum {
	QTVRFlattenerType = FOUR_CHAR_CODE('vrwe')	/* An oversight prevented this from going into QuickTimeVRFormat.h */
};

// 
// C struct that keep track of the QT data
// 
typedef struct rbMovieAttrs{
  Movie movie;
  CFStringRef inPth;
  } rbMovieAttrs;

//
// RUBY METHODS
// 
void Init_qtvr();
static VALUE rb_GetQTVersion(VALUE self);
static VALUE Movie_initialize(VALUE self, VALUE filename);
static VALUE rb_FlattenMovieFile(VALUE self);
static VALUE get_filename(VALUE self);
static VALUE set_filename(VALUE self, VALUE filename);
static VALUE is_active(VALUE self);

//
// C functions
//
Movie get_movie(VALUE self);
