#ifndef _H_YAML_IN_
#define _H_YAML_IN_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef MEMCMP
#define MEMCMP(_a_,_C_,_b_,_n_) ( memcmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef MAX
#define MAX(_a_,_b_) (_a_>_b_?_a_:_b_)
#endif

typedef struct YAML_mark_s
{
	int			line ;
	int			column ;
	int			index ;
} YAML_mark_t ;

typedef struct YAML_info_s
{
	char				*value ;
	int				value_len ;
	int				value_size ;
	YAML_mark_t			mark ;
} YAML_info_t ;

typedef enum YAML_encode_type_e
{
	YAML_ANY_ENCODING,
	YAML_UTF8_ENCODING
} YAML_encode_type_t ;

typedef enum YAML_token_type_e
{
    YAML_NO_TOKEN,

    YAML_STREAM_START_TOKEN,
    YAML_STREAM_END_TOKEN,

    YAML_DOCUMENT_START_TOKEN,
    YAML_DOCUMENT_END_TOKEN,

    YAML_BLOCK_SEQUENCE_START_TOKEN,
    YAML_BLOCK_MAPPING_START_TOKEN,
    YAML_BLOCK_END_TOKEN,

    YAML_FLOW_SEQUENCE_START_TOKEN,
    YAML_FLOW_SEQUENCE_END_TOKEN,
    YAML_FLOW_MAPPING_START_TOKEN,
    YAML_FLOW_MAPPING_END_TOKEN,
    
    YAML_BLOCK_ENTRY_TOKEN,
    YAML_FLOW_ENTRY_TOKEN,

    YAML_KEY_TOKEN,
    YAML_VALUE_TOKEN,

    YAML_SCALAR_TOKEN
} YAML_token_type_t ;

struct YAML_token
{
	YAML_token_type_t		token_type ;

	YAML_mark_t			mark ;

	YAML_info_t			scalar ;
} ;

struct YAML_parser
{
	YAML_encode_type_t		encode ;

	struct
	{
		char			*start ;
		char			*last ;
		char			*pointer ;
	} buffer ;
	int				unread ;

	YAML_mark_t			mark ;

	YAML_info_t			path ;
	YAML_info_t			pre_node ;

	int				indent ;
	int				block_level ;
	int				flow_level ;
	int				jump_leaf_2 ;
	int				key_allow ;

	void				*p ;

	funcCallbackOnYAMLNode		*pfuncCallbackOnEnterYAMLBranch ;
	funcCallbackOnYAMLNode		*pfuncCallbackOnLeaveYAMLBranch ;
	funcCallbackOnYAMLNode		*pfuncCallbackOnEnterYAMLArray ;
	funcCallbackOnYAMLNode		*pfuncCallbackOnLeaveYAMLArray ;
	funcCallbackOnYAMLNode		*pfuncCallbackOnYAMLLeaf ;

} ;

int YAML_fetch_next_token( struct YAML_parser *parser , struct YAML_token *token ) ;

/*
 * String check operations.
 */

/*
 * Check the octet at the specified position.
 */

#define CHECK_AT(string,octet,offset)                   \
    ((string).pointer[offset] == (char)(octet))

/*
 * Check the current octet in the buffer.
 */

#define CHECK(string,octet) (CHECK_AT((string),(octet),0))

/*
 * Check if the character at the specified position is NUL.
 */

#define IS_Z_AT(string,offset)    CHECK_AT((string),'\0',(offset))

#define IS_Z(string)    IS_Z_AT((string),0)

/*
 * Check if the character at the specified position is space.
 */

#define IS_SPACE_AT(string,offset)  CHECK_AT((string),' ',(offset))

#define IS_SPACE(string)    IS_SPACE_AT((string),0)

/*
 * Check if the character at the specified position is tab.
 */

#define IS_TAB_AT(string,offset)    CHECK_AT((string),'\t',(offset))

#define IS_TAB(string)  IS_TAB_AT((string),0)

/*
 * Check if the character at the specified position is blank (space or tab).
 */

#define IS_BLANK_AT(string,offset)                                              \
    (IS_SPACE_AT((string),(offset)) || IS_TAB_AT((string),(offset)))

#define IS_BLANK(string)    IS_BLANK_AT((string),0)

/*
 * Check if the character at the specified position is a line break.
 */

#define IS_BREAK_AT(string,offset)                                              \
    (CHECK_AT((string),'\r',(offset))               /* CR (#xD)*/               \
     || CHECK_AT((string),'\n',(offset)))            /* LF (#xA) */             \

#define IS_BREAK(string)    IS_BREAK_AT((string),0)

#define IS_CRLF_AT(string,offset)                                               \
     (CHECK_AT((string),'\r',(offset)) && CHECK_AT((string),'\n',(offset)+1))

#define IS_CRLF(string) IS_CRLF_AT((string),0)

/*
 * Check if the character is a line break or NUL.
 */

#define IS_BREAKZ_AT(string,offset)                                             \
    (IS_BREAK_AT((string),(offset)) || IS_Z_AT((string),(offset)))

#define IS_BREAKZ(string)   IS_BREAKZ_AT((string),0)

/*
 * Check if the character is a line break, space, or NUL.
 */

#define IS_SPACEZ_AT(string,offset)                                             \
    (IS_SPACE_AT((string),(offset)) || IS_BREAKZ_AT((string),(offset)))

#define IS_SPACEZ(string)   IS_SPACEZ_AT((string),0)

/*
 * Check if the character is a line break, space, tab, or NUL.
 */

#define IS_BLANKZ_AT(string,offset)                                             \
    (IS_BLANK_AT((string),(offset)) || IS_BREAKZ_AT((string),(offset)))

#define IS_BLANKZ(string)   IS_BLANKZ_AT((string),0)

/*
 * Determine the width of the character.
 */

#define WIDTH_AT(string,offset)                                                 \
     (((string).pointer[offset] & 0x80) == 0x00 ? 1 :                           \
      ((string).pointer[offset] & 0xE0) == 0xC0 ? 2 :                           \
      ((string).pointer[offset] & 0xF0) == 0xE0 ? 3 :                           \
      ((string).pointer[offset] & 0xF8) == 0xF0 ? 4 : 0)

#define WIDTH(string)   WIDTH_AT((string),0)

/*
 * Move the string pointer to the next character.
 */

#define MOVE(string)    ((string).pointer += WIDTH((string)))

#endif
