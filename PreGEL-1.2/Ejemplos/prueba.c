/*****************************************
* Autores:
* 
*   Fabian Diaz Lorenzo   alu0100702545@ull.edu.es
*   Ricardo Pérez Castillo  alu0100832976@ull.edu.es
* 
* Proyecto:
*       Este proyecto se basa en la creacion de un relog que puedas
*       configurar la hora y que puedas configurar una alarma.
* 
* 
* 
******************************************** */

#define DEBUG 0

#include <sys/param.h>
#include <sys/interrupts.h>
#include <sys/sio.h>
#include <sys/locks.h>

typedef unsigned char byte;  /*por comodidad*/
typedef unsigned short word;  /*por comodidad*/

/*Acceso a IO PORTS como palabra*/
#define _IO_PORTS_W(d)  (((unsigned volatile short*) & _io_ports[(d)])[0])

#define NOP __asm__ __volatile__ ( "nop" )

/* Puerto de datos del teclado */
#define P_TEC   M6812_PORTG
#define P_TEC_DDR   M6812_DDRG

/* Puerto de datos del display */
#define P_DATOS   M6812_PORTH
#define P_DATOS_DDR   M6812_DDRH
/* Puerto de control de dislplay */
#define P_CONT    M6812_PORTT
#define P_CONT_DDR    M6812_DDRT

/* Bits de control del display */
#define B_EN      M6812B_PT7
#define B_RW      M6812B_PT6
#define B_RS      M6812B_PT5

/* Bits de control del display */
#define CLEAR     ( 1 )
#define SALIDA	  (0xff)
#define SALIDA_CONT (0xe0)
#define INI_TEC (F1|F2|F3|F4)

#define RETURN    ( 1 << 1 )

#define CUR_INC    ( ( 1 << 2 ) | ( 1 << 1 ) )
#define SHIFT    ( ( 1 << 2 ) | 1 )

#define DISP_OFF    ( ( 1 << 3 ) )
#define DISP_ON    ( ( 1 << 3 ) | ( 1 << 2 ) )
#define CUR_ON      ( ( 1 << 3 ) | ( 1 << 1 ) )
#define CUR_BLIK      ( ( 1 << 3 ) | ( 1 ) )
#define PARPADEO    ( ( 1 << 3 ) | ( 1 << 2 ) | ( 1 << 1 ) | ( 1 ))
#define SHIFT_DISP  ( ( 1 << 4 ) | ( 1 << 3 ) )
#define SHIFT_LEFT  ( ( 1 << 4 ) | ( 1 << 2 ) )
#define SHIFT_RIGHT  ( ( 1 << 4 ) )

#define DL_8BITS   ( ( 1 << 5 ) | ( 1 << 4 ) )
#define DOS_FILAS   ( ( 1 << 5 ) | ( 1 << 3 ) )
#define FUENTE_5X10   ( ( 1 << 5 ) | ( 1 << 2 ) )
#define LINE2   ( ( 1 << 7 ) | ( 1 << 6 ) )
#define F1 (1 << 5)
#define F2 (1)
#define F3 (1 << 1)
#define F4 (1 << 3)
#define C1 M6812B_PG4
#define C2 M6812B_PG6
#define C3 M6812B_PG2
#define  TCM_FACTOR ( 7 )  /*La potencia de 2 a aplicar al factor*/
#define  TCM_FREQ ( M6812_CPU_E_CLOCK / ( 1 << TCM_FACTOR ) )
/*Pasa de microsegundos a ticks*/
#define  USG_2_TICKS(us)  ( ( us ) * ( TCM_FREQ / 1000000L ) )
/*Pasa de milisegundos a ticks*/
#define  MSG_2_TICKS(ms)  ( ( ms ) * ( TCM_FREQ / 1000L ) )

unsigned short Periodo; /* Ticks del temporizador que dura el periodo */
unsigned short horas; /* Se incremente en cada interrupción */
unsigned short minutos;
unsigned short segundos;
char numero[4];
unsigned short tamano;
unsigned short contador_alarma;
unsigned short hora_alarma;
unsigned short minutos_alarma;
unsigned short habilitado;



/*
   función que realiza un retarso del el número de microsegundos indicados
   en el parámetro usg
   Utiliza canal 6 del temporizador
*/
void delayusg( unsigned long useg ) {
    unsigned int numCiclos;
    unsigned long numCiclosL;

    /* Desconectamos para no afectar al pin */
    _io_ports[ M6812_TCTL1 ] &= ~(M6812B_OM6 | M6812B_OL6);

    /* Vemos velocidad del temporizador*/
    byte factorT = _io_ports[ M6812_TMSK2 ] & 0x07; /*Factor de escalado actual*/
    unsigned long frec = M6812_CPU_E_CLOCK/( 1 << factorT ); /* Frecuencia del temporizador*/
    /* Según la frecuencia elegimos el modo de dividir para evitar desbordamientos */
    if( frec/1000000 )
        numCiclosL = frec/1000000 * useg;
    else
        numCiclosL = frec/100 * useg/10000;

    unsigned int numDisparos = numCiclosL >> 16;  /* Numero de disparos necesarios */
    numCiclos = numCiclosL & 0xffff; /* Número restante de ciclos */

    /* Por si escalado muy grande y useg pequeño */
    if( ( numCiclos == 0 ) && ( numDisparos == 0 ) ) numCiclos = 1;

    _io_ports[ M6812_TIOS ] |= M6812B_IOS6; /*configuramos canal como comparador salida*/
    _io_ports[ M6812_TFLG1 ] = M6812B_C6F; /*Bajamos el banderín  */
    /*preparamos disparo*/
    _IO_PORTS_W( M6812_TC6 ) = _IO_PORTS_W( M6812_TCNT ) + numCiclos ;

    /*Habilitamos el temporizador, por si no lo está*/
    _io_ports[ M6812_TSCR ] |= M6812B_TEN;

    /* Esparamos los desboradmientos necesarios */
    do {
        /* Nos quedamos esperando a que se produzca la igualdad*/
        while ( ! ( _io_ports[ M6812_TFLG1 ] & M6812B_C6F ) );
        _io_ports[ M6812_TFLG1 ] = M6812B_C6F; /* Bajamos el banderín */
    } while( numDisparos-- );
}

/* función que genera un cilo de la señal E para realizar un acceso al display
   los valores de las señales RW, RS y datos deben de fijarse antes de llamar a esta
   función */
void cicloAcceso( ) {
    NOP;
    NOP;
    NOP;
    _io_ports[ P_CONT ] |= B_EN; /* subimos señal E */
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    _io_ports[ P_CONT ] &= ~B_EN; /* bajamos señal E */
    NOP;
    NOP;
    NOP;
}

/* Envía un byte como comando */
void enviaComando( byte b ) {
    /* Ciclo de escritura con RS = 0 */
    _io_ports[ P_CONT ] &= ~( B_RS | B_RW );
    _io_ports[ P_DATOS ] = b;

    cicloAcceso( );

    /* Esperamos el tiempo que corresponda */
    if( ( b == 1 ) || ( ( b & 0xfe ) == 2 ) )
        delayusg( 1520UL );
    else
        delayusg( 37 );
    //delayusg( 2000000UL );
}

/* Envía un byte como dato */
void enviaDato( byte b ) {
    /* Ciclo de escritura con RS = 1 */
    _io_ports[ P_CONT ] &= ~B_RW;
    _io_ports[ P_CONT ] |= B_RS;
    _io_ports[ P_DATOS ] = b;

    cicloAcceso( );

    /* Esperamos el tiemp que corresponde */
    delayusg( 37 );
}

void inicializaDisplay( ) {

    /*======== Configurar puertos del display como salida ============= */
    
    _io_ports[ P_DATOS_DDR ] = SALIDA;
    _io_ports[ P_CONT_DDR ] |= SALIDA_CONT;
    

    delayusg( 15000UL );
    enviaComando( DL_8BITS );//00110000
    
    delayusg( 4100UL );
    enviaComando( DL_8BITS );//00110000
    
    delayusg( 100UL );
    enviaComando( DL_8BITS );//00110000
    
    enviaComando( DL_8BITS | DISP_ON );//00111100
    enviaComando( DISP_OFF );//00001000
    enviaComando( CLEAR  ); //00000001
    enviaComando( CUR_INC );//00000110
    



    /* Encendemos display con cursor parpadeante */
    enviaComando( DISP_ON | CUR_ON );//1110

    /*Sacamos mensaje */
    
    enviaDato( 'H' );
    enviaDato( 'o' );
    enviaDato( 'l' );
    enviaDato( 'a' );


}

char leer_tec(){
	_io_ports[ P_TEC ] = INI_TEC;

	while ( ((_io_ports[ P_TEC ]) & (C1|C2|C3))){}
	delayusg( 20000UL );
	//algo
	while ( !((_io_ports[ P_TEC ]) & (C1|C2|C3))){}
    //algo
	delayusg( 20000UL );
	_io_ports[ P_TEC ] = F1;
	if (_io_ports[ P_TEC ] & C1)
	    return '1';
	if (_io_ports[ P_TEC ] & C2)
	    return '2';
	if (_io_ports[ P_TEC ] & C3)
	    return '3';
	
	_io_ports[ P_TEC ] = F2;
	if (_io_ports[ P_TEC ] & C1)
	    return '4';
	if (_io_ports[ P_TEC ] & C2)
	    return '5';
	if (_io_ports[ P_TEC ] & C3)
	    return '6';
	
	_io_ports[ P_TEC ] = F3;
	if (_io_ports[ P_TEC ] & C1)
	    return '7';
	if (_io_ports[ P_TEC ] & C2)
	    return '8';
	if (_io_ports[ P_TEC ] & C3)
	    return '9';
	
	_io_ports[ P_TEC ] = F4;
	if (_io_ports[ P_TEC ] & C1)
	    return '*';
	if (_io_ports[ P_TEC ] & C2)
	    return '0';
	if (_io_ports[ P_TEC ] & C3)
	    return '#';
    return 'P';
}


void sacaDisplay( char c ) {

    //serial_printdecbyte( c );
    //char cod_ascii =c+'0';
     /*========= Implementar el código de la función de gestión =============== */
    switch ( c ){
        case  2  :
	    serial_print( "\n$ entra en 2 $\n" );
            enviaComando( CLEAR );
            break;
        case  8  :
            enviaComando( RETURN );
            break;
        case  11 :
            enviaComando( SHIFT_RIGHT );//0000010000
            break;
        case  12 :
            enviaComando( SHIFT_LEFT);//0000010100
            break;
        case  15 :
            enviaComando( DISP_OFF  );//0000001000
            break;
        case  3  :
            enviaComando( DISP_ON );//0000001100
            break;
        case  16 :
            enviaComando( PARPADEO );//0000001111
            break;
        default  :
            enviaDato( c );
            break;
     }

}

void imp_tec( char c ){
	  sacaDisplay( c );
}


void ini_tec(){
  _io_ports[ P_TEC_DDR ] = INI_TEC;
  
}

void imp_cadena_reloj(char * c){
    unsigned short i;
    for (i=0;i<4; i++){
        sacaDisplay(c[i]);
        if (i%2!=0 && i<3)
                    sacaDisplay(':');
        
    }
    
}

void imp_cadena(char * c, unsigned short tamano){
    unsigned short i;
    for (i=0;i<tamano; i++){
        sacaDisplay(c[i]);
        
    }
    
}



int main ( ) {
    horas=0; /* Se incremente en cada interrupción */
    minutos=0;
    segundos=0;
    contador_alarma=30;
    tamano=0;
    habilitado=0;
    hora_alarma=31;
    minutos_alarma=0;
    /* Deshabilitamos interrupciones */
    lock ( );

    /* Inicializamos la serial */
    serial_init( );
    //serial_print( "\n$Id: EsqueletoDisplay.c $\n" );


    /*Encendemos led*/
    _io_ports[ M6812_DDRG ] |= M6812B_PG7;
    _io_ports[ M6812_PORTG ] |= M6812B_PG7;

    /* Iniciamos periodo según microsegundos que queremos que dure */
    Periodo = MSG_2_TICKS(1000UL);  /* se hace en tiempo de COMPILACION */
    //serial_print( "\r\n usg del periodo: " );
    //serial_printdecword( Periodo/USG_2_TICKS( 1 ) );

    /*Inicialización del Temporizador*/
    _io_ports[ M6812_TMSK2 ] = TCM_FACTOR;

    /* OC2 Invierte el pin en cada disparo */
    _io_ports[ M6812_TCTL2 ] &= ~M6812B_OM2;
    _io_ports[ M6812_TCTL2 ] |= M6812B_OL2;

    /*preparamos disparo*/
    _IO_PORTS_W( M6812_TC2 ) = _IO_PORTS_W( M6812_TCNT ) +Periodo;


    /*configuramos canal 2 como comparador salida*/
    _io_ports[ M6812_TIOS ] |= M6812B_IOS2;


    _io_ports[ M6812_TFLG1 ] = M6812B_IOS2; /*Bajamos el banderín de OC2 */
    _io_ports[ M6812_TMSK1 ] |= M6812B_IOS2; /*habilitamos sus interrupciones*/
    _io_ports[ M6812_TSCR ] = M6812B_TEN; /*Habilitamos temporizador*/

    
    ini_tec( );

    inicializaDisplay( );

    unlock( ); /* habilitamos interrupciones */
    serial_print( "\n\rTerminada inicialización\n" );

    while( 1 ) {
        char c;
        //c = serial_recv( );
        //serial_send( c ); /* Hacemos eco para confirmar la recepción */
	serial_print( "\n\rTerminada inicialización\n" );
        //sacaDisplay( c );
        
	    c= leer_tec();
	    switch (c){
	        case '#':
	            lock ( );
	            _io_ports[ M6812_TMSK2 ] = TCM_FACTOR;
                _io_ports[ M6812_TCTL2 ] &= ~M6812B_OM2;
                _io_ports[ M6812_TCTL2 ] |= M6812B_OL2;
                _IO_PORTS_W( M6812_TC2 ) = _IO_PORTS_W( M6812_TCNT ) + Periodo;
                _io_ports[ M6812_TIOS ] |= M6812B_IOS2;
                _io_ports[ M6812_TFLG1 ] = M6812B_IOS2; 
                _io_ports[ M6812_TMSK1 ] |= M6812B_IOS2; 
                _io_ports[ M6812_TSCR ] = M6812B_TEN;
	            c=leer_tec();
	            enviaComando( LINE2 );
	             imp_cadena("CLK ", 4);
	            
	            if (c !='#' && c !='*'){
	                    horas=0;
	                    horas += (c-'0') ;
	                    sacaDisplay(c);
	                    c=leer_tec();
	                    if (c !='#' && c !='*'){
	                        horas *=10;  
	                        horas += (c-'0');
	                        sacaDisplay(c);
	                    }
	                    if (horas >=24)
	                        horas=0;
	            }
	            sacaDisplay(':' ) ;
	            c=leer_tec();
	            if (c !='#' && c !='*'){
	                    minutos=0;
	                    minutos += (c-'0') ;
	                    sacaDisplay(c);
	                    c=leer_tec();
	                    if (c !='#' && c !='*'){
	                        minutos *=10;  
	                        minutos += (c-'0');
	                        sacaDisplay(c);
	                    }
	                    if (minutos>=60)
	                         minutos=0;
	            }  
	            segundos=0;
	            unlock( );
	        break;
	        case '0':
    	        habilitado=1;
    	        c=leer_tec();
    	        enviaComando( LINE2 );
    	        imp_cadena("ALR ", 4);
    	        if (c !='#'){
    	            if ( c !='*'){
    	                   hora_alarma=0;
    	                   sacaDisplay(c);
    	                   hora_alarma += (c-'0') ;
    	                   c=leer_tec();
    	                   if (c !='#' && c !='*'){
    	                       hora_alarma *=10;  
    	                       hora_alarma += (c-'0');
    	                       sacaDisplay(c);
    	                   }
    	            }
    	            sacaDisplay(':' ) ;
    	            c=leer_tec();
    	            if (c !='#' && c !='*'){
    	                   minutos_alarma=0;
    	                   sacaDisplay(c);
    	                   minutos_alarma += (c-'0') ;
    	                   c=leer_tec();
    	                   if (c !='#' && c !='*'){
    	                    minutos_alarma *=10; 
    	                    sacaDisplay(c);
    	                    minutos_alarma += (c-'0');
    	                   }
    	            }
    	        }else {
    	            imp_cadena("OFF", 3);
    	            hora_alarma=31;
    	        }
    	       habilitado=0;
	      break;
	        
	    }
	    
      
        _io_ports[ M6812_PORTG ] ^= M6812B_PG7;
    
    //serial_printdecbyte( c );
    }
}
    
void __attribute__( ( interrupt ) ) vi_ioc2 ( void )
{
    _io_ports[ M6812_TFLG1 ] = M6812B_IOS2; /*Bajamos el banderín de OC2 */

    /*preparamos siguiente disparo*/
    _IO_PORTS_W( M6812_TC2 ) = _IO_PORTS_W( M6812_TC2 ) + Periodo + Periodo;
     /* Se incremente en cada interrupción */
    /*segundos++;
    if(segundos>=60){
        segundos=0;
        minutos++;
        if (minutos >= 60){
            minutos =0;
            horas++;
            if (horas >=24)
                horas=0;
        }
    }*/
    minutos++;
        if (minutos >= 60){
            minutos =0;
            horas++;
            if (horas >=24)
                horas=0;
        }
    
    
   unsigned short num =horas;
    
    tamano=0;
   unsigned short i=0;
    for (i=0; i <4;i++)
        numero[i]=0;
    if (num >=10){
        numero[1] = num %10 +'0';
        num/=10;
        numero[0] = num %10 +'0';
         
    }else {
        numero[1] = num +'0';
        numero[0] = '0';
         
    }
    num =minutos;
    if (num >=10){
        numero[3] = num %10 +'0';
        num/=10;
        numero[2] = num %10 +'0';
         
    }else {
        numero[3] = num  +'0';
        numero[2] = '0';
         
    }
    /*num =segundos;
    if (num >=10){
        numero[tamano+1] = num %10 +'0';
        num/=10;
        numero[tamano] = num %10 +'0';
        tamano+=2; 
    }else {
       numero[tamano+1] = num  +'0';
        numero[tamano] = '0';
        t*mano+=2; 
    }*/
    
    if (horas == hora_alarma  && minutos==minutos_alarma  )
        contador_alarma=0;
   
        
        
    if (habilitado==0){
        enviaComando( CLEAR );
        if (contador_alarma<30){
            sacaDisplay( 2 );
            sacaDisplay( 'h');
            sacaDisplay( 'o' );
            sacaDisplay( 'l' );
            sacaDisplay( 'a');
            sacaDisplay( contador_alarma +'0');
           
        }else{
            enviaComando( CLEAR );
            //enviaComando( RETURN );
            imp_cadena_reloj(numero);
        }
        //sacaDisplay( LINE2 );
        //enviaComando( LINE2 );
        //volver a home 0000000010
        // cambiar a segunda linea 0011000000
    
    }
}
