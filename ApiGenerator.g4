grammar ApiGenerator;

program : apiDef EOF ; // Regla principal del programa;

apiDef
    : API API_N '{' endpoint (',' endpoint)* '}' #apiDefinition // Definición de la API
;
endpoint
    : DEF 'get' STRING '{' response '}' #getEndpoint
    | DEF 'post' STRING '{' params response '}' #postEndpoint
    | DEF 'put' STRING '{' params response '}' #putEndpoint
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
// Definición de tokens y reglas léxicas
API     : 'api' ;
DEF     : 'def' ;
API_N   : '"'[a-zA-Z_][a-zA-Z_0-9]*'"';     // "API_FLASK1"
STRING  : '"' (~["\r\n])* '"' ;             // "ruta/endpoint"
ID      : [a-zA-Z_][a-zA-Z_0-9]* ;          // nombre, edad
INT     : [0-9]+ ;                          // Números enteros
FLOAT   : [0-9]+ ('.' [0-9]+)? ;            // Números decimales
COMMENT : '//' ~[\r\n]* -> skip ;
WS      : [ \t\r\n]+    -> skip ;


