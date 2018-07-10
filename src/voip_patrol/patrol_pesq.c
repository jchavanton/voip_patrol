#include <stdio.h>
#include <math.h>
#include "PESQ/pesq.h"
#include "PESQ/dsp.h"

#define ITU_RESULTS_FILE          "pesq_results.txt"

void print_licence () {
	printf (" \n\
		Perceptual Evaluation of Speech Quality (PESQ)\n\
		Reference implementation for ITU-T Recommendations P.862, P.862.1 and P.862.2.\n\
		Version 2.0 October 2005.\n\
		\n\
		PESQ Intellectual Property Rights Notice.\n\
		\n\
		DEFINITIONS:\n\
		For the purposes of this Intellectual Property Rights Notice the terms\n\
		'Perceptual Evaluation of Speech Quality Algorithm' and 'PESQ Algorithm'\n\
		refer to the objective speech quality measurement algorithm defined in ITU-T\n\
		Recommendation P.862; the term 'PESQ Software' refers to the C-code component\n\
		of P.862. These definitions also apply to those parts of ITU-T Recommendation\n\
		P.862.2 and its source code that are common with P.862.\n\
		\n\
		NOTICE:\n\
		All copyright, trade marks, trade names, patents, know-how and all or any other\n\
		intellectual rights subsisting in or used in connection with including all\n\
		algorithms, documents and manuals relating to the PESQ Algorithm and or PESQ\n\
		Software are and remain the sole property in law, ownership, regulations,\n\
		treaties and patent rights of the Owners identified below. The user may not\n\
		dispute or question the ownership of the PESQ Algorithm and or PESQ Software.\n\
		\n\
		OWNERS ARE:\n\
		1.	British Telecommunications plc (BT), all rights assigned\n\
		      to Psytechnics Limited\n\
		2.	Royal KPN NV, all rights assigned to OPTICOM GmbH\n\
		\n\
		RESTRICTIONS:\n\
		The user cannot:\n\
		1.	alter, duplicate, modify, adapt, or translate in whole or in\n\
		      part any aspect of the PESQ Algorithm and or PESQ Software\n\
		2.	sell, hire, loan, distribute, dispose or put to any commercial\n\
		      use other than those permitted below in whole or in part any\n\
		      aspect of the PESQ Algorithm and or PESQ Software\n\
		\n\
		PERMITTED USE:\n\
		The user may:\n\
		1.	Use the PESQ Software to:\n\
		      i)   understand the PESQ Algorithm; or\n\
		      ii)  evaluate the ability of the PESQ Algorithm to perform its intended\n\
		           function of predicting the speech quality of a system; or\n\
		      iii) evaluate the computational complexity of the PESQ Algorithm,\n\
		           with the limitation that none of said evaluations or its\n\
		           results shall be used for external commercial use.\n\
		2.	Use the PESQ Software to test if an implementation of the PESQ\n\
		      Algorithm conforms to ITU-T Recommendation P.862.\n\
		3.	With the prior written permission of both Psytechnics Limited and\n\
		      OPTICOM GmbH, use the PESQ Software in accordance with the above\n\
		      Restrictions to perform work that meets all of the following criteria:\n\
		      i)    the work must contribute directly to the maintenance of an\n\
		            existing ITU recommendation or the development of a new ITU\n\
		            recommendation under an approved ITU Study Item; and\n\
		      ii)   the work and its results must be fully described in a\n\
		            written contribution to the ITU that is presented at a formal\n\
		            ITU meeting within one year of the start of the work; and\n\
		      iii)  neither the work nor its results shall be put to any\n\
		            commercial use other than making said contribution to the ITU.\n\
		            Said permission will be provided on a case-by-case basis.\n\
		\n\
		ANY OTHER USE OR APPLICATION OF THE PESQ SOFTWARE AND/OR THE PESQ ALGORITHM\n\
		WILL REQUIRE A PESQ LICENCE AGREEMENT, WHICH MAY BE OBTAINED FROM EITHER\n\
		OPTICOM GMBH OR PSYTECHNICS LIMITED.\n\
		\n\
		EACH COMPANY OFFERS OEM LICENSE AGREEMENTS, WHICH COMBINE OEM\n\
		IMPLEMENTATIONS OF THE PESQ ALGORITHM TOGETHER WITH A PESQ PATENT LICENSE\n\
		AGREEMENT. PESQ PATENT-ONLY LICENSE AGREEMENTS MAY BE OBTAINED FROM OPTICOM.\n\
		\n\
		***********************************************************************\n\
		*  OPTICOM GmbH                    *  Psytechnics Limited             *\n\
		*  Naegelsbachstr. 38,             *  Fraser House, 23 Museum Street, *\n\
		*  D- 91052 Erlangen, Germany      *  Ipswich IP1 1HN, England        *\n\
		*  Phone: +49 (0) 9131 53020 0     *  Phone: +44 (0) 1473 261 800     *\n\
		*  Fax:   +49 (0) 9131 53020 20    *  Fax:   +44 (0) 1473 261 880     *\n\
		*  E-mail: info@opticom.de,        *  E-mail: info@psytechnics.com,   *\n\
		*  www.opticom.de                  *  www.psytechnics.com             *\n\
		***********************************************************************\n\
	\n");
}


void pesq_measure (SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info, long * Error_Flag, char ** Error_Type);

static int licence_printed;

float pesq_process (long sample_rate, const char *reference, const char *degraded) {
	if (!licence_printed) {
		print_licence();
		licence_printed=1;
	}
	SIGNAL_INFO ref_info;
	SIGNAL_INFO deg_info;
	ERROR_INFO err_info;

	long Error_Flag = 0;
	char * Error_Type = "Unknown error type.";

	strcpy(ref_info.path_name,reference);
	strcpy(deg_info.path_name,degraded);
	printf("pesq_process ref[%s] deg[%s]\n", ref_info.path_name, deg_info.path_name);

	ref_info.apply_swap = 0; /* Swap byte order - machine native format by default. */
	deg_info.apply_swap = 0;
	ref_info.input_filter = 1;
	deg_info.input_filter = 1;
	err_info.mode = NB_MODE; /* WB_MODE P.862.2 wideband extension (headphone listening) */

	if (sample_rate != 8000 && sample_rate != 16000) {
		printf ("PESQ Error. Must specify either +8000 or +16000 sample frequency option!\n");
		return 0.0f;
	}
	if (sample_rate == 8000L && err_info.mode == WB_MODE ) {
		printf ("PESQ Error. P.862.2 operation must use 16kHz sample rate\n");
		return 0.0f;
	}

	select_rate (sample_rate, &Error_Flag, &Error_Type);
	pesq_measure (&ref_info, &deg_info, &err_info, &Error_Flag, &Error_Type);

	if (Error_Flag == 0) {
		if ( err_info.mode == NB_MODE )
			printf ("\nP.862 Prediction (Raw MOS, MOS-LQO):  = %.3f\t%.3f\n", (double) err_info.pesq_mos, 
			(double) err_info.mapped_mos);
		else
			printf ("\nP.862.2 Prediction (MOS-LQO):  = %.3f\n", (double) err_info.mapped_mos);
		return err_info.mapped_mos;
	} else {
		printf ("An error of type %d ", (int)Error_Flag);
		if (Error_Type != NULL) {
			printf (" (%s) occurred during processing.\n", Error_Type);
		} else {
			printf ("occurred during processing.\n");
		}
		return 0.0f;
	}
}

double align_filter_dB [26] [2] = {{0.,-500},
                                 {50., -500},
                                 {100., -500},
                                 {125., -500},
                                 {160., -500},
                                 {200., -500},
                                 {250., -500},
                                 {300., -500},
                                 {350.,  0},
                                 {400.,  0},
                                 {500.,  0},
                                 {600.,  0},
                                 {630.,  0},
                                 {800.,  0},
                                 {1000., 0},
                                 {1250., 0},
                                 {1600., 0},
                                 {2000., 0},
                                 {2500., 0},
                                 {3000., 0},
                                 {3250., 0},
                                 {3500., -500},
                                 {4000., -500},
                                 {5000., -500},
                                 {6300., -500},
                                 {8000., -500}}; 


double standard_IRS_filter_dB [26] [2] = {{  0., -200},
                                         { 50., -40}, 
                                         {100., -20},
                                         {125., -12},
                                         {160.,  -6},
                                         {200.,   0},
                                         {250.,   4},
                                         {300.,   6},
                                         {350.,   8},
                                         {400.,  10},
                                         {500.,  11},
                                         {600.,  12},
                                         {700.,  12},
                                         {800.,  12},
                                         {1000., 12},
                                         {1300., 12},
                                         {1600., 12},
                                         {2000., 12},
                                         {2500., 12},
                                         {3000., 12},
                                         {3250., 12},
                                         {3500., 4},
                                         {4000., -200},
                                         {5000., -200},
                                         {6300., -200},
                                         {8000., -200}}; 


#define TARGET_AVG_POWER    1E7

void fix_power_level (SIGNAL_INFO *info, char *name, long maxNsamples) 
{
    long   n = info-> Nsamples;
    long   i;
    float *align_filtered = (float *) safe_malloc ((n + DATAPADDING_MSECS  * (Fs / 1000)) * sizeof (float));    
    float  global_scale;
    float  power_above_300Hz;

    for (i = 0; i < n + DATAPADDING_MSECS  * (Fs / 1000); i++) {
        align_filtered [i] = info-> data [i];
    }
    apply_filter (align_filtered, info-> Nsamples, 26, align_filter_dB);

    power_above_300Hz = (float) pow_of (align_filtered, 
                                        SEARCHBUFFER * Downsample, 
                                        n - SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000),
                                        maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000));

    global_scale = (float) sqrt (TARGET_AVG_POWER / power_above_300Hz); 

    for (i = 0; i < n; i++) {
        info-> data [i] *= global_scale;    
    }

    safe_free (align_filtered);
}

long WB_InIIR_Nsos;
float * WB_InIIR_Hsos;
long WB_InIIR_Nsos_8k = 1L;
float WB_InIIR_Hsos_8k[LINIIR] = {
    2.6657628f,  -5.3315255f,  2.6657628f,  -1.8890331f,  0.89487434f };
long WB_InIIR_Nsos_16k = 1L;
float WB_InIIR_Hsos_16k[LINIIR] = {
    2.740826f,  -5.4816519f,  2.740826f,  -1.9444777f,  0.94597794f };
       
void pesq_measure (SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info, long * Error_Flag, char ** Error_Type)
{
    float * ftmp = NULL;

    ref_info-> data = NULL;
    ref_info-> VAD = NULL;
    ref_info-> logVAD = NULL;
    
    deg_info-> data = NULL;
    deg_info-> VAD = NULL;
    deg_info-> logVAD = NULL;
        
    if ((*Error_Flag) == 0)
    {
        printf ("Reading reference file %s...", ref_info->path_name);

       load_src (Error_Flag, Error_Type, ref_info);
       if ((*Error_Flag) == 0)
           printf ("done.\n");
    }
    if ((*Error_Flag) == 0)
    {
        printf ("Reading degraded file %s...", deg_info-> path_name);

       load_src (Error_Flag, Error_Type, deg_info);
       if ((*Error_Flag) == 0)
           printf ("done.\n");
    }

    if (((ref_info-> Nsamples - 2 * SEARCHBUFFER * Downsample < Fs / 4) ||
         (deg_info-> Nsamples - 2 * SEARCHBUFFER * Downsample < Fs / 4)) &&
        ((*Error_Flag) == 0))
    {
        (*Error_Flag) = 2;
        (*Error_Type) = "Reference or Degraded below 1/4 second - processing stopped ";
    }

    if ((*Error_Flag) == 0)
    {
        alloc_other (ref_info, deg_info, Error_Flag, Error_Type, &ftmp);
    }

    if ((*Error_Flag) == 0)
    {   
        int     maxNsamples = max (ref_info-> Nsamples, deg_info-> Nsamples);
        float * model_ref; 
        float * model_deg; 
        long    i;
        FILE *resultsFile;

        printf (" Level normalization...\n");            
        fix_power_level (ref_info, "reference", maxNsamples);
        fix_power_level (deg_info, "degraded", maxNsamples);

        printf (" IRS filtering...\n"); 
        if( Fs == 16000 ) {
            WB_InIIR_Nsos = WB_InIIR_Nsos_16k;
            WB_InIIR_Hsos = WB_InIIR_Hsos_16k;
        } else {
            WB_InIIR_Nsos = WB_InIIR_Nsos_8k;
            WB_InIIR_Hsos = WB_InIIR_Hsos_8k;
        }
        if( ref_info->input_filter == 1 ) {
            apply_filter (ref_info-> data, ref_info-> Nsamples, 26, standard_IRS_filter_dB);
        } else {
            for( i = 0; i < 16; i++ ) {
                ref_info->data[SEARCHBUFFER * Downsample + i - 1]
                    *= (float)i / 16.0f;
                ref_info->data[ref_info->Nsamples - SEARCHBUFFER * Downsample - i]
                    *= (float)i / 16.0f;
            }
            IIRFilt( WB_InIIR_Hsos, WB_InIIR_Nsos, NULL,
                 ref_info->data + SEARCHBUFFER * Downsample,
                 ref_info->Nsamples - 2 * SEARCHBUFFER * Downsample, NULL );
        }
        if( deg_info->input_filter == 1 ) {
            apply_filter (deg_info-> data, deg_info-> Nsamples, 26, standard_IRS_filter_dB);
        } else {
            for( i = 0; i < 16; i++ ) {
                deg_info->data[SEARCHBUFFER * Downsample + i - 1]
                    *= (float)i / 16.0f;
                deg_info->data[deg_info->Nsamples - SEARCHBUFFER * Downsample - i]
                    *= (float)i / 16.0f;
            }
            IIRFilt( WB_InIIR_Hsos, WB_InIIR_Nsos, NULL,
                 deg_info->data + SEARCHBUFFER * Downsample,
                 deg_info->Nsamples - 2 * SEARCHBUFFER * Downsample, NULL );
        }

        model_ref = (float *) safe_malloc ((ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000)) * sizeof (float));
        model_deg = (float *) safe_malloc ((deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000)) * sizeof (float));

        for (i = 0; i < ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
            model_ref [i] = ref_info-> data [i];
        }
    
        for (i = 0; i < deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
            model_deg [i] = deg_info-> data [i];
        }
    
        input_filter( ref_info, deg_info, ftmp );

        printf (" Variable delay compensation...\n");            
        calc_VAD (ref_info);
        calc_VAD (deg_info);
        
        crude_align (ref_info, deg_info, err_info, WHOLE_SIGNAL, ftmp);

        utterance_locate (ref_info, deg_info, err_info, ftmp);
    
        for (i = 0; i < ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
            ref_info-> data [i] = model_ref [i];
        }
    
        for (i = 0; i < deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
            deg_info-> data [i] = model_deg [i];
        }

        safe_free (model_ref);
        safe_free (model_deg); 
    
        if ((*Error_Flag) == 0) {
            if (ref_info-> Nsamples < deg_info-> Nsamples) {
                float *new_ref = (float *) safe_malloc((deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000)) * sizeof(float));
                long  i;
                for (i = 0; i < ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
                    new_ref [i] = ref_info-> data [i];
                }
                for (i = ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); 
                     i < deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
                    new_ref [i] = 0.0f;
                }
                safe_free (ref_info-> data);
                ref_info-> data = new_ref;
                new_ref = NULL;
            } else {
                if (ref_info-> Nsamples > deg_info-> Nsamples) {
                    float *new_deg = (float *) safe_malloc((ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000)) * sizeof(float));
                    long  i;
                    for (i = 0; i < deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
                        new_deg [i] = deg_info-> data [i];
                    }
                    for (i = deg_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); 
                         i < ref_info-> Nsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
                        new_deg [i] = 0.0f;
                    }
                    safe_free (deg_info-> data);
                    deg_info-> data = new_deg;
                    new_deg = NULL;
                }
            }
        }

		printf (" Acoustic model processing...\n");
		pesq_psychoacoustic_model (ref_info, deg_info, err_info, ftmp);

		safe_free (ref_info-> data);
		safe_free (ref_info-> VAD);
		safe_free (ref_info-> logVAD);
		safe_free (deg_info-> data);
		safe_free (deg_info-> VAD);
		safe_free (deg_info-> logVAD);
		safe_free (ftmp);

		if ( err_info->mode == NB_MODE ) {
			err_info->mapped_mos = 0.999f+4.0f/(1.0f+(float)exp((-1.4945f*err_info->pesq_mos+4.6607f)));
		}
		else {
			err_info->mapped_mos = 0.999f+4.0f/(1.0f+(float)exp((-1.3669f*err_info->pesq_mos+3.8224f)));
			err_info->pesq_mos = -1.0;
		}

		resultsFile = fopen (ITU_RESULTS_FILE, "at");

        if (resultsFile != NULL) {
            long start, end;

            if (0 != fseek (resultsFile, 0, SEEK_SET)) {
                printf ("Could not move to start of results file %s!\n", ITU_RESULTS_FILE);
                exit (1);
            }
            start = ftell (resultsFile);

            if (0 != fseek (resultsFile, 0, SEEK_END)) {
                printf ("Could not move to end of results file %s!\n", ITU_RESULTS_FILE);
                exit (1);
            }
            end = ftell (resultsFile);

            if (start == end) {
                fprintf (resultsFile, "REFERENCE\t DEGRADED\t PESQMOS\t MOSLQO\t SAMPLE_FREQ\t MODE\n"); 
				fflush (resultsFile);
            }

            fprintf (resultsFile, "%s\t ", ref_info-> path_name);
            fprintf (resultsFile, "%s\t ", deg_info-> path_name);

			fprintf (resultsFile, "%.3f\t ", err_info->pesq_mos);
            fprintf (resultsFile, "%.3f\t ", err_info->mapped_mos);
            fprintf (resultsFile, "%d\t",(int) Fs);

			if ( err_info->mode == NB_MODE )
				fprintf (resultsFile, "nb");
			else
				fprintf (resultsFile, "wb");

           fprintf (resultsFile, "\n");

           fclose (resultsFile);
        }

    }

    return;
}

/* END OF FILE */
