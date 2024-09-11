#pragma once
#include <math.h>
#include <stdio.h>
#include <float.h>

#define M_PI 3.14159265358979323846
enum {
    LP = 0,
    HP,
    BP,
    NC,
    PK,
    LS,
    HS
};

int type_filters[] = { PK
                      ,HP
                      ,BP
                      ,NC
                      ,PK
                      ,LS
                      ,HS
                      ,HS
};
double Hz[] = {220 
              ,2000.0 
              ,3000.0 
              ,4000.0 
              , 1000.0
              , 2000.0
              , 3000.0
              , 4000.0

};

double dB[] = { 0.0
              , -3.0
              , -4.5
              , -6.0
              , -3
              , -3.0
              , -4.5
              , -6.0

};

double Qd[] = {  0.7071
              , 3.2
              , 1.56
              , 2.11
              , 1.0
              , 3.2
              , 1.56
              , 2.11 

};



static struct iir_filt {
    float in_z1;
    float in_z2;
    float out_z1;
    float out_z2;
    float a0;
    float a1;
    float a2;
    float b1;
    float b2;
};

struct iir_filt iir_coeff[5];


 static void calcBiquad(int type, double Fc, double peakGain, double Q, struct iir_filt* config) {
     // int type = type;

     double a0 = 1.0;
     double a1 = 0.0, a2 = 0.0, b1 = 0.0, b2 = 0.0;
    // float z1 = 0.0, z2 = 0.0;
     (*config).in_z1 = 0.0;
     (*config).in_z2 = 0.0;
     (*config).out_z1 = 0.0;
     (*config).out_z2 = 0.0;

     double norm;
     double V = pow(10, fabs(peakGain) / 20.0);
     double K = tan(M_PI * Fc);
     switch (type) {
     case LP:
         norm = 1 / (1 + K / Q + K * K);
         a0 = K * K * norm;
         a1 = 2 * a0;
         a2 = a0;
         b1 = 2 * (K * K - 1) * norm;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case HP:
         norm = 1 / (1 + K / Q + K * K);
         a0 = 1 * norm;
         a1 = -2 * a0;
         a2 = a0;
         b1 = 2 * (K * K - 1) * norm;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case BP:
         norm = 1 / (1 + K / Q + K * K);
         a0 = K / Q * norm;
         a1 = 0;
         a2 = -a0;
         b1 = 2 * (K * K - 1) * norm;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case NC:
         norm = 1 / (1 + K / Q + K * K);
         a0 = (1 + K * K) * norm;
         a1 = 2 * (K * K - 1) * norm;
         a2 = a0;
         b1 = a1;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case PK:
         if (peakGain >= 0) {    // boost
             norm = 1 / (1 + 1 / Q * K + K * K);
             a0 = (1 + V / Q * K + K * K) * norm;
             a1 = 2 * (K * K - 1) * norm;
             a2 = (1 - V / Q * K + K * K) * norm;
             b1 = a1;
             b2 = (1 - 1 / Q * K + K * K) * norm;
         }
         else {    // cut
             norm = 1 / (1 + V / Q * K + K * K);
             a0 = (1 + 1 / Q * K + K * K) * norm;
             a1 = 2 * (K * K - 1) * norm;
             a2 = (1 - 1 / Q * K + K * K) * norm;
             b1 = a1;
             b2 = (1 - V / Q * K + K * K) * norm;
         }
         break;
     case LS:
         if (peakGain >= 0) {    // boost
             norm = 1 / (1 + sqrt(2) * K + K * K);
             a0 = (1 + sqrt(2 * V) * K + V * K * K) * norm;
             a1 = 2 * (V * K * K - 1) * norm;
             a2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
             b1 = 2 * (K * K - 1) * norm;
             b2 = (1 - sqrt(2) * K + K * K) * norm;
         }
         else {    // cut
             norm = 1 / (1 + sqrt(2 * V) * K + V * K * K);
             a0 = (1 + sqrt(2) * K + K * K) * norm;
             a1 = 2 * (K * K - 1) * norm;
             a2 = (1 - sqrt(2) * K + K * K) * norm;
             b1 = 2 * (V * K * K - 1) * norm;
             b2 = (1 - sqrt(2 * V) * K + V * K * K) * norm;
         }
         break;
     case HS:
         if (peakGain >= 0) {    // boost
             norm = 1 / (1 + sqrt(2) * K + K * K);
             a0 = (V + sqrt(2 * V) * K + K * K) * norm;
             a1 = 2 * (K * K - V) * norm;
             a2 = (V - sqrt(2 * V) * K + K * K) * norm;
             b1 = 2 * (K * K - 1) * norm;
             b2 = (1 - sqrt(2) * K + K * K) * norm;
         }
         else {    // cut
             norm = 1 / (V + sqrt(2 * V) * K + K * K);
             a0 = (1 + sqrt(2) * K + K * K) * norm;
             a1 = 2 * (K * K - 1) * norm;
             a2 = (1 - sqrt(2) * K + K * K) * norm;
             b1 = 2 * (K * K - V) * norm;
             b2 = (V - sqrt(2 * V) * K + K * K) * norm;
         }
         break;
     }
     (*config).a0 = a0;
     (*config).a1 = a1;
     (*config).a2 = a2;
     (*config).b1 = b1;
     (*config).b2 = b2;

     printf(" a0 %13.15f\n a1 %13.16lf0\n a2 %13.16lf\n b1 %13.16lf\n b2 %13.16lf\n",
         (*config).a0, (*config).a1, (*config).a2, (*config).b1, (*config).b2);
     printf("\n");
    

 }



 static void create_biquad() {
     int length = sizeof(Hz) / sizeof(Hz[0]);
     for (int i = 0; i < length; i++){
         printf("type iir filter %d  %fHz, dB %f, Q %f\n", type_filters[i], Hz[i], dB[i], Qd[i]);
         calcBiquad(type_filters[i], Hz[i] / 44100, dB[i], Qd[i], &iir_coeff[i]);
     }
     
 }
#pragma once


 static float process(float in, struct iir_filt* config) {
    
     float out = in * (*config).a0 + (*config).in_z1;
     (*config).in_z1 = in * (*config).a1 + (*config).in_z2 - (*config).b1 * out;
     (*config).in_z2 = in * (*config).a2 - (*config).b2 * out;
    // printf("process %f\n", out);
     return out;
 }

/*a0 = 0.00011247994327214796
a1 = 0.00022495988654429593
a2 = 0.00011247994327214796
b1 = -1.9697782746275025
b2 = 0.9702281944005912*/




// should match bit rate in pipeline?
int16_t DspBuf[4096];

float coeffs_lpf[5];
float w_lpf[5] = {0,0};
#define NNN 4096
float FloatDspBuf[NNN];
float FloatDspBufB[NNN];
const float *pFloatDspBuf = FloatDspBuf;


// ********** PROCESS the buffer with DSP IIR !!

static audio_element_err_t Dsp_process(audio_element_handle_t self, char *inbuf, int len)
{


audio_element_input(self, (char *)DspBuf, len);

// ************* DSP Process DspBuf here ********************************



// Calculate iir filter coefficients ( instead of preset )

// generate low pass filter

float freq = 4000;
float qFactor = 200;

esp_err_t rety = ESP_OK;

rety = dsps_biquad_gen_lpf_f32(coeffs_lpf, freq, qFactor); // low pass
// rety = dsps_biquad_gen_lpf_f32(coeffs_hpf, freq, qFactor); // high pass

if (rety != ESP_OK) { ESP_LOGE(TAG, "Operation error dsps_biquad_gen_lpf_f32 = %i", rety);
return rety; }


// convert 16bit audio smaples to float ****************
for ( int i = 0; i < len; i++ )
{
// do this properly with ESP-DSP maths ?
FloatDspBuf = ((float)DspBuf) / (float)32768;
}

// DSP IIR biquad process
esp_err_t rett = ESP_OK;
rett = dsps_biquad_f32(pFloatDspBuf,FloatDspBufB,len,coeffs_lpf,w_lpf);

if (rett != ESP_OK) { ESP_LOGE(TAG, "Operation error = %i", rett);
return rett; }

// convert float audio samples back into 16bit audio samples for pipeline
for ( int j = 0; j < len; j++ )
{
// do this properly with ESP-DSP maths ?
FloatDspBufB[j] = FloatDspBufB[j] * 32768 ;
if( FloatDspBufB[j] > 32767 ) FloatDspBufB[j] = 32767;
if( FloatDspBufB[j] < -32768 ) FloatDspBufB[j] = -32768;
DspBuf[j] = (int16_t)FloatDspBufB[j]; // cast back
}

/// ************* END DSP Process ********************************

// DspBuf samples back into pipeline

int ret = audio_element_output(self, (char *)DspBuf, len);
return (audio_element_err_t)ret;
}
