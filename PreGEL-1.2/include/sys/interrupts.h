/********************************
* Fichero de inclusi�n que define la tabla de vectores.
* Se definen todos los vectores a _start salvo en aquellos para los 
*  que exista una etiqueta vi_nombre, en cuyo caso se usar� esa etiqueta.

   Copyright (C) Alberto F. Hamilton Castro
   Dpto. de Ingenier�a de Sistemas y Autom�tica 
        y Arquitectura y Tecnolog�a de Comutadores
   Universidad de La Laguna

   $Id: interrupts.h,v 1.2 2008/03/28 13:44:56 alberto Exp $

  Este programa es software libre. Puede redistribuirlo y/o modificarlo bajo 
  los t�rminos de la Licencia P�blica General de GNU seg�n es publicada 
  por la Free Software Foundation, bien de la versi�n 2 de dicha Licencia 
  o bien (seg�n su elecci�n) de cualquier versi�n posterior. 

  Este programa se distribuye con la esperanza de que sea �til, pero 
  SIN NING�N TIPO DE GARANT�A, incluso sin la garant�a MERCANTIL impl�cita 
  o sin garantizar la CONVENIENCIA PARA UN PROP�SITO PARTICULAR. 
  V�ase la Licencia P�blica General de GNU para m�s detalles. 

******************************************/

/*
            
Todos los vectores de interrupci�n quedan apuntando 
  a la etiqueta 'vi_default', salvo el vector de reset de 
  encendido que queda apuntando a la etiqueta '_start'.
  A su vez, si no se define, 'vi_default'  apuntar� a '_start'.


  Para modificar alg�n vector de interrupci�n es necesario
    definir la etiqueta correspondiente, asign�ndole un valor 
    con un equ o utiliz�ndola como nombre de 
    la rutina de tratamiento de la interrupci�n. 
    El nombre de las etiquetas es el que aparece en la siguiente lista:

  * vi_osc   Interrupci�n del sistema oscilador
  * vi_cantx   Transmisi�n en el subsistema CAN
  * vi_canrx   Recepci�n en el subsistema CAN
  * vi_canerr   Errores en el subsistema CAN
  * vi_pabov   Desbordamiento del acumulador de pulsos B
  * vi_cmuv   Cuenta final del contador de m�dulo
  * vi_kwgh   Activaci�n de los puestos G � H
  * vi_canwu   Activaci�n del subsistema CAN
  * vi_atd   Conversores anal�gicos
  * vi_sci1   Interrupci�n del sistema SCI 1
  * vi_sci0   Interrupci�n del sistema SCI 0
  * vi_spi   Interrupci�n del sistema SPI
  * vi_pai   Flanco del acumulador de pulsos
  * vi_paov   Desbordamiento del acumulador de pulsosA
  * vi_tov   Desbordamiento del temporizador
  * vi_ioc7   Canal 7 del temporizador
  * vi_ioc6   Canal 6 del temporizador
  * vi_ioc5   Canal 5 del temporizador
  * vi_ioc4   Canal 4 del temporizador
  * vi_ioc3   Canal 3 del temporizador
  * vi_ioc2   Canal 2 del temporizador
  * vi_ioc1   Canal 1 del temporizador
  * vi_ioc0   Canal 0 del temporizador
  * vi_rti   Interrupci�n de tiempo real
  * vi_irq   Interrupci�n l�nea IRQ
  * vi_xirq   Interrupci�n l�nea XIRQ 
  * vi_swi   Instrucci�n SWI
  * vi_trap   Instrucci�n Ilegal
  * vi_copreset   Reset por temporizador COP
  * vi_clkreset   Reset por fallo del reloj
  * vi_poreset   Reset externo o reset de encendido

 Estas etiquetas han de estar definidas cuando se carga este fichero,
  por ello, debe incluirse el final del c�digo fuente de cada programa.

*/
             

void __attribute__((interrupt)) vi_default(void){/* do nothing */}

extern void  __attribute__((interrupt))    vi_osc(void);
extern void  __attribute__((interrupt))    vi_cantx(void);
extern void  __attribute__((interrupt))    vi_canrx(void);
extern void  __attribute__((interrupt))    vi_canerr(void);
extern void  __attribute__((interrupt))    vi_pabov(void);
extern void  __attribute__((interrupt))    vi_cmuv(void);
extern void  __attribute__((interrupt))    vi_kwgh(void);
extern void  __attribute__((interrupt))    vi_canwu(void);
extern void  __attribute__((interrupt))    vi_atd(void);
extern void  __attribute__((interrupt))    vi_sci1(void);
extern void  __attribute__((interrupt))    vi_sci0(void);
extern void  __attribute__((interrupt))    vi_spi(void);
extern void  __attribute__((interrupt))    vi_pai(void);
extern void  __attribute__((interrupt))    vi_paov(void);
extern void  __attribute__((interrupt))    vi_tov(void);
extern void  __attribute__((interrupt))    vi_ioc7(void);
extern void  __attribute__((interrupt))    vi_ioc6(void);
extern void  __attribute__((interrupt))    vi_ioc5(void);
extern void  __attribute__((interrupt))    vi_ioc4(void);
extern void  __attribute__((interrupt))    vi_ioc3(void);
extern void  __attribute__((interrupt))    vi_ioc2(void);
extern void  __attribute__((interrupt))    vi_ioc1(void);
extern void  __attribute__((interrupt))    vi_ioc0(void);
extern void  __attribute__((interrupt))    vi_rti(void);
extern void  __attribute__((interrupt))    vi_irq(void);
extern void  __attribute__((interrupt))    vi_xirq(void);
extern void  __attribute__((interrupt))    vi_swi(void);
extern void  __attribute__((interrupt))    vi_trap(void);
extern void  __attribute__((interrupt))    vi_copreset(void);
extern void  __attribute__((interrupt))    vi_clkreset(void);


extern void _start(void);/* entry point in crt0.s */

void __attribute__ (( section (".vectors") )) (* const interrupt_vectors[])(void) = {
   vi_osc,  //Interrupci�n del sistema oscilador
   vi_cantx,  //Transmisi�n en el subsistema CAN
   vi_canrx,  //Recepci�n en el subsistema CAN
   vi_canerr,  //Errores en el subsistema CAN
   vi_pabov,  //Desbordamiento del acumulador de pulsos B
   vi_cmuv,  //Cuenta final del contador de m�dulo
   vi_kwgh,  //Activaci�n de los puestos G � H
   vi_canwu,  //Activaci�n del subsistema CAN
   vi_atd,  //Conversores anal�gicos
   vi_sci1, //  Interrupci�n del sistema SCI 1
   vi_sci0, //  Interrupci�n del sistema SCI 0
   vi_spi,  //Interrupci�n del sistema SPI
   vi_pai,  //Flanco del acumulador de pulsos
   vi_paov,  //Desbordamiento del acumulador de pulsosA
   vi_tov,  //Desbordamiento del temporizador
   vi_ioc7, //  Canal 7 del temporizador
   vi_ioc6, //  Canal 6 del temporizador
   vi_ioc5, //  Canal 5 del temporizador
   vi_ioc4, //  Canal 4 del temporizador
   vi_ioc3, //  Canal 3 del temporizador
   vi_ioc2, //  Canal 2 del temporizador
   vi_ioc1, //  Canal 1 del temporizador
   vi_ioc0, //  Canal 0 del temporizador
   vi_rti,  //Interrupci�n de tiempo real
   vi_irq,  //Interrupci�n l�nea IRQ
   vi_xirq,  //Interrupci�n l�nea XIRQ 
   vi_swi,  //Instrucci�n SWI
   vi_trap,  //Instrucci�n Ilegal
   vi_copreset,  //Reset por temporizador COP
   vi_clkreset,  //Reset por fallo del reloj
   _start,  //Reset externo o reset de encendido
};

/* fin tabla vectores*/
