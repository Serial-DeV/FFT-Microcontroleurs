#include "mbed.h"
#include <math.h> 
#include <cstdio>

#include <cmath>
#include <complex>


#include <iostream>
#include <valarray>

    
void initLCD();
void write_command(char c);
void write_data(char c);
void print_LCD_char(char c);
void print_LCD_String(char *s);
void initLED();
void configLED(char b, char g, char r, char a);
void print_LCD_int(int en);

AnalogIn micro(A0);
I2C i2c(I2C_SDA , I2C_SCL );
Serial pc(USBTX, USBRX);


int LCD_addr = 0x7C;
int LED_addr = 0xC4;
int i = 0;
char*str;
char ch[36];


void initLED(){
    
    char cmd[3];
    wait(0.04);
    cmd[0] = 0x80;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    i2c.write(LED_addr, cmd, 3, false);
    cmd[0] = 0x08;
    cmd[1] = 0xAA;
    i2c.write(LED_addr, cmd, 2, false);
}

void configLED(char b, char g, char r, char a){
    
    char cmd[5];
    cmd[0] = 0xA2;
    cmd[1] = b;
    cmd[2] = g;
    cmd[3] = r;
    cmd[4] = a;
    i2c.write(LED_addr, cmd, 5, false);
    
}
    

void initLCD(){
    wait(0.04);
    write_command(0x3C);//4 pour mode 1 ligne(Celle du haut), C pour 2
    write_command(0x0C);
    write_command(0x01);
    wait(0.0016);
    write_command(0x06);
    
    
}

void write_command(char c){
    char cmd[2];
    cmd[0] = 0x80;
    cmd[1] = c;
    i2c.write(LCD_addr, cmd, 2, false);//false car répété
}

void write_data(char c){
    char cmd[2];
    cmd[0] = 0x40;
    cmd[1] = c;
    i2c.write(LCD_addr, cmd, 2, false);
}

void print_LCD_char(char c){
    char cmd[2];
    cmd[0] = 0x40;
    cmd[1] = c;
    i2c.write(LCD_addr, cmd, 2, false);
}

void print_LCD_String(char *s){
    char cmd[36];
    cmd[0] = 0x40;
    for(int n = 1; n < strlen(s)+1; n++){
        cmd[n] = s[n-1];
    }
    
    
    i2c.write(LCD_addr, cmd, strlen(s)+1, false);
}

void affichage(char n, char* b){
        wait(0.4);
        write_command(0x01);
        wait(0.00153);
        sprintf(b, "%d", n);
        print_LCD_String(b);
}
 
const double PI = 3.141592653589793238460;
 
typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;
 
// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void fft(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;
 
    // divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray  odd = x[std::slice(1, N/2, 2)];
 
    // conquer
    fft(even);
    fft(odd);
 
    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        x[k    ] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
        
}

 
int main() {
    
    initLED();
    configLED(0xFF, 0xFF, 0xFF, 0xFF);
    initLCD();
    
    const int nbEch = 128; //nombre d'échantillons
    int frqEch = 44000;
    Complex echantillons[nbEch];
    int continuer = 1;
    
  
    while(continuer){       
        //wait (0.1);
        continuer = 1;
        Complex test[nbEch];
        
        for (int nb = 0; nb <nbEch ; nb++){
                wait (1.0/(float)frqEch);
                test[nb] = micro.read(); 
        }
        
        /*for (int nb = 0; nb <nbEch ; nb++){
            //test[nb] = 100.0*cos(2.0*PI*nb*20000.0*rand()/(float)frqEch)+100.0*cos(2.0*PI*nb*20000.0*rand()/(float)frqEch);
            if (nb < nbEch/2)
                test[nb] = 100.0;
            else test[nb] = -100.0;
        }*/
        
        CArray data(test, nbEch);
        float VectFreq[nbEch/2];
 
        // forward fft
        fft(data);
        float max = 0.0;
        float maxFreq = 0.0;
        for (int nb = 0; nb <nbEch/2 ; nb++){
            data[nb] = (norm(data[nb]));
            VectFreq[nb] =((float)nb) * (float)frqEch/((float)(nbEch)) ;
            //pc.printf("\r\n%.f Hz --> %f",VectFreq[nb],data[nb].real());
            if (data[nb].real() > max){
                max = data[nb].real();
                maxFreq = VectFreq[nb];
            }
        }
        
                
        pc.printf("\rFrequence dominante = %f Hz avec A = %f  ", maxFreq, max);
        
    }
}
