#ifndef _H_YAML_
#define _H_YAML_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* util */

#if ( defined _WIN32 )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC		_declspec(dllexport)
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC		extern
#endif
#endif

/* error */

#define YAML_ERROR_SCANNER_INVALID			-110
#define YAML_ERROR_SCANNER_QUOTE_NOT_MATCH		-111
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_0	-221
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1	-231
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2	-232
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_3	-233
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_4	-234
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_ARRAY_1	-241
#define YAML_ERROR_PARSER_INVALID_ON_TOKEN_ARRAY_2	-242

#define YAML_NODE_BRANCH				0x10
#define YAML_NODE_LEAF					0x20
#define YAML_NODE_ARRAY					0x40
#define YAML_NODE_ENTER					0x01
#define YAML_NODE_LEAVE					0x02

typedef int funcCallbackOnYAMLNode( int type , char *ypath , int ypath_len , int ypath_size , char *node , int node_len , char *content , int content_len , void *p );
_WINDLL_FUNC int ParserYAMLBuffer( char *yaml_buffer , int yaml_buffer_len , char *ypath , int ypath_size 
		    , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLNode
		    , void *p ) ;
_WINDLL_FUNC int ParserYAMLBuffer4( char *yaml_buffer , int yaml_buffer_len , char *ypath , int ypath_size 
		     , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLBranch
		     , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLBranch
		     , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLArray
		     , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLArray
		     , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLLeaf
		     , void *p ) ;

#ifdef __cplusplus
}
#endif

#endif

