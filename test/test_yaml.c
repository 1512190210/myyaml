#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "myyaml.h"

void usage()
{
	printf("USAGE : test_yaml yaml_filename\n") ;
}

static int _GetFileSize(char *filename)
{
	struct stat stat_buf;
	int ret;

	ret=stat(filename,&stat_buf);
	
	if( ret == -1 )
		return -1;
	
	return stat_buf.st_size;
}

static int ReadEntireFile( char *filename , char *mode , char *buf , int *bufsize )
{
	FILE	*fp = NULL ;
	int	filesize ;
	long	lret ;
	
	if( filename == NULL )
		return -1;
	if( strcmp( filename , "" ) == 0 )
		return -1;
	
	filesize = _GetFileSize( filename ) ;
	if( filesize  < 0 )
		return -2;
	
	fp = fopen( filename , mode ) ;
	if( fp == NULL )
		return -3;
	
	if( filesize <= (*bufsize) )
	{
		lret = fread( buf , sizeof(char) , filesize , fp ) ;
		if( lret < filesize )
		{
			fclose( fp );
			return -4;
		}
		
		(*bufsize) = filesize ;
		
		fclose( fp );
		
		return 0;
	}
	else
	{
		lret = fread( buf , sizeof(char) , (*bufsize) , fp ) ;
		if( lret < (*bufsize) )
		{
			fclose( fp );
			return -4;
		}
		
		fclose( fp );
		
		return 1;
	}
}

static int ReadEntireFileSafely( char *filename , char *mode , char **pbuf , int *pbufsize )
{
	int	filesize ;
	
	int	nret ;
	
	filesize = _GetFileSize( filename );
	
	(*pbuf) = (char*)malloc( filesize + 1 ) ;
	if( (*pbuf) == NULL )
		return -1;
	memset( (*pbuf) , 0x00 , filesize + 1 );
	
	nret = ReadEntireFile( filename , mode , (*pbuf) , & filesize ) ;
	if( nret )
	{
		free( (*pbuf) );
		(*pbuf) = NULL ;
		return nret;
	}
	else
	{
		if( pbufsize )
			(*pbufsize) = filesize ;
		return 0;
	}
}

funcCallbackOnYAMLNode CallbackOnYAMLNode ;
int CallbackOnYAMLNode( int type , char *ypath , int ypath_len , int ypath_size , char *nodename , int nodename_len , char *content , int content_len , void *p )
{
	if( type & YAML_NODE_BRANCH )
	{
		if( type & YAML_NODE_ENTER )
		{
			printf( "ENTER-BRANCH p[%s] ypath[%.*s] nodename[%.*s]\n" , (char*)p , ypath_len , ypath , nodename_len , nodename );
		}
		else if( type & YAML_NODE_LEAVE )
		{
			printf( "LEAVE-BRANCH p[%s] ypath[%.*s] nodename[%.*s]\n" , (char*)p , ypath_len , ypath , nodename_len , nodename );
		}
	}
	else if( type & YAML_NODE_ARRAY )
	{
		if( type & YAML_NODE_ENTER )
		{
			printf( "ENTER-ARRAY  p[%s] ypath[%.*s] nodename[%.*s]\n" , (char*)p , ypath_len , ypath , nodename_len , nodename );
		}
		else if( type & YAML_NODE_LEAVE )
		{
			printf( "LEAVE-ARRAY  p[%s] ypath[%.*s] nodename[%.*s]\n" , (char*)p , ypath_len , ypath , nodename_len , nodename );
		}
	}
	else if( type & YAML_NODE_LEAF )
	{
		printf( "LEAF         p[%s] ypath[%.*s] nodename[%.*s] content[%.*s]\n" , (char*)p , ypath_len , ypath , nodename_len , nodename , content_len , content );
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	char	ypath[ 1024 + 1 ] ;
	char	*yaml_buf = NULL ;
	int	yaml_buf_len = 0 ;
	char	*p = "hello world" ;
	
	if( argc >= 1 + 1 )
	{
		int		nret = 0 ;
		
		if( argc == 1 + 1 )
		{
			nret = ReadEntireFileSafely( argv[1] , "rb" , & yaml_buf , & yaml_buf_len ) ;
			if( nret )
			{
				printf( "ReadEntireFileSafely[%s] failed[%d]\n" , argv[1] , nret );
				return nret;
			}
		}
		else
		{
			usage() ;
			return -1 ;
		}
		
		memset( ypath , 0x00 , sizeof(ypath) );
		nret = ParserYAMLBuffer( yaml_buf , yaml_buf_len , ypath , sizeof(ypath) , & CallbackOnYAMLNode , p ) ;
		free( yaml_buf );
		if( nret )
		{
			printf( "TravelYAMLTree failed[%d]\n" , nret );
			return -nret;
		}
	}
	else
	{
		printf( "USAGE : test_fastYAML YAML_pathfilename\n" );
		exit(7);
	}
	
	return 0;
}
