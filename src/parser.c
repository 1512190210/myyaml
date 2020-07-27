#include "myyaml.h"
#include "myyaml_in.h"

void YAML_parser_init( struct YAML_parser *parser ) ;
void YAML_parser_set_p( struct YAML_parser *parser , void *p ) ;
void YAML_parser_set_indent( struct YAML_parser *parser , int indent ) ;
void YAML_parser_set_buffer( struct YAML_parser *parser , char *buf , int buf_len ) ;
void YAML_parser_set_path( struct YAML_parser *parser , char *path , int path_size ) ;
void YAML_parser_set_callback( struct YAML_parser *parser
			     , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLNode ) ;
void YAML_parser_set_callback4( struct YAML_parser *parser
			      , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLBranch
			      , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLBranch
			      , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLArray
			      , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLArray
			      , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLLeaf ) ;

int ParserYAMLLeafBuffer( struct YAML_parser *parser , char *nodename , int nodename_len , char *content , int content_len ) ;
int _ParserYAMLArrayBuffer( struct YAML_parser *parser , char *nodename , int nodename_len ) ;
int ParserYAMLArrayBuffer( struct YAML_parser *parser , char *nodename , int nodename_len ) ;
int _ParserYAMLSeqBuffer( struct YAML_parser *parser , int indent ) ;
int ParserYAMLSeqBuffer( struct YAML_parser *parser , char *nodename , int nodename_len , int indent ) ;
int _ParserYAMLBranchBuffer( struct YAML_parser *parser ) ;
int ParserYAMLBranchBuffer( struct YAML_parser *parser , char *nodename , int nodename_len ) ;
int _ParserYAMLMapBuffer( struct YAML_parser *parser , int indent ) ;
int ParserYAMLMapBuffer( struct YAML_parser *parser , char *nodename , int nodename_len , int indent ) ;
int _ParserYAMLBuffer( struct YAML_parser *parser ) ;

void YAML_parser_init( struct YAML_parser *parser )
{
	memset( parser , 0x00 , sizeof(struct YAML_parser) ) ;
}

void YAML_parser_set_indent( struct YAML_parser *parser , int indent )
{
	parser->indent = indent ;
}

void YAML_parser_set_buffer( struct YAML_parser *parser , char *buf , int buf_len )
{
	parser->buffer.start = parser->buffer.pointer = buf ;
	parser->buffer.last = parser->buffer.start + buf_len ;
	parser->unread = buf_len ;
}

void YAML_parser_set_path( struct YAML_parser *parser , char *path , int path_size )
{
	parser->path.value = path ;
	parser->path.value_size = path_size ;
}

void YAML_parser_add_path( struct YAML_parser *parser , char *add_path , int add_path_len )
{
	if( parser->path.value_len + add_path_len + 1 < parser->path.value_size )
	{
		*(parser->path.value + parser->path.value_len) = '/' ;
		memcpy( parser->path.value + parser->path.value_len + 1 , add_path , add_path_len ) ;
		parser->path.value_len += add_path_len + 1 ;
		*(parser->path.value + parser->path.value_len) = '\0' ;
	}
	else if( parser->path.value_len + 2 < parser->path.value_size )
	{
		memcpy( parser->path.value + parser->path.value_len , "/*" , 2 ) ;
		parser->path.value_len += 2 ;
		*(parser->path.value + parser->path.value_len) = '\0' ;
	}
}

void YAML_parser_rollback_path( struct YAML_parser *parser , int path_len )
{
	parser->path.value[path_len] = '\0' ;
	parser->path.value_len = path_len ;
}

void YAML_parser_set_callback( struct YAML_parser *parser
			     , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLNode )
{
	return YAML_parser_set_callback4( parser , pfuncCallbackOnYAMLNode
						 , pfuncCallbackOnYAMLNode
						 , pfuncCallbackOnYAMLNode
						 , pfuncCallbackOnYAMLNode
						 , pfuncCallbackOnYAMLNode );

}

void YAML_parser_set_callback4( struct YAML_parser *parser
			      , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLBranch
			      , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLBranch
			      , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLArray
			      , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLArray
			      , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLLeaf )
{
	parser->pfuncCallbackOnEnterYAMLBranch = pfuncCallbackOnEnterYAMLBranch ;
	parser->pfuncCallbackOnLeaveYAMLBranch = pfuncCallbackOnLeaveYAMLBranch ;
	parser->pfuncCallbackOnEnterYAMLArray = pfuncCallbackOnEnterYAMLArray ;
	parser->pfuncCallbackOnLeaveYAMLArray = pfuncCallbackOnLeaveYAMLArray ;
	parser->pfuncCallbackOnYAMLLeaf = pfuncCallbackOnYAMLLeaf ;
}

void YAML_parser_set_p( struct YAML_parser *parser , void *p )
{
	parser->p = p ;
}

void YAML_parser_print_token( struct YAML_token *token )
{
	printf("token_type[%d] get value[%.*s][%d]\n" , token->token_type , token->scalar.value_len , token->scalar.value , token->scalar.value_len ) ;
}

int ParserYAMLLeafBuffer( struct YAML_parser *parser , char *nodename , int nodename_len , char *content , int content_len )
{
	int		nret = 0 ;

	if( parser->pfuncCallbackOnYAMLLeaf )
	{
		nret = parser->pfuncCallbackOnYAMLLeaf( YAML_NODE_LEAF , parser->path.value , parser->path.value_len , parser->path.value_size , nodename ,  nodename_len , content , content_len ,  parser->p ) ;
		if( nret < 0 )
			return nret;
	}

	return 0 ;
}

int _ParserYAMLArrayBuffer( struct YAML_parser *parser , char *nodename , int nodename_len ) 
{
	int				nret = 0 , first = 1 ;
	int				ypath_len = parser->path.value_len ;
	
	struct YAML_token		token ;
	
	while(1) 
	{
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_SEQUENCE_END_TOKEN )
			return first ? first = 0 : YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
		if( token.token_type == YAML_FLOW_SEQUENCE_START_TOKEN ) 
		{
			nret = ParserYAMLArrayBuffer( parser , nodename , nodename_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_FLOW_MAPPING_START_TOKEN ) 
		{
			YAML_parser_rollback_path( parser , ypath_len ) ;

			YAML_parser_add_path( parser , "." , 1 ) ;
			nret = ParserYAMLBranchBuffer( parser , nodename , nodename_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_SCALAR_TOKEN ) 
		{
			YAML_parser_rollback_path( parser , ypath_len ) ;

			YAML_parser_add_path( parser , "." , 1 ) ;
			nret = ParserYAMLLeafBuffer( parser , nodename , nodename_len , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
		}
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_SEQUENCE_END_TOKEN ) 
		{
			break ;
		} 
		else if( token.token_type == YAML_FLOW_ENTRY_TOKEN ) 
		{
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
		}
	}
	
	return 0 ;
}

int ParserYAMLArrayBuffer( struct YAML_parser *parser , char *nodename , int nodename_len ) 
{
	int			nret = 0 ;
	int			ypath_len = parser->path.value_len ;

	parser->flow_level++ ;
	if( parser->pfuncCallbackOnEnterYAMLArray ) 
	{
		nret = parser->pfuncCallbackOnEnterYAMLArray( YAML_NODE_ENTER | YAML_NODE_ARRAY , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	
	nret = _ParserYAMLArrayBuffer( parser , nodename , nodename_len ) ;
	if( nret )
		return nret;
	YAML_parser_rollback_path( parser , ypath_len ) ;

	if( parser->pfuncCallbackOnLeaveYAMLArray ) 
	{
		nret = parser->pfuncCallbackOnLeaveYAMLArray( YAML_NODE_LEAVE | YAML_NODE_ARRAY , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	parser->flow_level-- ;
	
	return 0 ;
}

int _ParserYAMLSeqBuffer( struct YAML_parser *parser , int indent ) 
{
	int				nret = 0 , first = 1 ;
	int				ypath_len = parser->path.value_len ;
	struct YAML_token		token ;
	while(1) 
	{
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_BLOCK_END_TOKEN ||
		    token.token_type == YAML_DOCUMENT_START_TOKEN ||
		    token.token_type == YAML_DOCUMENT_END_TOKEN ||
		    token.token_type == YAML_STREAM_END_TOKEN )
			return first ? first = 0 : YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
		if( token.token_type == YAML_FLOW_SEQUENCE_START_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			nret = ParserYAMLArrayBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_FLOW_MAPPING_START_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			nret = ParserYAMLBranchBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_BLOCK_ENTRY_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			nret = ParserYAMLSeqBuffer( parser , token.scalar.value , token.scalar.value_len , token.mark.column ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_SCALAR_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			
			parser->pre_node = token.scalar ;
			nret = YAML_fetch_next_token( parser , & token ) ;
			if( nret )
				return nret ;

			if( token.token_type == YAML_BLOCK_END_TOKEN ||
			    token.token_type == YAML_DOCUMENT_START_TOKEN ||
			    token.token_type == YAML_DOCUMENT_END_TOKEN ||
			    token.token_type == YAML_STREAM_END_TOKEN ) 
			{
				YAML_parser_rollback_path( parser , ypath_len ) ;

				YAML_parser_add_path( parser , "." , 1 ) ;
				nret = ParserYAMLLeafBuffer( parser , NULL , 0 , token.scalar.value , token.scalar.value_len ) ;
				if( nret )
					return nret ;
				break ;
			} 
			else if( token.token_type == YAML_BLOCK_ENTRY_TOKEN ) 
			{
				YAML_parser_rollback_path( parser , ypath_len ) ;

				YAML_parser_add_path( parser , "." , 1 ) ;
				nret = ParserYAMLLeafBuffer( parser , NULL , 0 , token.scalar.value , token.scalar.value_len ) ;
				if( nret )
					return nret ;
				if( token.mark.column != indent )
					return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
				continue ;
			} 
			else if( token.token_type == YAML_VALUE_TOKEN ) 
			{
				YAML_parser_rollback_path( parser , ypath_len ) ;

				YAML_parser_add_path( parser , "." , 1 ) ;
				parser->key_allow = 1 ;
				parser->jump_leaf_2 = 1 ;
				nret = ParserYAMLMapBuffer( parser , parser->pre_node.value , parser->pre_node.value_len , parser->pre_node.mark.column ) ;
				if( nret )
					return nret ;
			} 
			else 
			{
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			}
		} 
		else if( token.token_type == YAML_KEY_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			nret = YAML_fetch_next_token( parser , & token ) ;
			if( nret )
				return nret ;
			if( token.token_type != YAML_SCALAR_TOKEN )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			parser->pre_node = token.scalar ;
			nret = ParserYAMLMapBuffer( parser , "-" , 1 , indent ) ;
			if( nret )
				return nret ;
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
		}
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_BLOCK_END_TOKEN ||
		    token.token_type == YAML_DOCUMENT_START_TOKEN ||
		    token.token_type == YAML_DOCUMENT_END_TOKEN ||
		    token.token_type == YAML_STREAM_END_TOKEN ) 
			break ;
		else if( token.token_type == YAML_BLOCK_ENTRY_TOKEN ) 
		{
			if( token.mark.column != indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
		} 
		else 
		{	
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
		}
	}
	
	return 0 ;
}

int ParserYAMLSeqBuffer( struct YAML_parser *parser , char *nodename , int nodename_len , int indent ) 
{
	int			nret = 0 ;
	int			ypath_len = parser->path.value_len ;

	parser->block_level++ ;
	if( parser->pfuncCallbackOnEnterYAMLArray ) 
	{
		nret = parser->pfuncCallbackOnEnterYAMLArray( YAML_NODE_ENTER | YAML_NODE_ARRAY , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	
	nret = _ParserYAMLSeqBuffer( parser , indent ) ;
	if( nret )
		return nret;
	YAML_parser_rollback_path( parser , ypath_len ) ;

	if( parser->pfuncCallbackOnLeaveYAMLArray ) 
	{
		nret = parser->pfuncCallbackOnLeaveYAMLArray( YAML_NODE_LEAVE | YAML_NODE_ARRAY , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	parser->block_level-- ;
	
	return 0 ;
}

int _ParserYAMLBranchBuffer( struct YAML_parser *parser ) 
{
	int				nret = 0 , first = 1 ;
	
	char*				nodename = NULL ;
	int				nodename_len = 0 , ypath_len = parser->path.value_len ;
	
	struct YAML_token		token ;
	
	while(1) 
	{
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_MAPPING_END_TOKEN )
			return first ? first = 0 : YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;

		if( token.token_type == YAML_FLOW_SEQUENCE_START_TOKEN ) 
		{
			nret = ParserYAMLArrayBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_FLOW_MAPPING_START_TOKEN ) 
		{
			nret = ParserYAMLBranchBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_SCALAR_TOKEN ) 
		{
			YAML_parser_rollback_path( parser , ypath_len ) ;  /* reset path */
			nodename = token.scalar.value ;
			nodename_len = token.scalar.value_len ;
		} 
		else if( token.token_type == YAML_KEY_TOKEN ) 
		{
			YAML_parser_rollback_path( parser , ypath_len ) ;  /* reset path */
			nret = YAML_fetch_next_token( parser , & token ) ;
			if( nret )
				return nret ;
			if( token.token_type != YAML_SCALAR_TOKEN )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			nodename = token.scalar.value ;
			nodename_len = token.scalar.value_len ;
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
		}

		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_MAPPING_END_TOKEN ) 
		{
			break ;
		} 
		else if( token.token_type == YAML_FLOW_ENTRY_TOKEN ) 
		{
			YAML_parser_add_path( parser , "." , 1 ) ;
			nret = ParserYAMLLeafBuffer( parser , NULL , 0 , nodename , nodename_len ) ;
			if( nret )
				return nret ;
			continue ;
		} 
		else if( token.token_type == YAML_VALUE_TOKEN ) 
		{
			if( nodename_len )
				YAML_parser_add_path( parser , nodename , nodename_len ) ;
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
		}
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_SEQUENCE_START_TOKEN ) 
		{
			nret = ParserYAMLArrayBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_FLOW_MAPPING_START_TOKEN ) 
		{
			nret = ParserYAMLBranchBuffer( parser , nodename , nodename_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_SCALAR_TOKEN ) 
		{
			nret = ParserYAMLLeafBuffer( parser , nodename , nodename_len , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_3 ;
		}
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_MAPPING_END_TOKEN ) 
		{
			break ;
		} 
		else if( token.token_type == YAML_FLOW_ENTRY_TOKEN ) 
		{
			continue ;
		} 
		else 
		{
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_4 ;
		}
	}
	
	return 0;
}

int ParserYAMLBranchBuffer( struct YAML_parser *parser , char *nodename , int nodename_len ) 
{
	int			nret = 0 ;
	int			ypath_len = parser->path.value_len ;

	parser->flow_level++ ;
	if( parser->pfuncCallbackOnEnterYAMLBranch ) 
	{
		nret = parser->pfuncCallbackOnEnterYAMLBranch( YAML_NODE_ENTER | YAML_NODE_BRANCH , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}

	nret = _ParserYAMLBranchBuffer( parser ) ;
	if( nret )
		return nret;
	YAML_parser_rollback_path( parser , ypath_len ) ;

	if( parser->pfuncCallbackOnLeaveYAMLBranch ) 
	{
		nret = parser->pfuncCallbackOnLeaveYAMLBranch( YAML_NODE_LEAVE | YAML_NODE_BRANCH , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	parser->flow_level-- ;
	
	return 0 ;
}

int _ParserYAMLMapBuffer( struct YAML_parser *parser , int indent ) 
{
	int			nret = 0 , first = 1 ;
	
	char*			nodename = NULL ;
	int			nodename_len = 0 ;
	int			ypath_len = parser->path.value_len ;
	
	struct YAML_token	token ;

	if( parser->jump_leaf_2 ) 
	{
		parser->jump_leaf_2 = 0 ;
		goto YAML_MAP_TOKEN_LEAF_2 ;
	}
	while(1) 
	{
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;

		if( token.token_type == YAML_BLOCK_END_TOKEN ||
		    token.token_type == YAML_DOCUMENT_START_TOKEN ||
		    token.token_type == YAML_DOCUMENT_END_TOKEN ||
		    token.token_type == YAML_STREAM_END_TOKEN ) 
			return first ? first = 0 : YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
		if( token.token_type == YAML_VALUE_TOKEN ) 
		{
			if( token.mark.line != parser->pre_node.mark.line )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			nodename = parser->pre_node.value ;
			nodename_len = parser->pre_node.value_len ;
			YAML_parser_rollback_path( parser , ypath_len ) ;

			if( nodename_len )YAML_parser_add_path( parser , nodename , nodename_len ) ;
			parser->key_allow = 1 ;
		} 
		else
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;

YAML_MAP_TOKEN_LEAF_2 :
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_FLOW_SEQUENCE_START_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
			nret = ParserYAMLArrayBuffer( parser , nodename , nodename_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_FLOW_MAPPING_START_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
			nret = ParserYAMLBranchBuffer( parser , nodename , nodename_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_BLOCK_ENTRY_TOKEN ) 
		{
			if( token.mark.column == indent )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;
			nret = ParserYAMLSeqBuffer( parser , nodename , nodename_len , token.mark.column ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_SCALAR_TOKEN ) 
		{
			if( token.mark.line != parser->pre_node.mark.line ) 
			{
				if( token.mark.column == indent ) 
				{
					parser->pre_node = token.scalar ;
					continue ;
				} 
				else 
				{
					parser->pre_node = token.scalar ;
					nret = ParserYAMLMapBuffer( parser , nodename , nodename_len , token.mark.column ) ;
					if( nret )
						return nret ;
				}
			} 
			else 
			{
				nret = ParserYAMLLeafBuffer( parser , nodename , nodename_len , token.scalar.value , token.scalar.value_len ) ;
				if( nret )
					return nret ;
			}
		} 
		else if( token.token_type == YAML_BLOCK_END_TOKEN ||
		    token.token_type == YAML_DOCUMENT_START_TOKEN ||
		    token.token_type == YAML_DOCUMENT_END_TOKEN ||
		    token.token_type == YAML_STREAM_END_TOKEN )
			break ;
		else
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_2 ;

		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;
		if( token.token_type == YAML_SCALAR_TOKEN )parser->pre_node = token.scalar ; 
		else if( token.token_type == YAML_KEY_TOKEN ) 
		{
			nret = YAML_fetch_next_token( parser , & token ) ;
			if( nret )
				return nret ;
			if( token.token_type != YAML_SCALAR_TOKEN )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_1 ;
			parser->pre_node = token.scalar ;
		} 
		else if( token.token_type == YAML_BLOCK_END_TOKEN ||
		    token.token_type == YAML_DOCUMENT_START_TOKEN ||
		    token.token_type == YAML_DOCUMENT_END_TOKEN ||
		    token.token_type == YAML_STREAM_END_TOKEN )
			break ;
		else
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_3 ;
	}
	
	return 0;
}

int ParserYAMLMapBuffer( struct YAML_parser *parser , char *nodename , int nodename_len , int indent ) 
{
	int			nret = 0 ;
	int			ypath_len = parser->path.value_len ;

	parser->block_level++ ;
	if( parser->pfuncCallbackOnEnterYAMLBranch ) 
	{
		nret = parser->pfuncCallbackOnEnterYAMLBranch( YAML_NODE_ENTER | YAML_NODE_BRANCH , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	
	nret = _ParserYAMLMapBuffer( parser , indent ) ;
	if( nret )
		return nret;
	YAML_parser_rollback_path( parser , ypath_len ) ;

	if( parser->pfuncCallbackOnLeaveYAMLBranch ) 
	{
		nret = parser->pfuncCallbackOnLeaveYAMLBranch( YAML_NODE_LEAVE | YAML_NODE_BRANCH , parser->path.value , parser->path.value_len , parser->path.value_size , nodename , nodename_len , NULL , 0 , parser->p ) ;
		if( nret < 0 )
			return nret;
	}
	parser->block_level-- ;
	
	return 0 ;
}

int _ParserYAMLBuffer( struct YAML_parser *parser ) 
{
	int				nret = 0 ;
	
	struct YAML_token		token ;

	while(1) 
	{
		nret = YAML_fetch_next_token( parser , & token ) ;
		if( nret )
			return nret ;

		if( token.token_type == YAML_DOCUMENT_START_TOKEN ||
			token.token_type == YAML_DOCUMENT_END_TOKEN ) 
		{
			continue ;
		} 
		else if( token.token_type == YAML_STREAM_END_TOKEN ) 
		{
			break ;
		} 
		else if( token.token_type == YAML_FLOW_SEQUENCE_START_TOKEN ) 
		{
			nret = ParserYAMLArrayBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_FLOW_MAPPING_START_TOKEN ) 
		{
			nret = ParserYAMLBranchBuffer( parser , token.scalar.value , token.scalar.value_len ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_BLOCK_ENTRY_TOKEN ) 
		{
			nret = ParserYAMLSeqBuffer( parser , token.scalar.value , token.scalar.value_len , token.mark.column ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_SCALAR_TOKEN ) 
		{
			parser->pre_node = token.scalar ;
			nret = ParserYAMLMapBuffer( parser , "{" , 1 , token.mark.column ) ;
			if( nret )
				return nret ;
		} 
		else if( token.token_type == YAML_KEY_TOKEN ) 
		{
			int		indent = token.mark.column ;
			nret = YAML_fetch_next_token( parser , & token ) ;
			if( nret )
				return nret ;

			if( token.token_type != YAML_SCALAR_TOKEN )
				return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_0 ;

			parser->pre_node = token.scalar ;
			nret = ParserYAMLMapBuffer( parser , "{" , 1 , indent ) ;
			if( nret )
				return nret ;
		} 
		else
			return YAML_ERROR_PARSER_INVALID_ON_TOKEN_LEAF_0 ;
	}
	
	return 0 ;
}

int ParserYAMLBuffer4( char *yaml_buffer , int yaml_buffer_len , char *ypath , int ypath_size 
		     , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLBranch
		     , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLBranch
		     , funcCallbackOnYAMLNode *pfuncCallbackOnEnterYAMLArray
		     , funcCallbackOnYAMLNode *pfuncCallbackOnLeaveYAMLArray
		     , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLLeaf
		     , void *p )
{
	int				nret = 0 ;

	struct YAML_parser		parser ;

	YAML_parser_init( & parser ) ;
	YAML_parser_set_buffer( & parser , yaml_buffer , yaml_buffer_len ) ;
	YAML_parser_set_path( & parser , ypath , ypath_size ) ;
	YAML_parser_set_callback4( & parser 
				 , pfuncCallbackOnEnterYAMLBranch
				 , pfuncCallbackOnLeaveYAMLBranch
				 , pfuncCallbackOnEnterYAMLArray
				 , pfuncCallbackOnLeaveYAMLArray
				 , pfuncCallbackOnYAMLLeaf ) ;
	YAML_parser_set_p( & parser , p ) ;

	nret = _ParserYAMLBuffer( & parser ) ;
	if( nret == 0 )
		return 0;
	else
		return nret;
}

int ParserYAMLBuffer( char *yaml_buffer , int yaml_buffer_len , char *ypath , int ypath_size 
		    , funcCallbackOnYAMLNode *pfuncCallbackOnYAMLNode
		    , void *p )
{
	return ParserYAMLBuffer4( yaml_buffer , yaml_buffer_len , ypath , ypath_size 
				, pfuncCallbackOnYAMLNode
				, pfuncCallbackOnYAMLNode
				, pfuncCallbackOnYAMLNode 
				, pfuncCallbackOnYAMLNode
				, pfuncCallbackOnYAMLNode
				, p ) ;
}
