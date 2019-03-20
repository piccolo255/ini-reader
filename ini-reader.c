#include "ini-reader.h"

// local includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifndef INI_READER_STRLEN
   #define INI_READER_STRLEN  256
#endif

// linked list to store config file properties
typedef struct _t_ini_reader_property * t_ini_reader_property;
struct _t_ini_reader_property {
   t_ini_reader_property   next;
   char                    key[INI_READER_STRLEN];
   char                    value[INI_READER_STRLEN];
};

// linked list to store config file sections
typedef struct _t_ini_reader_section * t_ini_reader_section;
struct _t_ini_reader_section {
   t_ini_reader_section next;
   char                 key[INI_READER_STRLEN];
   t_ini_reader_property       properties;
};

// parent structure
struct _ini_reader_data {
   t_ini_reader_section    head_section;
   ini_reader_error_code   last_error_code;
   char                    last_error_details[INI_READER_STRLEN];
};

// linked list handling
void add_section( ini_reader_data    parent
                , t_ini_reader_section new );
t_ini_reader_section get_section( ini_reader_data   parent
                                , const char         *key );
t_ini_reader_section new_section( const char *key );
void clear_sections( ini_reader_data parent );
void add_property( t_ini_reader_section   parent
                 , t_ini_reader_property  new );
t_ini_reader_property get_property( t_ini_reader_section  parent
                           , const char           *key );
t_ini_reader_property new_property( const char *key
                           , const char *value );
void clear_properties( t_ini_reader_section parent );

// utility functions
void strncpy_fixed( char        *dest
                  , const char  *src
                  , size_t       count );
void trim_edge_whitespace( char *str );
void set_last_error( ini_reader_data         ini_data
                   , ini_reader_error_code   errcode
                   , const char             *message );
void append_last_error( ini_reader_data   ini_data
                       , const char      *message );

// basic getter
const char *ini_reader_get_basic( ini_reader_data  ini_data
                                , const char      *section
                                , const char      *key );

/************************************************************* Implementation */

void strncpy_fixed
   ( char        *dest
   , const char  *src
   , size_t       count
){
   strncpy( dest, src, count );
   dest[count-1] = '\0';
}


void trim_edge_whitespace
   ( char *str
){
   char *first = str;
   char *last  = str + strlen(str) - 1;

   while( *first && isspace(*first) )
      first++;
   while( last > first && isspace(*last) )
      last--;
   last[1] = '\0';

   while( *first ) {
      *str = *first;
      str++;
      first++;
   }
   *str = *first;
}

void set_last_error
   ( ini_reader_data          ini_data
   , ini_reader_error_code    errcode
   , const char              *message
){
   ini_data->last_error_code = errcode;
   strncpy_fixed( ini_data->last_error_details, message, INI_READER_STRLEN );
}

void append_last_error
   ( ini_reader_data    ini_data
   , const char        *message
){
   int len = strlen( ini_data->last_error_details );
   strncpy_fixed( ini_data->last_error_details+len, message, INI_READER_STRLEN-len );
}

t_ini_reader_section new_section
   ( const char *key
){
   t_ini_reader_section section;

   section = (t_ini_reader_section)malloc( sizeof(*section) );
   section->next = NULL;
   section->properties = NULL;
   strncpy_fixed( section->key, key, INI_READER_STRLEN );

   return section;
}

void add_section
   ( ini_reader_data parent
   , t_ini_reader_section new
){
   new->next = parent->head_section;
   parent->head_section = new;
}

t_ini_reader_section get_section
   ( ini_reader_data  parent
   , const char        *key
){
   t_ini_reader_section s = parent->head_section;

   while( s ){
      if( strcmp( s->key, key ) == 0 )
         break;
      s = s->next;
   }

   return s;
}

void clear_sections
   ( ini_reader_data parent
){
   t_ini_reader_section s = parent->head_section;
   t_ini_reader_section next;
   while( s ){
      clear_properties( s );
      next = s->next;
      free( s );
      s = next;
   }
   parent->head_section = NULL;
}

t_ini_reader_property new_property
   ( const char *key
   , const char *value
){
   t_ini_reader_property property;

   property = (t_ini_reader_property)malloc( sizeof(*property) );
   property->next = NULL;
   strncpy_fixed( property->key,   key,   INI_READER_STRLEN );
   strncpy_fixed( property->value, value, INI_READER_STRLEN );

   return property;
}

void add_property
   ( t_ini_reader_section  parent
   , t_ini_reader_property new
){
   new->next = parent->properties;
   parent->properties = new;
}

t_ini_reader_property get_property
   ( t_ini_reader_section  section
   , const char           *key
){
   t_ini_reader_property p = section->properties;

   while( p ){
      if( strcmp( p->key, key ) == 0 )
         break;
      p = p->next;
   }

   return p;
}

void clear_properties
   ( t_ini_reader_section parent
){
   t_ini_reader_property p = parent->properties;
   t_ini_reader_property next;
   while( p ){
      next = p->next;
      free( p );
      p = next;
   }
   parent->properties = NULL;
}

ini_reader_data ini_reader_parse
   ( const char *filename
){
   FILE *fp;
   char line[INI_READER_STRLEN];
   char key[INI_READER_STRLEN];
   char value[INI_READER_STRLEN];
   ini_reader_data    data;
   t_ini_reader_section current_section;
   int line_number = 0;
   char error_message[INI_READER_STRLEN];

   /// Initialize data structures
   data = (ini_reader_data)malloc( sizeof(*data) );
   set_last_error( data, E_INI_READER_SUCCESS, "" );

   data->head_section = new_section( "" );;
   current_section = data->head_section;

   /// Open file for reading, check for errors
   fp = fopen( filename, "r" );
   if( fp == NULL ){
      snprintf( error_message, INI_READER_STRLEN, "failed to open: %s", filename );
      set_last_error( data, E_INI_READER_FILE_NOT_FOUND, error_message );
      return data;
   }

   /// Parse the config file
   while( fgets( line, INI_READER_STRLEN, fp ) != NULL ){
      // increase line counter
      line_number++;

      // remove whitespace from the beginning and end of the line
      trim_edge_whitespace( line );

      // section
      if( line[0] == '[' && line[strlen(line)-1] == ']' ){
         // remove section delimiters ( '[', ']' )
         char *name = line+1;
         name[strlen(name)-1] = '\0';

         // refuse duplicate sections
         if( get_section( data, name ) ){
            snprintf( error_message, INI_READER_STRLEN, "%s, line %d: Duplicate section defined.", filename, line_number );
            set_last_error( data, E_INI_READER_DUPLICATE_SECTION, error_message );
            return data;
         }

         // add new section, set as current
         current_section = new_section( name );
         add_section( data, current_section );

         continue;
      }

      // if line begins with a number or a letter, read a property
      if( isalnum(line[0]) ){
         // look for the '=' sign
         char *eq = strchr( line, '=' );

         // check if proper assignment
         if( !eq ){
            snprintf( error_message, INI_READER_STRLEN, "%s, line %d: Not a 'key = value' expression.", filename, line_number );
            set_last_error( data, E_INI_READER_PARSE_FAIL, error_message );
            return data;
         }

         // extract key
         *eq = '\0';
         strncpy_fixed( key, line, INI_READER_STRLEN );
         trim_edge_whitespace( key );

         // extract value
         strncpy_fixed( value, eq+1, INI_READER_STRLEN );
         trim_edge_whitespace( value );

         // refuse duplicate properties in a section
         if( get_property( current_section, key ) ){
            sprintf( error_message, "%s, line %d: Duplicate property key defined.", filename, line_number );
            set_last_error( data, E_INI_READER_DUPLICATE_PROPERTY, error_message );
            return data;
         }

         // add new property
         add_property( current_section, new_property( key, value ) );
      }
   }

   return data;
}

void ini_reader_free
  ( ini_reader_data ini_data
){
   clear_sections( ini_data );
   free( ini_data );
}

const char *ini_reader_get_basic
   ( ini_reader_data    ini_data
   , const char        *section
   , const char        *key
){
   t_ini_reader_section  s;
   t_ini_reader_property p;
   char error_message[INI_READER_STRLEN];

   // locate section, return NULL if not found
   s = get_section( ini_data, section );
   if( !s ){
      snprintf( error_message, INI_READER_STRLEN, "section [%s] not found", section );
      set_last_error( ini_data, E_INI_READER_SECTION_NOT_FOUND, error_message );
      return NULL;
   }

   // locate property, return NULL if not found
   p = get_property( s, key );
   if( !p ){
      snprintf( error_message, INI_READER_STRLEN, "property '%s' in section [%s] not found", key, section );
      set_last_error( ini_data, E_INI_READER_PROPERTY_NOT_FOUND, error_message );
      return NULL;
   }

   // property found, return value
   set_last_error( ini_data, E_INI_READER_SUCCESS, "" );
   return p->value;
}

const char *ini_reader_get_string
   ( ini_reader_data    ini_data
   , const char        *section
   , const char        *key
   , char              *default_value
){
   const char *val_str;
   char error_message[INI_READER_STRLEN];

   // locate property, return default value if not found
   val_str = ini_reader_get_basic( ini_data, section, key );
   if( !val_str ){
      snprintf( error_message, INI_READER_STRLEN, ": assuming [%s] %s = %s", section, key, default_value );
      append_last_error( ini_data, error_message );
      return default_value;
   }

   // property found, return value
   return val_str;
}

int ini_reader_get_int
   ( ini_reader_data    ini_data
   , const char        *section
   , const char        *key
   , int                default_value
){
   const char *val_str;
   char error_message[INI_READER_STRLEN];

   // locate property, return default value if not found
   val_str = ini_reader_get_basic( ini_data, section, key );
   if( !val_str ){
      snprintf( error_message, INI_READER_STRLEN, ": assuming [%s] %s = %d", section, key, default_value );
      append_last_error( ini_data, error_message );
      return default_value;
   }

   // property found, return value
   return atoi( val_str );
}

double ini_reader_get_double
   ( ini_reader_data    ini_data
   , const char        *section
   , const char        *key
   , double             default_value
){
   const char *val_str;
   char error_message[INI_READER_STRLEN];

   // locate property, return default value if not found
   val_str = ini_reader_get_basic( ini_data, section, key );
   if( !val_str ){
      snprintf( error_message, INI_READER_STRLEN, ": assuming [%s] %s = %lf", section, key, default_value );
      append_last_error( ini_data, error_message );
      return default_value;
   }

   // property found, return value
   return atof( val_str );
}

const char *ini_reader_get_error_description
   ( ini_reader_error_code error_code
){
   // TODO
   printf( "ini_reader_get_error_description: not implemented!" );
   exit( EXIT_FAILURE );
   return NULL;
}

ini_reader_error_code ini_reader_get_last_error_code
   ( ini_reader_data ini_data
){
   return ini_data->last_error_code;
}

const char *ini_reader_get_last_error_description
   ( ini_reader_data ini_data
){
   // TODO
   printf( "ini_reader_get_last_error_description: not implemented!" );
   exit( EXIT_FAILURE );
   return NULL;
}

const char *ini_reader_get_last_error_details
   ( ini_reader_data ini_data
){
   return ini_data->last_error_details;
}
