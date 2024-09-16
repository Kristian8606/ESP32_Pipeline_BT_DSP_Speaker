#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.141592654)
#endif
#define SAMPLE_RATE (44100)
// Bi-Quad Module

static struct biquad{
	float a0;
	float a1;
	float a2;
	float b0;
	float b1;
	float b2;
	float in_z1;
    float in_z2;
    float out_z1;
    float out_z2;
	char* type;
	float coeffs[5];
//    float w_lpf[5] = {0,0};
}biquad;

 enum {
	LOWPASS 	= 0,
	HIGHPASS 	= 1,
	BANDPASS 	= 2,
	NOTCH		= 3,
	PEAK		= 4,
	LOWSHELF	= 5,
	HIGHSHELF	= 6,
	ALLPASS180  = 7,
};

struct biquad lpf_150;
struct biquad lpf_150;
struct biquad allpass;
struct biquad hpf_150;
struct biquad hpf_150;

static float process_iir_mono(float inSampleF, struct biquad* config) {
     float outSampleF =
	config->a0 * inSampleF
	+ config->a1 * config->in_z1
	+ config->a2 * config->in_z2
	- config->b1 * config->out_z1
	- config->b2 * config->out_z2;
	config->in_z2 = config->in_z1;
	config->in_z1 = inSampleF;
	config->out_z2 = config->out_z1;
	config->out_z1 = outSampleF;
	return outSampleF;
 }

void bq_destroy(struct biquad* bq){
	free(bq);
}

void bq_print_info(struct biquad* bq){
	printf("A0: %13.16f\n",bq->a0);
	printf("A1: %13.16f\n",bq->a1);
	printf("A2: %13.16f\n",bq->a2);
	//printf("B0:13.16.8f\n",bq->b0);
	printf("B1: %13.16f\n",bq->b1);
	printf("B2: %13.16f\n",bq->b2);
	printf("TYPE: %s\n",bq->type);

}

static void bq_load_coefficients(struct biquad* bq, int filter_type,float frequency, float Q,double dbGain,int sample_rate){
	
	switch (filter_type){
		case LOWPASS:
		 bq->in_z1 = 0.0;
	 bq->in_z2 = 0.0;
	bq->out_z1 = 0.0;
	bq->out_z2 = 0.0;
	
			dsps_biquad_gen_lpf_f32(bq->coeffs, frequency/sample_rate, Q);
			bq->a0 = bq->coeffs[0];
			bq->a1 = bq->coeffs[1];
			bq->a2 = bq->coeffs[2];
			bq->b1 = bq->coeffs[3];
			bq->b2 = bq->coeffs[4];
			
    		bq->type = "LOWPASS";
			break;

		case HIGHPASS:
			dsps_biquad_gen_hpf_f32(bq->coeffs, frequency/sample_rate, Q);
			bq->a0 = bq->coeffs[0];
			bq->a1 = bq->coeffs[1];
			bq->a2 = bq->coeffs[2];
			bq->b1 = bq->coeffs[3];
			bq->b2 = bq->coeffs[4];
			
    		bq->type = "HIGHPASS";
			break;

		case BANDPASS:
			
    		bq->type = "BANDPASS";
			break;
		
		case ALLPASS180:
			dsps_biquad_gen_allpass360_f32(bq->coeffs, frequency/sample_rate, Q);
			bq->a0 = bq->coeffs[0];
			bq->a1 = bq->coeffs[1];
			bq->a2 = bq->coeffs[2];
			bq->b1 = bq->coeffs[3];
			bq->b2 = bq->coeffs[4];
    		bq->type = "ALLPASS180";
			break;
		
		case NOTCH:
			
    		bq->type = "NOTCH";
			break;
		
		case PEAK:
		
    		bq->type = "PEAK";
			break;
		
		case LOWSHELF:
			
    		bq->type = "LOWSHELF";
			break;
		
		case HIGHSHELF:
			
    		bq->type = "HIGHSHELF";
			break;

		default:
			break;
	}
}

void create_biquad(){
	bq_load_coefficients(&lpf_150, LOWPASS, 300.0, 0.707, 0.0, 44100);
	bq_load_coefficients(&allpass, ALLPASS180, 100.0, 0.707, 0.0, 44100);
	
	bq_load_coefficients(&hpf_150, HIGHPASS, 300.0, 0.707, 0.0, 44100);
	//bq_load_coefficients(&hpf_150, HIGHPASS, 300, 0.707, 0.0, 44100);
	bq_print_info(&hpf_150);
	bq_print_info(&lpf_150);
	bq_print_info(&allpass);
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
		 
		float lowsample = process_iir_mono(insample, &lpf_150);
	    lowsample = process_iir_mono(lowsample, &lpf_150);
	   
	 //   lowsample = process_iir_mono(lowsample, &iir_coeff[5]);
	 

		//process tweeter lowpass
		float highsample = process_iir_mono(insample, &allpass);
		highsample = process_iir_mono(highsample, &hpf_150);
		highsample = process_iir_mono(highsample, &hpf_150);
/*		highsample = process_iir_mono(highsample, &iir_coeff[7]);
		highsample = process_iir_mono(highsample, &iir_coeff[8]);
		highsample = process_iir_mono(highsample, &iir_coeff[9]);
		highsample = process_iir_mono(highsample, &iir_coeff[10]);
*/	

		//restore two outputsamples lowsample & highsample to outputbuffer
		*outsample = (int16_t) lowsample; // *0.9;
		outsample++;
		*outsample = (int16_t) highsample * 0.8;
		outsample++;		
		
	
	}

}
