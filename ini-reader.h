#ifndef INI_READER_H_
#define INI_READER_H_

enum _ini_reader_error_code
   { E_INI_READER_SUCCESS = 0
   , E_INI_READER_FILE_NOT_FOUND = -1
   , E_INI_READER_PARSE_FAIL = -10
   , E_INI_READER_DUPLICATE_SECTION = -11
   , E_INI_READER_DUPLICATE_PROPERTY = -12
   , E_INI_READER_SECTION_NOT_FOUND = -21
   , E_INI_READER_PROPERTY_NOT_FOUND = -22
};

typedef enum _ini_reader_error_code ini_reader_error_code;

// config file struct
typedef struct _ini_reader_data * ini_reader_data;

// parse configuration file
ini_reader_error_code ini_reader_parse( ini_reader_data *ini_data
                                      , const char      *filename );

//// getters for config data
const char *ini_reader_get_string( ini_reader_data    ini_data
                                 , const char        *section
                                 , const char        *key
                                 , char              *default_value );
//int ini_reader_get_int( t_ini_reader_data ini_data
//                      , const char       *section
//                      , const char       *key
//                      , int               default_value );
//double ini_reader_get_double( t_ini_reader_data ini_data
//                            , const char       *section
//                            , const char       *key
//                            , double            default_value );

// bookkeeping
void ini_reader_free( ini_reader_data *ini_data );

// error handling
const char *ini_reader_get_error_description( ini_reader_error_code error_code );
ini_reader_error_code ini_reader_get_last_error_code( ini_reader_data ini_data );
const char *ini_reader_get_last_error_description( ini_reader_data ini_data );
const char *ini_reader_get_last_error_details( ini_reader_data ini_data );

#endif
