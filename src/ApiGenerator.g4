grammar ApiGenerator;

program : apiDef EOF ; // Regla principal del programa;

apiDef
    : API API_N '{' endpoint (',' endpoint)* '}' #apiDefinition // Definición de la API
;
endpoint
    : DEF 'get' ENDPOINT '{' search response '}' #getEndpoint
    | DEF 'post' ENDPOINT '{' params response '}' #postEndpoint
    | DEF 'put' ENDPOINT '{' params response '}' #putEndpoint
;
params
    : 'params' ':' idList
;

idList
    : ID (',' ID)*
;

response
    : 'response' ':' STRING ('with' ID (',' ID)*)? #responseContent
;

search
    : 'search' ':' COLLECTIONDB ('with' ID (',' ID)*)? #searchContent
;


// Definición de tokens y reglas léxicas
API     : 'api' ;
DEF     : 'def' ;
API_N   : '"'[a-zA-Z_][a-zA-Z_0-9]*'"';     // "API_FLASK1"
ENDPOINT : '"' '/' [a-zA-Z_][a-zA-Z0-9_/]* '"' ;
STRING : '"' (~["\r\n])* '"' ;
COLLECTIONDB : '%' [a-zA-Z_][a-zA-Z0-9_/]* ;
ID      : [a-zA-Z_][a-zA-Z_0-9]* ;          // nombre, edad
INT     : [0-9]+ ;                          // Números enteros
FLOAT   : [0-9]+ ('.' [0-9]+)? ;            // Números decimales
COMMENT : '//' ~[\r\n]* -> skip ;
WS      : [ \t\r\n]+    -> skip ;


