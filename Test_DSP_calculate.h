#include <math.h>
#include <stdio.h>

#define M_PI 3.14159265358979323846
enum {
    bq_type_lowpass = 0,
    bq_type_highpass,
    bq_type_bandpass,
    bq_type_notch,
    bq_type_peak,
    bq_type_lowshelf,
    bq_type_highshelf
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


double Q = 0.707;
double peakGain = -3.0;

static struct iir_filt iir150Hz;
static struct iir_filt iir5050Hz;

 static void calcBiquad(int type, double Fc, double Q, double peakGain, struct iir_filt* config) {
     // int type = type;

     double a0 = 1.0;
     double a1 = 0.0, a2 = 0.0, b1 = 0.0, b2 = 0.0;
     double z1 = 0.0, z2 = 0.0;

     double norm;
     double V = pow(10, fabs(peakGain) / 20.0);
     double K = tan(M_PI * Fc);
     switch (type) {
     case bq_type_lowpass:
         norm = 1 / (1 + K / Q + K * K);
         a0 = K * K * norm;
         a1 = 2 * a0;
         a2 = a0;
         b1 = 2 * (K * K - 1) * norm;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case bq_type_highpass:
         norm = 1 / (1 + K / Q + K * K);
         a0 = 1 * norm;
         a1 = -2 * a0;
         a2 = a0;
         b1 = 2 * (K * K - 1) * norm;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case bq_type_bandpass:
         norm = 1 / (1 + K / Q + K * K);
         a0 = K / Q * norm;
         a1 = 0;
         a2 = -a0;
         b1 = 2 * (K * K - 1) * norm;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case bq_type_notch:
         norm = 1 / (1 + K / Q + K * K);
         a0 = (1 + K * K) * norm;
         a1 = 2 * (K * K - 1) * norm;
         a2 = a0;
         b1 = a1;
         b2 = (1 - K / Q + K * K) * norm;
         break;

     case bq_type_peak:
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
     case bq_type_lowshelf:
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
     case bq_type_highshelf:
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


     printf(" a0 %13.16f\n a1 %13.16f0\n a2 %13.16f\n b1 %13.16f\n b2 %13.16f\n",
         iir150Hz.a0, iir150Hz.a1, iir150Hz.a2, iir150Hz.b1, iir150Hz.b2);
     printf("\n");


 }

 static float process_iir(float inSampleF, struct iir_filt* config) {
     float outSampleF =
         (*config).a0 * inSampleF
         + (*config).a1 * (*config).in_z1
         + (*config).a2 * (*config).in_z2
         - (*config).b1 * (*config).out_z1
         - (*config).b2 * (*config).out_z2;
     (*config).in_z2 = (*config).in_z1;
     (*config).in_z1 = inSampleF;
     (*config).out_z2 = (*config).out_z1;
     (*config).out_z1 = outSampleF;
     return outSampleF;
 }

 static void create_biquad() {
     calcBiquad(bq_type_peak, 1000.0/44100, 0.707, -3.0, &iir150Hz);
     calcBiquad(bq_type_peak, 2000.0/44100, 1.0, 12.0, &iir5050Hz);

 }
/*
static float process(float in) {
    double out = in * a0 + z1;
    z1 = in * a1 + z2 - b1 * out;
    z2 = in * a2 - b2 * out;
    printf("process %f\n",out);
    return out;
}

*/
