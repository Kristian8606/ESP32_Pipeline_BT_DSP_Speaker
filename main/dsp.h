//#include <app_priv.h>
#include <inttypes.h>

extern uint8_t fs;



struct iir_filt {
    float in_z1 = 0.0;
    float in_z2 = 0.0;
   float out_z1 = 0.0;
   float out_z2 = 0.0;
   
    float in_z1c = 0.0;
    float in_z2c = 0.0;
   float out_z1c = 0.0;
   float out_z2c = 0.0;
   
   float a0[4];
   float a1[4];
   float a2[4];
   float b1[4];
   float b2[4];  
};


//bass-speaker,99.5 Hz  peak -10.0 dB Q1.927 
static struct iir_filt conf_99_5_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = { 0.9917041170065917, 0.9958434200204267 ,  0.9921381058417389 , 0.9972270234220063 },
	.a1 = { -1.9834082340131833, -1.9916868400408534, -1.9768056932289475  ,  -1.9944540468440126 },
	.a2 = { 0.9917041170065917, 0.9958434200204267,   0.9848662421925081 , 0.9972270234220063 },
	.b1 = { -1.9833394124635135, -1.9916695631391037, -1.9768056932289475  , -1.9944463575772033 },
	.b2 = { 0.9834770555628534, 0.991704116942603,    0.977004348034247 , 0.9944617361108218 },
	
}; 	

//bass-speaker, 50Hz peak 10db Q2
static struct iir_filt conf_50_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = { 0.9917041170065917, 0.9958434200204267 ,   1.003844026017551 , 0.9972270234220063 },
	.a1 = { -1.9834082340131833, -1.9916868400408534,  -1.996393807871002  ,  -1.9944540468440126 },
	.a2 = { 0.9917041170065917, 0.9958434200204267,    0.9926004398451252 , 0.9972270234220063 },
	.b1 = { -1.9833394124635135, -1.9916695631391037,  -1.996393807871002  , -1.9944463575772033 },
	.b2 = { 0.9834770555628534, 0.991704116942603,     0.9964444658626761  , 0.9944617361108218 },
	
}; 



//189.0 Hz -7.7 dB Q2.920
static struct iir_filt conf_189hz_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = { 0.13993146179027674, 0.044278036805267616, 0.9934955392446729 , 0.021620113635254866 },
	.a1 = { 0.2798629235805535, 0.08855607361053523,   -1.9771552660575853 , 0.04324022727050973 },
	.a2 = { 0.13993146179027674, 0.044278036805267616, 0.984376774730461 , 0.021620113635254866 } ,
	.b1 = { -0.699698900564656, -1.3228374096880198,   -1.9771552660575853 , -1.5430779694435248 },
	.b2 = { 0.259424747725763, 0.4999495569090904,     0.9778723139751341 , 0.6295584239845442}
	
}; 
	

//bass-speaker, 150Hz Lowpass///////////////////////
static struct iir_filt conf_150Hz_lp_Q_05 = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = { 0.0008325330343748603, 0.00021242182071631172, 0.00011196899738775855, 0.00009505983285468104 },
	.a1 = { 0.0016650660687497205, 0.00042484364143262343,  0.0002239379947755171, 0.00019011966570936208 },
	.a2 = { 0.0008325330343748603, 0.00021242182071631172,  0.00011196899738775855, 0.00009505983285468104 } ,
	.b1 = { -1.9167296538189462, -1.9583477764103403, 	    -1.9608304562582712, -1.9722296503999952 },
	.b2 = { 0.9200597859564457, 0.9591974636932055,         0.9612783322478222, 	0.9726098897314139}
	
}; 	
//bass-speaker, 150Hz Lowpass///////////////////////
static struct iir_filt conf_150Hz_lp_Q_1_3 = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = { 0.0008325330343748603, 0.00021242182071631172,0.0001132534402289635, 0.00009505983285468104 },
	.a1 = { 0.0016650660687497205, 0.00042484364143262343, 0.000226506880457927, 0.00019011966570936208 },
	.a2 = { 0.0008325330343748603, 0.00021242182071631172, 0.0001132534402289635, 0.00009505983285468104 } ,
	.b1 = { -1.9167296538189462, -1.9583477764103403, 	   -1.983323956254842, -1.9722296503999952 },
	.b2 = { 0.9200597859564457, 0.9591974636932055,        0.9837769700157579, 	0.9726098897314139}
	
}; 

//bass-speaker, eq 45Hz, q=0.707,  :-)
static struct iir_filt conf_45_hp = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = { 1.0045571784556593, 1.0022854380342285,  0.9954766638878052, 1.0015251377673262 },
	.a1 = { -1.9877372051598552, -1.9939885650528055, -1.9909533277756104, -1.9960191810845285 },
	.a2 = { 0.9837319156746053, 0.991841509165585,    0.9954766638878052, 0.9945556071485654 },
	.b1 = { -1.9877372051598552, -1.9939885650528055, -1.9909328674920315, -1.9960191810845285 },
	.b2 = { 0.9882890941302647, 0.9941269471998133,   0.9909737880591895, 0.9960807449158914 }
	
}; 	

//tweeter 200Hz Hipass Q0.5
static struct iir_filt conf_200_hp_Q0_5 = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {0.9329230232468055,0.7056967416492775, 0.9741547885778307 ,0.9771222901052568,},
	.a1 = {-1.865846046493611,-1.411393483298555,  -1.9483095771556613 ,-1.9542445802105135,},
	.a2 = {0.9329230232468055,0.7056967416492775,  0.9741547885778307 ,0.9771222901052568,},
	.b1 = {-1.8613429252631009,-1.32283740968802,  -1.9479140290594112 ,-1.9537212766076175,},
	.b2 = {0.8703491677241215,0.4999495569090903,  0.9487051252519113 ,0.9547678838134095,}
	 
};


static struct iir_filt conf_200_hp_Q1_3 = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {0.9329230232468055,0.7056967416492775,0.989013651809719,0.9771222901052568,},
	.a1 = {-1.865846046493611,-1.411393483298555, -1.978027303619438,-1.9542445802105135,},
	.a2 = {0.9329230232468055,0.7056967416492775, 0.989013651809719,0.9771222901052568,},
	.b1 = {-1.8613429252631009,-1.32283740968802, -1.9776257221954943,-1.9537212766076175,},
	.b2 = {0.8703491677241215,0.4999495569090903, 0.9784288850433815,0.9547678838134095,}
	 
};

// 2,775 Hz -5.0 dB Q1.000 
static struct iir_filt conf_2775hz_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {  0.6666331051985078, 0.7870056430245259,  0.8883556512003908 , 0.8452797115980594},
	.a1 = { -0.9427615784986199, -1.4541968111224874, -1.3748778153853343   , -1.6329550077414818},
	.a2 = { 0.6666331051985078, 0.7870056430245259,   0.6014552209448205 , 0.8452797115980594},
	.b1 = { -0.9427615784986199, -1.4541968111224874, -1.3748778153853343   , -1.6329550077414818},
	.b2 = { 0.3332662103970156, 0.574011286049052,    0.48981087214521124 , 0.690559423196119}
	
}; 	
//5,274 Hz -6.7 dB Q1.000
static struct iir_filt conf_5274_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {  0.6666331051985078, 0.7870056430245259,  0.7716743234068036 , 0.8452797115980594},
	.a1 = { -0.9427615784986199, -1.4541968111224874, -0.8407714963625843   , -1.6329550077414818},
	.a2 = { 0.6666331051985078, 0.7870056430245259,   0.3789297821719864 , 0.8452797115980594},
	.b1 = { -0.9427615784986199, -1.4541968111224874, -0.8407714963625843   , -1.6329550077414818},
	.b2 = { 0.3332662103970156, 0.574011286049052,    0.1506041055787901 , 0.690559423196119}
	
}; 	
// 122.5 Hz +3.0 dB Q1.013 
static struct iir_filt conf_122_5hz_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {  0.6666331051985078, 0.7870056430245259,  1.0035233376988448 , 0.8452797115980594},
	.a1 = { -0.9427615784986199, -1.4541968111224874, -1.9826166971205357   , -1.6329550077414818},
	.a2 = { 0.6666331051985078, 0.7870056430245259,   0.979395367544982 , 0.8452797115980594},
	.b1 = { -0.9427615784986199, -1.4541968111224874, -1.9826166971205357   , -1.6329550077414818},
	.b2 = { 0.3332662103970156, 0.574011286049052,    0.9829187052438266 , 0.690559423196119}
	
}; 	

// 231 Hz -5.1 dB Q4.890
static struct iir_filt conf_231hz_peak = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {  0.6666331051985078, 0.7870056430245259,  0.9973282737652758 , 0.8452797115980594},
	.a1 = { -0.9427615784986199, -1.4541968111224874, -1.9868912108456136 , -1.6329550077414818},
	.a2 = { 0.6666331051985078, 0.7870056430245259,   0.9906395179622348  , 0.8452797115980594},
	.b1 = { -0.9427615784986199, -1.4541968111224874, -1.9868912108456136 , -1.6329550077414818},
	.b2 = { 0.3332662103970156, 0.574011286049052,    0.9879677917275108 , 0.690559423196119}
	 
}; 	 
 static struct iir_filt conf_200hz_test = {
	//index0 = 16k, index1 = 32k, index 2 = 44k1, index 3 = 48k
	.a0 = {  0.6666331051985078, 0.7870056430245259,  0.9812039070614107, 0.8452797115980594},
	.a1 = { -0.9427615784986199, -1.4541968111224874, -1.9534836262586168 , -1.6329550077414818},
	.a2 = { 0.6666331051985078, 0.7870056430245259,   0.9730730773412465 , 0.8452797115980594},
	.b1 = { -0.9427615784986199, -1.4541968111224874, -1.9534836262586168 , -1.6329550077414818},
	.b2 = { 0.3332662103970156, 0.574011286049052,    0.9542769844026571, 0.690559423196119}
	 
}; 	 

static float process_iir_ch_1 (float inSampleF, struct iir_filt * config) {
	
	
	float outSampleF =
	(* config).a0[fs] * inSampleF
	+ (* config).a1[fs] * (* config).in_z1
	+ (* config).a2[fs] * (* config).in_z2
	- (* config).b1[fs] * (* config).out_z1
	- (* config).b2[fs] * (* config).out_z2;
	(* config).in_z2 = (* config).in_z1;
	(* config).in_z1 = inSampleF;
	(* config).out_z2 = (* config).out_z1;
	(* config).out_z1 = outSampleF;
	return outSampleF;
}

static float process_iir_ch_2 (float inSampleF, struct iir_filt * config) {
	
	float outSampleF =
	(* config).a0[fs] * inSampleF
	+ (* config).a1[fs] * (* config).in_z1c
	+ (* config).a2[fs] * (* config).in_z2c
	- (* config).b1[fs] * (* config).out_z1c
	- (* config).b2[fs] * (* config).out_z2c;
	(* config).in_z2c = (* config).in_z1c;
	(* config).in_z1c = inSampleF;
	(* config).out_z2c = (* config).out_z1c;
	(* config).out_z1c = outSampleF;
	return outSampleF;
}
static float process_iir_mono(float inSampleF, struct iir_filt * config) {
	
	float outSampleF =
	(* config).a0[fs] * inSampleF
	+ (* config).a1[fs] * (* config).in_z1
	+ (* config).a2[fs] * (* config).in_z2
	- (* config).b1[fs] * (* config).out_z1
	- (* config).b2[fs] * (* config).out_z2;
	(* config).in_z2 = (* config).in_z1;
	(* config).in_z1 = inSampleF;
	(* config).out_z2 = (* config).out_z1;
	(* config).out_z1 = outSampleF;
	return outSampleF;
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

		float lowsample = process_iir_mono(insample, &conf_45_hp);
	    lowsample = process_iir_mono(lowsample, &conf_50_peak);
	    lowsample = process_iir_mono(lowsample, &conf_99_5_peak);
	    lowsample = process_iir_mono(lowsample, &conf_122_5hz_peak);
		lowsample = process_iir_mono(lowsample, &conf_150Hz_lp_Q_05);
		lowsample = process_iir_mono(lowsample, &conf_150Hz_lp_Q_1_3);
		//process bass speaker EQ

		//process tweeter lowpass
		float highsample = process_iir_mono(insample, &conf_200_hp_Q0_5);
		highsample = process_iir_mono(highsample, &conf_200_hp_Q1_3);
		//process tweeter EQ
		highsample = process_iir_mono(highsample, &conf_189hz_peak);
		highsample = process_iir_mono(highsample, &conf_231hz_peak);
		highsample = process_iir_mono(highsample, &conf_2775hz_peak);
		highsample = process_iir_mono(highsample, &conf_5274_peak);
		
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
		
		
		float sample_ch_1 = process_iir_ch_1(insample, &conf_45_hp);
	//    sample_ch_1 = process_iir_ch_1(sample_ch_1, &conf_50_peak);
	//	sample_ch_1 = process_iir_ch_1(sample_ch_1, &conf_200hz_test);

		
		float sample_ch_2 = process_iir_ch_2(insample2, &conf_45_hp);
	//	sample_ch_2 = process_iir_ch_2(sample_ch_2, &conf_50_peak);
	//	sample_ch_2 = process_iir_ch_2(sample_ch_2, &conf_200hz_test);
	//	sample_ch_2 = process_iir_ch_2(sample_ch_2, &conf_2775hz_peak);
	//	sample_ch_2 = process_iir_ch_2(sample_ch_2, &conf_189hz_peak);

		*outsample = (int16_t) sample_ch_1 ;//*0.6;
		outsample++;
		*outsample = (int16_t) sample_ch_2;
		outsample++;	
	
	}

}


