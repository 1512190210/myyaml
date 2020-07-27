#include "myyaml.h"
#include "myyaml_in.h"

#define CACHE(parser,length)                                                    \
    (parser->unread >= (length)                                                 \
        ? 1                                                                     \
        : 0)

#define SKIP(parser)                                                            \
     (parser->mark.index ++,                                                    \
      parser->mark.column ++,                                                   \
      parser->buffer.pointer += WIDTH(parser->buffer))

#define SKIP_N(parser , len)                                                    \
     (parser->mark.index += len,                                                \
      parser->mark.column += len,                                               \
      parser->buffer.pointer += len)

#define SKIP_LINE(parser)                                                       \
     (IS_BREAK(parser->buffer) ?                                                \
      (parser->mark.index ++,                                                   \
       parser->mark.column = 0,                                                 \
       parser->mark.line ++,                                                    \
       parser->unread --,                                                       \
       parser->buffer.pointer += WIDTH(parser->buffer)) : 0)

void YAML_init_token( struct YAML_token *token )
{
        memset( token , 0x00 , sizeof(struct YAML_token)) ;
}

int YAML_set_token_type( struct YAML_token *token , YAML_token_type_t type )
{
        token->token_type = type ;
        return 0 ;
}

int YAML_scan_to_next_token( struct YAML_parser *parser , struct YAML_token *token )
{
        while (1)
        {
                if( !CACHE(parser, 1) )
                       return YAML_set_token_type( token , YAML_STREAM_END_TOKEN ) ;

                /*
                 * 去掉空�/tab
                 * tab可以出现的位置:
                 *  - flow内部
                 *  - 缩进时不能使用tab
                 */
                while( CHECK(parser->buffer , ' ') ||
                    ( ( parser->flow_level || parser->key_allow )
                     && CHECK(parser->buffer , '\t') ) )
                {
                        SKIP( parser ) ;
                        if( !CACHE(parser, 1) )
                                return YAML_set_token_type( token , YAML_STREAM_END_TOKEN ) ;
                }

                /* 去掉#后整行注释 */
                if( CHECK(parser->buffer , '#') )
		{
                        while( !IS_BREAKZ(parser->buffer) )
			{
                                SKIP( parser ) ;
                                if( !CACHE(parser, 1) )
                                        return YAML_set_token_type( token , YAML_STREAM_END_TOKEN ) ;
			}
		}

                /* 去掉换行 */
                if( IS_BREAK(parser->buffer) )
                {
                        /* TODO: add IS_CRLF. */
                        if( !CACHE(parser, 1) )
                                return YAML_set_token_type( token , YAML_STREAM_END_TOKEN ) ;
                        SKIP_LINE( parser );
			parser->key_allow = 0 ;
                }
                else
                        break;
        }
        return 0 ;
}

int YAML_scan_flow_scalar( struct YAML_parser *parser , struct YAML_token *token , char quote )
{
	int offset = 0 ;
	while( 1 ) 
	{
		/* 文件结束 */
		if( IS_BREAKZ_AT(parser->buffer , offset) )
			return YAML_ERROR_SCANNER_QUOTE_NOT_MATCH ;

		/* 引号 */
		if( CHECK_AT(parser->buffer , quote , offset) )
			break ;
		offset += WIDTH_AT(parser->buffer , offset) ;
	}
	
	token->scalar.value = parser->buffer.pointer ;
	token->scalar.value_len = offset ;
	SKIP_N( parser , offset ) ;
	return 0;
}

int YAML_scan_plain_scalar( struct YAML_parser *parser , struct YAML_token *token ) 
{
	int offset = 0 ;
	int pre_read ;
	while( 1 ) 
	{
		/* 空�或tab间断预读 */
		pre_read = 0 ;
		while( IS_BLANK_AT(parser->buffer , offset + pre_read) ) 
		{
			pre_read++ ;
			if( !CACHE(parser, offset + pre_read) )
				break ;
		}
		/* 文件结束 */
		if( IS_BREAKZ_AT(parser->buffer , offset + pre_read) )
			break ;

		if( parser->flow_level && strchr(",[]{}" , *(parser->buffer.pointer + offset + pre_read)) )
			break ;

		if( pre_read && CHECK_AT( parser->buffer , '#', offset + pre_read) &&
			IS_BLANKZ_AT(parser->buffer, 1 + offset + pre_read) )
			break ;

		if( CHECK_AT(parser->buffer , ':' , offset + pre_read) &&
			(parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1 + offset + pre_read)) )
			break ;
		
		offset += pre_read ? pre_read : WIDTH_AT(parser->buffer , offset) ;
	}
	token->scalar.value = parser->buffer.pointer ;
	token->scalar.value_len = offset ;
	SKIP_N( parser , offset ) ;
	return 0;
}

int YAML_unroll_indent( struct YAML_parser *parser , struct YAML_token *token )
{

        /* flow的层次语法不依赖缩进 直接返回结果. */
        if ( parser->flow_level )
                return 0 ;

        if ( parser->indent > parser->mark.column )
                return YAML_set_token_type( token , YAML_BLOCK_END_TOKEN ) ;

        return 0 ;
}

int YAML_fetch_document_token( struct YAML_parser *parser , struct YAML_token *token , 
                                                                    YAML_token_type_t token_type )
{
        token->scalar.value = parser->buffer.pointer ;
        token->scalar.value_len = 3 ;

        if( parser->flow_level == 0 && parser->block_level == 0 )
        {
                SKIP(parser);
                SKIP(parser);
                SKIP(parser);
        }
        return YAML_set_token_type( token , token_type ) ;
}

int YAML_fetch_flow_collection_token( struct YAML_parser *parser , struct YAML_token *token , 
                                                                    YAML_token_type_t token_type )
{
        token->scalar.value = parser->buffer.pointer ;
        token->scalar.value_len = 1 ;

        SKIP(parser);

        return YAML_set_token_type( token , token_type ) ;
}

int YAML_fetch_flow_entry( struct YAML_parser *parser , struct YAML_token *token )
{
        SKIP(parser);

        return YAML_set_token_type( token , YAML_FLOW_ENTRY_TOKEN ) ;
}

int YAML_fetch_block_entry( struct YAML_parser *parser , struct YAML_token *token )
{
        token->scalar.value = parser->buffer.pointer ;
        token->scalar.value_len = 1 ;

        SKIP(parser);

        return YAML_set_token_type( token , YAML_BLOCK_ENTRY_TOKEN ) ;
}

int YAML_fetch_key( struct YAML_parser *parser , struct YAML_token *token )
{
        SKIP(parser);

        return YAML_set_token_type( token , YAML_KEY_TOKEN ) ;
}

int YAML_fetch_value( struct YAML_parser *parser , struct YAML_token *token )
{
        parser->key_allow = 1 ;

        SKIP(parser);

        return YAML_set_token_type( token , YAML_VALUE_TOKEN ) ;
}

int YAML_fetch_flow_scalar( struct YAML_parser *parser , struct YAML_token *token , char quote )
{
        int                         nret = 0 ;

        SKIP(parser) ;
        nret = YAML_scan_flow_scalar( parser , token , quote ) ;
        if( nret )
            return nret ;
        SKIP(parser) ;

        return YAML_set_token_type( token , YAML_SCALAR_TOKEN ) ;
}

int YAML_fetch_plain_scalar( struct YAML_parser *parser , struct YAML_token *token )
{
        int                         nret = 0 ;

        nret = YAML_scan_plain_scalar( parser , token ) ;
        if( nret )
            return nret ;

        parser->key_allow = 0 ;

        return YAML_set_token_type( token , YAML_SCALAR_TOKEN ) ;
}

int YAML_fetch_next_token( struct YAML_parser *parser , struct YAML_token *token )
{
        int                         nret = 0 ;

        YAML_init_token( token ) ;

        if( !CACHE(parser, 1) )
                return YAML_set_token_type( token , YAML_STREAM_END_TOKEN ) ;

        nret = YAML_scan_to_next_token( parser , token ) ;
        if( nret )
                return nret ;
        else if( token->token_type )
                return 0 ;

        /* 通过缩进判断block是否结束. */
        YAML_unroll_indent( parser , token ) ;
        if( token->token_type )
                return 0 ;

        token->mark = parser->mark ;
	token->scalar.mark = parser->mark ;

        /* 判断stream结束 */
        if( IS_Z(parser->buffer) )
                return YAML_set_token_type( token , YAML_STREAM_END_TOKEN ) ;

        /*
         * 判断最长特殊字符 文档开始 文档结束 4字节 ('--- ' 和 '... ').
         */
        if( CACHE(parser, 4) )
        {
            if( parser->mark.column == 0
                && CHECK_AT( parser->buffer , '-' , 0 )
                && CHECK_AT( parser->buffer , '-' , 1 )
                && CHECK_AT( parser->buffer , '-' , 2 )
                && IS_BLANKZ_AT( parser->buffer , 3 ) )
                return YAML_fetch_document_token( parser, token , 
                                                          YAML_DOCUMENT_START_TOKEN );
            if (parser->mark.column == 0
                    && CHECK_AT(parser->buffer , '.' , 0)
                    && CHECK_AT(parser->buffer , '.' , 1)
                    && CHECK_AT(parser->buffer , '.' , 2)
                    && IS_BLANKZ_AT(parser->buffer , 3))
                return YAML_fetch_document_token( parser , token , 
                                                          YAML_DOCUMENT_END_TOKEN );
        }

        /* flow类型结构 */
        if( CHECK(parser->buffer , '[') )
                return YAML_fetch_flow_collection_token( parser , token , 
                                                          YAML_FLOW_SEQUENCE_START_TOKEN );

        if( CHECK(parser->buffer , '{') )
                return YAML_fetch_flow_collection_token( parser , token , 
                                                          YAML_FLOW_MAPPING_START_TOKEN );

        if( CHECK(parser->buffer , ']') )
                return YAML_fetch_flow_collection_token( parser , token , 
                                                          YAML_FLOW_SEQUENCE_END_TOKEN );

        if( CHECK(parser->buffer , '}') )
                return YAML_fetch_flow_collection_token( parser , token , 
                                                          YAML_FLOW_MAPPING_END_TOKEN );

        if( CHECK(parser->buffer , ',') )
                return YAML_fetch_flow_entry( parser, token );

        /* block队列 */
        if( CHECK(parser->buffer , '-') && IS_BLANKZ_AT(parser->buffer , 1) )
            return YAML_fetch_block_entry( parser , token );

        /* key */
        if( CHECK(parser->buffer , '?')
                && (parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1)))
            return YAML_fetch_key( parser , token );

        /* value */
        if( CHECK(parser->buffer , ':')
                && (parser->flow_level || IS_BLANKZ_AT(parser->buffer, 1)))
            return YAML_fetch_value( parser , token );

        /* scalar */
        if( CHECK(parser->buffer , '\'' ))
            return YAML_fetch_flow_scalar( parser , token , '\'' );

        if( CHECK(parser->buffer , '"') )
            return YAML_fetch_flow_scalar( parser , token , '"' );

        if ( !strchr("&*!|>%@`" , *parser->buffer.pointer) )
            return YAML_fetch_plain_scalar(parser , token );

        return YAML_ERROR_SCANNER_INVALID ;
}

