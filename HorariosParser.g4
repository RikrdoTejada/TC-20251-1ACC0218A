parser grammar HorariosParser;
options { tokenVocab = HorariosLexer; }

// ─── REGLAS DE PARSER ─────────────────────────────
program
    : (profesorDSL | cursoDSL | aulaDSL | bloqueHorario)+ EOF
    ;

// ejemplo: profesor P001 { nombre: "…" }
profesorDSL
    : PROFESOR ID LLAVEI NOMBRE DOSPUNTOS STRING LLAVED
    ;

// ejemplo: curso C101 { codigo: "…", profesor: P001, horas: 3 }
cursoDSL
    : CURSO ID LLAVEI
        CODIGO   DOSPUNTOS STRING COMA
        PROFESOR DOSPUNTOS ID     COMA
        HORAS    DOSPUNTOS INT
      LLAVED
    ;

// ejemplo: aula A101 { capacidad: 40 }
aulaDSL
    : AULA ID LLAVEI CAPACIDAD DOSPUNTOS INT LLAVED
    ;

// horario { … }
bloqueHorario
    : HORARIO LLAVEI entradaHorario+ LLAVED
    ;

// ejemplo: C101 @ A101 en LUNES 08:00-10:00 ;
entradaHorario
    : ID ARROBA ID EN DIA rangoHora PUNTOCOMA
    ;

// ejemplo: 08:00-10:00
rangoHora
    : HORA GUION HORA
    ;
