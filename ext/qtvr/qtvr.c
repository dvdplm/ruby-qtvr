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
/*
static VALUE rb_FlattenMovieFile(VALUE self){ //, Movie theMovie, FSSpec *theFile)
	OSErr 		    anErr = noErr;
	FSSpec 		    tempFile;
	char          tempFileName[255];
	char          error_msg[100];
  rbMovieAttrs* mov;
  Data_Get_Struct(self, rbMovieAttrs, mov);
	
	DebugAssert(mov->movie != NULL); if(mov->movie == NULL) return invalidMovie;
	
	// Create the needed temp file.
	// NumToString(TickCount(), tempFileName);
	numtostring(TickCount(), tempFileName);
  anErr = FSMakeFSSpec(tempFile->vRefNum, tempFile->parID, tempFileName, &tempFile);
  
  extern OSErr  FSMakeFSRefUnicode(const FSRef *parentRef, UniCharCount nameLength, const UniChar *name, TextEncoding textEncodingHint, FSRef *newRef) AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER;
  
  
	if(anErr != fnfErr){
    sprintf(error_msg, "Cannot create reference to temp file \"%s\". Error: %d", tempFileName, anErr);
    rb_raise(rb_eRuntimeError, error_msg);
	}
	
	// Flatten the movie.
	FlattenMovie(mov->movie, flattenAddMovieToDataFork, &tempFile, QTVRFlattenerType, smSystemScript, createMovieFileDeleteCurFile, 0, NULL);
	anErr = GetMoviesError();
	if(anErr != noErr){
		FSpDelete(&tempFile);		// remove the temp file
    sprintf(error_msg, "Cannot flatten movie. Error: %d", tempFileName, anErr);
    rb_raise(rb_eRuntimeError, error_msg);
	}
	
	DisposeMovie(mov->movie);
  // anErr = FSpDelete(theFile);  ReturnIfError(anErr);
  // anErr = FSpRename(&tempFile, theFile->name); ReturnIfError(anErr);
	
	return Qtrue;
}
*/
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