#include "qtvr.h"  

VALUE mQTVR = Qnil;
VALUE cMovie = Qnil;

static VALUE rbMovieAttrs_alloc(VALUE klass){
  rbMovieAttrs *attrs = malloc(sizeof(rbMovieAttrs));
  attrs->movie = NULL;
  return Data_Wrap_Struct(klass, 0, free, attrs);
}

void Init_qtvr(){
  // QTVR class
  mQTVR = rb_define_module("QTVR");
  rb_define_module_function(mQTVR, "qt_version", rb_GetQTVersion,0);

  // QTVR::Movie class
  cMovie = rb_define_class_under(mQTVR, "Movie", rb_cObject);
  rb_define_alloc_func(cMovie, rbMovieAttrs_alloc);
  rb_define_method(cMovie, "initialize", Movie_initialize, 1);
  rb_define_method(cMovie, "filename", get_filename, 0);
  rb_define_method(cMovie, "filename=", set_filename, 1);
  rb_define_method(cMovie, "active?", is_active, 0);
  rb_define_method(cMovie, "flatten", rb_FlattenMovieFile, 0);
}

static VALUE rb_GetQTVersion(VALUE self){
	long version = 0L;
	if(Gestalt(gestaltQuickTime, &version) == noErr){
    char version_str[8];
    sprintf(version_str, "%x", ((version >> 16) & 0xFFFF));
    return INT2FIX(atoi(version_str));
  }else{
    rb_raise(rb_eRuntimeError, "Cannot determine QuickTime version");
  }
}

static VALUE rb_FlattenMovieFile(VALUE self){ //, Movie theMovie, FSSpec *theFile)
	OSErr 		    err = noErr;
	FSRef 		    tempFile_fsref;
  FSSpec        tempFile;
  // UniChar       tempFileName[255] = "atempfilename";
  char          tempFileNameC[255];
  CFStringRef   tempFileName;// = CFSTR("atempfilename");
	char          error_msg[200];
  rbMovieAttrs* mov;
  Data_Get_Struct(self, rbMovieAttrs, mov);
	
  // DebugAssert(mov->movie != NULL); if(mov->movie == NULL) return invalidMovie;
	
	// Create the needed temp file.
  sprintf(tempFileNameC, "flattened_tmpfile_%d", TickCount());
  // printf("\ntempFileNameC: %s\n", tempFileNameC);//CFStringGetCStringPtr(tempFileName, CFStringGetSystemEncoding()));
  tempFileName = CFStringCreateWithCString(NULL,tempFileNameC, CFStringGetSystemEncoding());
  // printf("\ntempFileName: %s\n", CFStringGetCStringPtr(tempFileName, CFStringGetSystemEncoding()));
  // FSCatalogInfo catInfo;
  FSRef folder;
  if(err = FSGetCatalogInfo(&mov->fsref, kFSCatInfoNone, NULL, NULL, NULL, &folder)){
    sprintf(error_msg, "Cannot get reference to the movies containing folder. Error: %d", err);
    rb_raise(rb_eRuntimeError, error_msg);
  }

  err = FSMakeFSRefUnicode(&folder, 255, (UniChar*) tempFileName, kTextEncodingUnknown, &tempFile_fsref);
  if(err != fnfErr){
    sprintf(error_msg, "Cannot create reference to temp file \"%s\". Error: %d", CFStringGetCStringPtr(tempFileName, CFStringGetSystemEncoding()), err);
    rb_raise(rb_eRuntimeError, error_msg);
  }
    
  FSCatalogInfo catInfo;
  if(err = FSGetCatalogInfo(&tempFile_fsref, kFSCatInfoNone, NULL, NULL, &tempFile, NULL)){
    sprintf(error_msg, "Cannot convert temp files FSRef to FSSpec. Error: %d", err);
    rb_raise(rb_eRuntimeError, error_msg);
  }
  
  // if(err != fnfErr){
  //     sprintf(error_msg, "Cannot create reference to temp file \"%s\". Error: %d", CFStringGetCStringPtr(tempFileName, CFStringGetSystemEncoding()), err);
  //     rb_raise(rb_eRuntimeError, error_msg);
  // }
	
  printf("HAVE tempFile FSSpec: %d", tempFile);
	
	// Flatten the movie.
  // FlattenMovie(mov->movie, flattenAddMovieToDataFork, &tempFile, QTVRFlattenerType, smSystemScript, createMovieFileDeleteCurFile, 0, NULL);
  // err = GetMoviesError();
  
	Movie dstMovie;
	SetMovieProgressProc(mov->movie, (MovieProgressUPP)-1, 0L);
	dstMovie = FlattenMovieData(mov->movie,
		flattenDontInterleaveFlatten | flattenAddMovieToDataFork | flattenForceMovieResourceBeforeMovieData,
		&tempFile, FOUR_CHAR_CODE('TVOD'),
		smSystemScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile
	);
  
  if ((dstMovie == NULL) || ((err = GetMoviesError()) == fnOpnErr)) {
		DeleteMovieFile(&tempFile);	/* I don't know why, but sometimes this is necessary */
		if (dstMovie != NULL)
			DisposeMovie(dstMovie);
		
    sprintf(error_msg, "Cannot flatten movie to temp file \"%s\". Error: %d", tempFileNameC, err);
    rb_raise(rb_eRuntimeError, error_msg);
	}
  
  // if(err != noErr){
  //   FSpDelete(&tempFile);   // remove the temp file
  //   sprintf(error_msg, "Cannot flatten movie to temp file \"%s\". Error: %d", tempFileNameC, err);
  //   rb_raise(rb_eRuntimeError, error_msg);
  // }
  // 
  // DisposeMovie(mov->movie);
  // anErr = FSpDelete(theFile);  ReturnIfError(anErr);
  // anErr = FSpRename(&tempFile, theFile->name); ReturnIfError(anErr);
	
	return Qtrue;
}
static VALUE Movie_initialize(VALUE self, VALUE filename){
  OSErr       err = 0;
  char        error_msg[100];
  OSType      myDataRefType;
  Handle      myDataRef = NULL;
  short       actualResId = DoTheRightThing;
    
  rb_iv_set(self, "@filename", filename);
  rbMovieAttrs* mov;
  Data_Get_Struct(self, rbMovieAttrs, mov);

  // Initialize MovieToolbox
  if ((err = EnterMovies())){
    sprintf(error_msg, "Could not initialize QuickTime. Error: %d\n", err);
    rb_raise(rb_eRuntimeError, error_msg);
  }
    
  // Convert movie path to CFString
  if(!(mov->inPth = CFStringCreateWithCString(NULL, RSTRING_PTR(filename), CFStringGetSystemEncoding()))){
    rb_raise(rb_eRuntimeError, "Could not get CFString from \"%s\"\n", RSTRING_PTR(filename));
  }
    
  // Get and store a FSRef to the file
  if (err = FSPathMakeRef((UInt8*) StringValuePtr(filename), &mov->fsref, false)){
    sprintf(error_msg, "Could not convert movie filename \"%s\" to a FSRef. Error: %d\n", StringValuePtr(filename), err);
    rb_raise(rb_eRuntimeError, error_msg);
  }

  // create the data reference
  if ((err = QTNewDataReferenceFromFullPathCFString(mov->inPth, kQTNativeDefaultPathStyle, 0, &myDataRef, &myDataRefType))){
    sprintf(error_msg, "Could not get DataRef from path \"%s\". Error: %d\n", RSTRING_PTR(filename), err);
    rb_raise(rb_eRuntimeError, error_msg);
  }
  
  // printf("MOVIE before: %d\n", mov->movie);
  
  // get the Movie
  if ((err = NewMovieFromDataRef(&mov->movie, newMovieDontInteractWithUser | newMovieDontAskUnresolvedDataRefs, &actualResId, myDataRef, myDataRefType))){
    sprintf(error_msg, "Could not get Movie from DataRef %d\n", err);
    rb_raise(rb_eRuntimeError, error_msg);
  }
  
  // dispose the data reference handle - we no longer need it
  DisposeHandle(myDataRef);
    
  // printf("MOVIE: %d\n", mov->movie);
  // printf("Active? %d\n", GetMovieActive(mov->movie));
  return self;
}

Movie get_movie(VALUE self){
  rbMovieAttrs* mov;
  Data_Get_Struct(self, rbMovieAttrs, mov);
  // printf("MOVIE %d\n", mov->movie);
  // return INT2FIX(mov->movie);
  return mov->movie;
}

static VALUE get_filename(VALUE self){
  rb_iv_get(self, "@filename");
}

static VALUE set_filename(VALUE self, VALUE filename){
  rb_iv_set(self, "@filename", filename);
}

static VALUE is_active(VALUE self){
  // int is_active;
  // rbMovieAttrs* mov;
  // Data_Get_Struct(self, rbMovieAttrs, mov);
  // is_active = GetMovieActive(mov->movie);
  
  if(GetMovieActive(get_movie(self)))
    return Qtrue;
  else
    return Qfalse;
}