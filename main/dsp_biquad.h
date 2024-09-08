#include <math.h>
#include <stdio.h>

//#define M_PI 3.14159265358979323846
/*
Equaliser: miniDSP 2x4 HD
RMS Average 1
Filter  1: ON  PK       Fc   92.60 Hz  Gain  -2.50 dB  Q  4.882
Filter  2: ON  PK       Fc   105.0 Hz  Gain  -3.30 dB  Q  4.968
Filter  3: ON  PK       Fc   125.0 Hz  Gain  -1.80 dB  Q  4.984
Filter  4: ON  PK       Fc   182.5 Hz  Gain  -4.80 dB  Q  4.968
Filter  5: ON  PK       Fc   230.0 Hz  Gain  -5.80 dB  Q  4.876
Filter  6: ON  PK       Fc   328.0 Hz  Gain   2.40 dB  Q  1.000
Filter  7: ON  PK       Fc    1083 Hz  Gain  -3.50 dB  Q  2.359
Filter  8: ON  PK       Fc    2698 Hz  Gain  -7.70 dB  Q  1.000
Filter  9: ON  PK       Fc    6550 Hz  Gain  -4.00 dB  Q  1.000
Filter 10: ON  PK       Fc   11674 Hz  Gain   2.80 dB  Q  5.799


PK       Fc   183.5 Hz  Gain  -3.60 dB  Q  4.824
HS       Fc    8882 Hz  Gain   8.40 dB
PK       Fc   11213 Hz  Gain  -3.60 dB  Q  1.247

*/
enum {
    LP = 0,
    HP,
    BP,
    Notch,
    PK,
    LS,
    HS
};


int type_filters[] = { LP
                      ,LP
                      ,HP
                      ,HP
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
                      ,PK
};
double Hz[] = {150.0 
              ,150.0 
              ,200.0 
              ,200.0 
              , 92.60
              , 105.0
              , 125.0
              , 182.5
			  ,	230.0
			  , 328.0
			  , 1083
			  , 2698
			  , 6550
			  , 11674
			  ,50.0
};

double dB[] = { 0.0
              , 0.0
              , 0.0
              , 0.0
              , -2.50
              , -3.30
              , -1.80
              , -4.80
              , -5.80
              ,  2.40
              , -3.50
              , -7.70
              , -4.00
              ,  2.80
              , 10.0
};

double Qd[] = { 0.54119610
              , 1.3065630
              , 0.54119610
              , 1.3065630
              , 4.882
              , 4.968
              , 4.984
              , 4.968
              , 4.876
              , 1.000
              , 2.359
              , 1.000
              , 1.000
              , 5.799
              , 5.0
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
int length = (sizeof(Hz) / sizeof(Hz[0]));

struct iir_filt iir_coeff[15];

 static void calcBiquad(int type, double Fc, double peakGain,  double Q, struct iir_filt* config) {
     // int type = type;

     double a0 = 1.0;
     double a1 = 0.0, a2 = 0.0, b1 = 0.0, b2 = 0.0;
   //  double z1 = 0.0, z2 = 0.0;
//		 (*config).in_z1 = 0.0;
//		 (*config).in_z2 = 0.0;
//		(*config).out_z1 = 0.0;
//		(*config).out_z2 = 0.0;

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

     case Notch:
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


     printf(" a0 %13.16f\n a1 %13.16f0\n a2 %13.16f\n b1 %13.16f\n b2 %13.16f\n",
          (*config).a0,  (*config).a1,  (*config).a2,  (*config).b1,  (*config).b2);
     printf("\n");


 }

 static float process_iir_mono(float inSampleF, struct iir_filt* config) {
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
 static float process_iir_ch_1 (float inSampleF, struct iir_filt * config) {
	
	
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

static float process_iir_ch_2 (float inSampleF, struct iir_filt * config) {
	
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
      
     for (int i = 0; i < length; i++){
         printf("type iir filter %d  %fHz, dB %f, Q %f\n", type_filters[i], Hz[i], dB[i], Qd[i]);
         calcBiquad(type_filters[i], Hz[i] / 44100, dB[i], Qd[i], &iir_coeff[i]);
     }
   
 }
 
 static void process_data_mono(char * data, size_t item_size) {
	
	int16_t * samples = (int16_t *) data;
	int16_t * outsample = (int16_t *) data;
	
	for (int i=0; i<item_size; i=i+4) {
		//restore input samples and make monosum
		float insample = (float) *samples;
		samples++;
		insample += *samples;
		insample = insample / 2;
		samples++;
		
		//monosum now available in insample
		
		//process bass speaker
		 
		float lowsample = process_iir_mono(insample, &iir_coeff[0]);
	    lowsample = process_iir_mono(lowsample, &iir_coeff[1]);
	    lowsample = process_iir_mono(lowsample, &iir_coeff[14]);
	    lowsample = process_iir_mono(lowsample, &iir_coeff[4]);
	    lowsample = process_iir_mono(lowsample, &iir_coeff[5]);
	    lowsample = process_iir_mono(lowsample, &iir_coeff[6]);
	    lowsample = process_iir_mono(lowsample, &iir_coeff[7]);
	   
		//lowsample = process_iir_mono(insample, &_45);
	  

		//process tweeter lowpass
		float highsample = process_iir_mono(insample, &iir_coeff[2]);
		highsample = process_iir_mono(highsample, &iir_coeff[3]);
		highsample = process_iir_mono(highsample, &iir_coeff[8]);
		highsample = process_iir_mono(highsample, &iir_coeff[9]);
		highsample = process_iir_mono(highsample, &iir_coeff[10]);
		highsample = process_iir_mono(highsample, &iir_coeff[11]);
		highsample = process_iir_mono(highsample, &iir_coeff[12]);
		highsample = process_iir_mono(highsample, &iir_coeff[13]);
		
		
		//restore two outputsamples lowsample & highsample to outputbuffer
		*outsample = (int16_t) lowsample ;//*0.6;
		outsample++;
		*outsample = (int16_t) highsample;
		outsample++;		
		
	
	}

}

static void process_data_stereo(char * data, size_t item_size) {
	
	int16_t * samples = (int16_t *) data;
	int16_t * outsample = (int16_t *) data;
	
	for (int i=0; i<item_size; i=i+4) {
	
		//restore input samples and make monosum
		float insample = (float) *samples;
		samples++;
		float insample2 = (float) *samples;
		samples++;
		
		
		float sample_ch_1 = process_iir_ch_1(insample, &iir_coeff[0]);
	

		
		float sample_ch_2 = process_iir_ch_2(insample2, &iir_coeff[0]);
	

		*outsample = (int16_t) sample_ch_1 ;//*0.6;
		outsample++;
		*outsample = (int16_t) sample_ch_2;
		outsample++;	
	
	}

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