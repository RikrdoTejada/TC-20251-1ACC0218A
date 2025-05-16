lexer grammar HorariosLexer;

// ─── PALABRAS CLAVE ───────────────────────────────
PROFESOR  : 'profesor' ;
CURSO     : 'curso' ;
AULA      : 'aula' ;
NOMBRE    : 'nombre' ;
CODIGO    : 'codigo' ;
HORAS     : 'horas' ;
CAPACIDAD : 'capacidad' ;
HORARIO   : 'horario' ;
EN        : 'en' ;

// ─── SÍMBOLOS ─────────────────────────────────────
ARROBA      : '@' ;
COMA        : ',' ;
DOSPUNTOS   : ':' ;
PUNTOCOMA   : ';' ;
LLAVEI      : '{' ;
LLAVED      : '}' ;
GUION       : '-' ;

// ─── LÉXICOS ──────────────────────────────────────
DIA   : 'LUNES' | 'MARTES' | 'MIERCOLES' | 'JUEVES' | 'VIERNES' | 'SABADO' ;
HORA  : DIGITO DIGITO ':' DIGITO DIGITO ;          // HH:MM  (24 h)
ID    : [a-zA-Z_] [a-zA-Z_0-9]* ;
INT   : [0-9]+ ;
STRING: '"' (~["\r\n])* '"' ;

// ─── ESPACIOS Y COMENTARIOS ───────────────────────
COMMENT : '//' ~[\r\n]* -> skip ;
WS      : [ \t\r\n]+    -> skip ;

fragment DIGITO : [0-9] ;
