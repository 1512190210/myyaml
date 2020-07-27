## MYYAML - 简单YAML解析实现

[TOC]

-----------------------------------

## 1. 概述

myyaml是一个专注于YAML配置文件的SAX模式YAML解析器。myyaml不依赖于任何其它库，追求轻量快捷，易代码易嵌入。调用myyaml提供的接口可以直接解析YAML文本，调用注册的事件函数，快速访问YAML中感兴趣的内容。

目前实现yaml语法：

-   基础数据结构(纯量、序列、映射)的解析
-   基础数据结构的嵌套
-   JSON语法内嵌



## 2.编译安装

以Linux操作系统为例

```
$ cd myyaml
$ make -f makefile.Linux install
make[1]: 进入目录“$HOME/src/myyaml/src”
gcc -g -fPIC -O2 -Wall -Werror -fno-strict-aliasing -I. -I/usr/local/include -I/usr/include -c scanner.c
gcc -g -fPIC -O2 -Wall -Werror -fno-strict-aliasing -I. -I/usr/local/include -I/usr/include -c parser.c
gcc -g -fPIC -O2 -Wall -Werror -fno-strict-aliasing -o libmyyaml.so scanner.o parser.o -shared -L. -L/usr/local/lib -L/usr/lib
cp -f libmyyaml.so /root/lib/
mkdir -p /root/include/myyaml
cp -f myyaml.h /root/include/myyaml/
make[1]: 离开目录“$HOME/src/myyaml/src”
```



## 3. 使用说明



```
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
```



myyaml使用接口函数只有三个：

funcCallbackOnYAMLNode是一个函数钩子类型，在解析函数读到YAML节点事件时被调用，应用可根据type确定读到的事件类型以及当前节点路径ypath，来筛选访问感兴趣的YAML节点。

函数ParserYAMLBuffer读入YAML文本并解析之，当遇到YAML节点事件时调用funcCallbackOnYAMLNode。
函数ParserYAMLBuffer4是ParserYAMLBuffer事件细分版本。

## 4. 示例

一个简单使用示例
```
funcCallbackOnYAMLNode CallbackOnYAMLNode ;
int CallbackOnYAMLNode( int type , char *ypath , int ypath_len , int ypath_size , char *nodename , int nodename_len , char *content , int content_
len , void *p )
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
                printf( "LEAF         p[%s] ypath[%.*s] nodename[%.*s] content[%.*s]\n" , (char*)p , ypath_len , ypath , nodename_len , nodename ,
 content_len , content );
        }   
            
        return 0;
}

...
        char    ypath[ 1024 + 1 ] ;
        char    *yaml_buf = NULL ;
        int     yaml_buf_len = 0 ;
        char    *p = "hello world" ;
        
        ...
        
        memset( ypath , 0x00 , sizeof(ypath) );
        nret = ParserYAMLBuffer( yaml_buf , yaml_buf_len , ypath , sizeof(ypath) , & CallbackOnYAMLNode , p ) ;
        
        ...
```



一个简单yaml配置文件的解析

```
# cat test_basic.yml 
---
Time: 2001-11-23 15:02:31 -5
User: ed
Warning: "A slightly different error message."
---
Date: 2001-11-23 15:03:17 -5
User: ed
Fatal: Unknown variable "bar"
Stack:
  - file: TopClass.py
    line: 23
    code: x = MoreObject("345\n")
...
Mark McGwire: {hr: 65, avg: 0.278}
Sammy Sosa: {
    hr: 63,
    avg: 0.288
  }
...
---
- [name        , hr, avg  ]
- [Mark McGwire, 65, 0.278]
- [Sammy Sosa  , 63, 0.288]

# ./test_yaml test_basic.yml
ENTER-BRANCH p[hello world] ypath[] nodename[{]
LEAF         p[hello world] ypath[/Time] nodename[Time] content[2001-11-23 15:02:31 -5]
LEAF         p[hello world] ypath[/User] nodename[User] content[ed]
LEAF         p[hello world] ypath[/Warning] nodename[Warning] content[A slightly different error message.]
LEAVE-BRANCH p[hello world] ypath[] nodename[{]
ENTER-BRANCH p[hello world] ypath[] nodename[{]
LEAF         p[hello world] ypath[/Date] nodename[Date] content[2001-11-23 15:03:17 -5]
LEAF         p[hello world] ypath[/User] nodename[User] content[ed]
LEAF         p[hello world] ypath[/Fatal] nodename[Fatal] content[Unknown variable "bar"]
ENTER-ARRAY  p[hello world] ypath[/Stack] nodename[Stack]
ENTER-BRANCH p[hello world] ypath[/Stack/.] nodename[file]
LEAF         p[hello world] ypath[/Stack/.] nodename[] content[TopClass.py]
LEAF         p[hello world] ypath[/Stack/./line] nodename[line] content[23]
LEAF         p[hello world] ypath[/Stack/./code] nodename[code] content[x = MoreObject("345\n")]
LEAVE-BRANCH p[hello world] ypath[/Stack/.] nodename[file]
LEAVE-ARRAY  p[hello world] ypath[/Stack] nodename[Stack]
LEAVE-BRANCH p[hello world] ypath[] nodename[{]
ENTER-BRANCH p[hello world] ypath[] nodename[{]
ENTER-BRANCH p[hello world] ypath[/Mark McGwire] nodename[Mark McGwire]
LEAF         p[hello world] ypath[/Mark McGwire/hr] nodename[hr] content[65]
LEAF         p[hello world] ypath[/Mark McGwire/avg] nodename[avg] content[0.278]
LEAVE-BRANCH p[hello world] ypath[/Mark McGwire] nodename[Mark McGwire]
ENTER-BRANCH p[hello world] ypath[/Sammy Sosa] nodename[Sammy Sosa]
LEAF         p[hello world] ypath[/Sammy Sosa/hr] nodename[hr] content[63]
LEAF         p[hello world] ypath[/Sammy Sosa/avg] nodename[avg] content[0.288]
LEAVE-BRANCH p[hello world] ypath[/Sammy Sosa] nodename[Sammy Sosa]
LEAVE-BRANCH p[hello world] ypath[] nodename[{]
ENTER-ARRAY  p[hello world] ypath[] nodename[-]
ENTER-ARRAY  p[hello world] ypath[] nodename[[]
LEAF         p[hello world] ypath[/.] nodename[[] content[name]
LEAF         p[hello world] ypath[/.] nodename[[] content[hr]
LEAF         p[hello world] ypath[/.] nodename[[] content[avg]
LEAVE-ARRAY  p[hello world] ypath[] nodename[[]
ENTER-ARRAY  p[hello world] ypath[] nodename[[]
LEAF         p[hello world] ypath[/.] nodename[[] content[Mark McGwire]
LEAF         p[hello world] ypath[/.] nodename[[] content[65]
LEAF         p[hello world] ypath[/.] nodename[[] content[0.278]
LEAVE-ARRAY  p[hello world] ypath[] nodename[[]
ENTER-ARRAY  p[hello world] ypath[] nodename[[]
LEAF         p[hello world] ypath[/.] nodename[[] content[Sammy Sosa]
LEAF         p[hello world] ypath[/.] nodename[[] content[63]
LEAF         p[hello world] ypath[/.] nodename[[] content[0.288]
LEAVE-ARRAY  p[hello world] ypath[] nodename[[]
LEAVE-ARRAY  p[hello world] ypath[] nodename[-]
```





## 5.TODO

-   不同操作环境兼容性实现及编码区分

-   复杂语法解析实现

-   YAML的序列化及反序列化

    

## 6.参考

-   [The Official *YAML* Web Site](https://yaml.org/)
-   [libyaml - "C" Fast YAML 1.1](https://github.com/yaml/libyaml)
-   [fasterjson](https://github.com/calvinwilliams/fasterjson/blob/master/README-CN)