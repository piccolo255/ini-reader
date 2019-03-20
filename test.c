#include "ini-reader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *message_success = "\033[32mTest passed\033[0m";
const char *message_failure = "\033[31mTest failed\033[0m";
//const char *message_success = "Test passed";
//const char *message_failure = "Test failed";

void test_int
   ( int          line_number
   , const char  *name
   , int          test_value
   , int          correct_value
){
   if( test_value == correct_value ){
      printf( "%d: %s: %s\n", line_number, message_success, name );
   } else {
      printf( "%d: %s: %s\n", line_number, message_failure, name );
      printf( "  | Correct: %d\n", correct_value );
      printf( "  | Program: %d\n", test_value );
   }
}

void test_double
   ( int          line_number
   , const char  *name
   , double       test_value
   , double       correct_value
){
   if( test_value == correct_value ){
      printf( "%d: %s: %s\n", line_number, message_success, name );
   } else {
      printf( "%d: %s: %s\n", line_number, message_failure, name );
      printf( "  | Correct: %lf\n", correct_value );
      printf( "  | Program: %lf\n", test_value );
   }
}

void test_string
   ( int          line_number
   , const char  *name
   , const char  *test_value
   , const char  *correct_value
){
   if( !strcmp( test_value, correct_value ) ){
      printf( "%d: %s: %s\n", line_number, message_success, name );
   } else {
      printf( "%d: %s: %s\n", line_number, message_failure, name );
      printf( "  | Correct: %s\n", correct_value );
      printf( "  | Program: %s\n", test_value );
   }
}

void test_pointer
   ( int          line_number
   , const char  *name
   , void        *test_value
   , void        *correct_value
){
   if( test_value == correct_value ){
      printf( "%d: %s: %s\n", line_number, message_success, name );
   } else {
      printf( "%d: %s: %s\n", line_number, message_failure, name );
      printf( "  | Correct: %p\n", correct_value );
      printf( "  | Program: %p\n", test_value );
   }
}

int main( void ){
   const char *test_file = "test.ini";
   const char *test_file_repeat = "test_repeat.ini";
   const char *test_file_missing = "test_non_existent.ini";
   
   FILE *fp = fopen( test_file, "w" );
   if( fp == NULL ){
      fprintf( stderr, "Error opening config file." );
      exit( EXIT_FAILURE );
   }
   fprintf( fp, "key in global section = -255\n" );
   fprintf( fp, "[section]\n" );
   fprintf( fp, "key in named section = 255\n" );
   fprintf( fp, "key in missing section = 256\n" );
   fprintf( fp, "[types]\n" );
   fprintf( fp, "integer value = 42\n" );
   fprintf( fp, "double value  = 3.14\n" );
   fprintf( fp, "string value  = accelerate\n" );
   fprintf( fp, "[whitespace]\n" );
   fprintf( fp, "  \t  \tkey left=8\n" );
   fprintf( fp, "key right  \t  \t=9\n" );
   fprintf( fp, "  \t  \tkey both  \t  \t=10\n" );
   fprintf( fp, "value left=  \t  \t11\n" );
   fprintf( fp, "value right=12  \t  \t\n" );
   fprintf( fp, "value both=  \t  \t13  \t  \t\n" );
   fprintf( fp, "=empty key\n" );
   fprintf( fp, "empty string =\n" );
   fprintf( fp, "empty int    =\n" );
   fprintf( fp, "empty double =\n" );
   fprintf( fp, "whitespace-only int =  \t  \t \n" );
   fclose( fp );

   fp = fopen( test_file_repeat, "w" );
   if( fp == NULL ){
      fprintf( stderr, "Error opening config file." );
      exit( EXIT_FAILURE );
   }
   fprintf( fp, "[repeat section]\n" );
   fprintf( fp, "[repeat section]\n" );
   fclose( fp );
   
   ini_reader_error_code err;
   ini_reader_data ini = NULL;
   err = ini_reader_parse( &ini, test_file_missing );

   test_int( __LINE__
           , "parse non-existent file, error code"
           , (int)err
           , (int)E_INI_READER_FILE_NOT_FOUND );

   ini_reader_free( &ini );

   test_pointer( __LINE__
               , "free data structure, data pointer"
               , (void*)ini
               , (void*)NULL );

   err = ini_reader_parse( &ini, test_file );

   test_int( __LINE__
           , "parse file, error code"
           , (int)err
           , (int)E_INI_READER_SUCCESS );

   if( err != E_INI_READER_SUCCESS ){
      // TODO: print detailed error
      fprintf( stderr, "Error parsing configuration file: %s\n", ini_reader_get_last_error_details(ini) );
      exit( EXIT_FAILURE );
   }

//   // read values
//
//   test_int( __LINE__
//           , "read integer"
//           , ini_reader_get_int( ini, "types", "integer value", -1 )
//           , 42 );
//
//   test_double( __LINE__
//              , "read double"
//              , ini_reader_get_double( ini, "types", "double value", -1.0 )
//              , 3.14 );

   test_string( __LINE__
              , "read string"
              , ini_reader_get_string( ini, "types", "string value", "break" )
              , "accelerate" );

//   // section & key presence
//
//   test_int( __LINE__
//           , "read from missing key"
//           , ini_reader_get_int( ini, "section", "missing key", -1 )
//           , -1 );
//
//   test_int( __LINE__
//           , "read from missing section"
//           , ini_reader_get_int( ini, "missing section", "key in missing section", -1 )
//           , -1 );
//
//   test_int( __LINE__
//           , "read from key in named section"
//           , ini_reader_get_int( ini, "section", "key in named section", -1 )
//           , 255 );
//
//   test_int( __LINE__
//           , "read from key in global section"
//           , ini_reader_get_int( ini, "", "key in global section", -1 )
//           , -255 );
//
//   // whitespace
//
//   test_int( __LINE__
//           , "key, left whitespace"
//           , ini_reader_get_int( ini, "whitespace", "key left", -1 )
//           , 8 );
//
//   test_int( __LINE__
//           , "key, right whitespace"
//           , ini_reader_get_int( ini, "whitespace", "key right", -1 )
//           , 9 );
//
//   test_int( __LINE__
//           , "key, left and right whitespace"
//           , ini_reader_get_int( ini, "whitespace", "key both", -1 )
//           , 10 );
//
//   test_int( __LINE__
//           , "value, left whitespace"
//           , ini_reader_get_int( ini, "whitespace", "value left", -1 )
//           , 11 );
//
//   test_int( __LINE__
//           , "value, right whitespace"
//           , ini_reader_get_int( ini, "whitespace", "value right", -1 )
//           , 12 );
//
//   test_int( __LINE__
//           , "value, left and right whitespace"
//           , ini_reader_get_int( ini, "whitespace", "value both", -1 )
//           , 13 );
//
//   test_string( __LINE__
//              , "empty key"
//              , ini_reader_get_string( ini, "whitespace", "", "skipped" )
//              , "skipped" );
//
//   test_string( __LINE__
//              , "empty string value"
//              , ini_reader_get_string( ini, "whitespace", "empty string", "aaa" )
//              , "" );
//
   ini_reader_free( &ini );

   err = ini_reader_parse( &ini, test_file_repeat );

   test_int( __LINE__
           , "parse file, duplicate sections"
           , (int)err
           , (int)E_INI_READER_DUPLICATE_SECTION );

   ini_reader_free( &ini );


   return 0;
}
