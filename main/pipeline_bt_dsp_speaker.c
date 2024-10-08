/* Play music from Bluetooth device

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "periph_touch.h"
#include "periph_adc_button.h"
#include "periph_button.h"
#include "board.h"
#include "filter_resample.h"
#include "audio_mem.h"
#include "bluetooth_service.h"
#include <math.h>
//#include "esp_dsp.h"

#include "Biquad.h"
uint8_t fs;

static const char *TAG = "BLUETOOTH_EXAMPLE";


static void bt_app_avrc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *p_param)
{
    esp_avrc_ct_cb_param_t *rc = p_param;
    switch (event) {
        case ESP_AVRC_CT_METADATA_RSP_EVT: {
            uint8_t *tmp = audio_calloc(1, rc->meta_rsp.attr_length + 1);
            memcpy(tmp, rc->meta_rsp.attr_text, rc->meta_rsp.attr_length);
            ESP_LOGI(TAG, "AVRC metadata rsp: attribute id 0x%x, %s", rc->meta_rsp.attr_id, tmp);
            audio_free(tmp);
            break;
        }
        default:
            break;
    }
}
char DspBuf[4096];

audio_element_info_t Dsp_info = { 0 };

static esp_err_t Dsp_open(audio_element_handle_t self)
	{
	audio_element_getinfo(self, &Dsp_info);	
	return ESP_OK;
	}

static esp_err_t Dsp_close(audio_element_handle_t self)
	{
	return ESP_OK;
	}

audio_element_err_t Dsp_read(audio_element_handle_t el, char *buf, int len, unsigned int wait_time, void *ctx)
	{
	return (audio_element_err_t)len;
	}

audio_element_err_t Dsp_write(audio_element_handle_t el, char *buf, int len, unsigned int wait_time, void *ctx)
	{
	return (audio_element_err_t)len;
	}


static audio_element_err_t Dsp_process(audio_element_handle_t self, char *inbuf, int len)
	{
	audio_element_input(self, (char *)DspBuf, len);
	
	// ********** PROCESS the buffer with DSP IIR !!
	
	process_data_mono(DspBuf, len);
	//process_data_stereo(DspBuf, len);
	
	int ret = audio_element_output(self, (char *)DspBuf, len);
	return (audio_element_err_t)ret;
	}
	
static esp_err_t Dsp_destroy(audio_element_handle_t self)
	{
	return ESP_OK;
	}
	

void app_main(void)
{
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t bt_stream_reader, DspProcessor, i2s_stream_writer;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "[ 1 ] Create Bluetooth service");
    bluetooth_service_cfg_t bt_cfg = {
        .device_name = "ESP-ADF-SPEAKER",
        .mode = BLUETOOTH_A2DP_SINK,
        .user_callback.user_avrc_ct_cb = bt_app_avrc_ct_cb,
    };
    bluetooth_service_start(&bt_cfg);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
		
    ESP_LOGI(TAG, "[ 3 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[3.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    
    i2s_stream_set_clk(i2s_stream_writer, 44100, 16, 2);

    ESP_LOGI(TAG, "[3.2] Get Bluetooth stream");
    bt_stream_reader = bluetooth_service_create_stream();
	
	
	
    
    audio_element_cfg_t DspCfg; // = DEFAULT_AUDIO_ELEMENT_CONFIG();
	memset(&DspCfg, 0, sizeof(audio_element_cfg_t));
	DspCfg.destroy = Dsp_destroy;
	DspCfg.process = Dsp_process;
	DspCfg.read = Dsp_read; // why are these needed if they are not called?
	DspCfg.write = Dsp_write; // why are these needed if they are not called?
	DspCfg.open = Dsp_open;
	DspCfg.close = Dsp_close;
	DspCfg.buffer_len = (4096);
	DspCfg.tag = "DSPProcessor";
	DspCfg.task_stack = (2 * 1024);
	DspCfg.task_prio = (23);
	DspCfg.task_core = (1); 
	DspCfg.out_rb_size = (8 * 1024);
	
	
	
   /*
    eq_cfg.out_rb_size = EQUALIZER_RINGBUFFER_SIZE;
    eq_cfg.channel = 2;
    eq_cfg.samplerate = 44100;
    eq_cfg.task_prio = EQUALIZER_TASK_PRIO;
    */
  //   equalizer = equalizer_init(&eq_cfg);
  
     ESP_LOGI(TAG, "[3.2B] Intitialize DSP Element");
     DspProcessor = audio_element_init(&DspCfg);
     create_biquad();
	
    ESP_LOGI(TAG, "[3.2] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, bt_stream_reader, "bt");
  	audio_pipeline_register(pipeline, DspProcessor, "DspProcessor");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[3.3] Link it together [Bluetooth]-->bt_stream_reader-->DspProcessor-->i2s_stream_writer-->[codec_chip]");


  //  audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);
   // audio_pipeline_register(pipeline, filter, "filter");
   
    
    const char *link_tag[3] = {"bt", "DspProcessor", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

	//audio_pipeline_relink(pipeline, &link_tag[0], 3);
	
    ESP_LOGI(TAG, "[ 4 ] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[4.1] Initialize Touch peripheral");
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[4.2] Create Bluetooth peripheral");
    esp_periph_handle_t bt_periph = bluetooth_service_create_periph();

    ESP_LOGI(TAG, "[4.2] Start all peripherals");
    esp_periph_start(set, bt_periph);

    ESP_LOGI(TAG, "[ 5 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[5.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[5.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[ 6 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);
  	

    ESP_LOGI(TAG, "[ 7 ] Listen for all pipeline events");
    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }
		
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) bt_stream_reader
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(bt_stream_reader, &music_info);
            

            ESP_LOGI(TAG, "[ * ] Receive music info from Bluetooth, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);
                     
                     fs = 2;
 //           if (Dsp_info(DspProcessor, music_info.sample_rates, music_info.channels) != ESP_OK) {
   //             break;
     //       }
             ESP_LOGI(TAG, "[ * ]  sample_rates=%d, ch=%d",  music_info.sample_rates, music_info.channels);
             
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;

            audio_element_set_music_info(i2s_stream_writer, music_info.sample_rates, music_info.channels, music_info.bits);
#if (CONFIG_ESP_LYRATD_MSC_V2_1_BOARD || CONFIG_ESP_LYRATD_MSC_V2_2_BOARD)
#else
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
#endif
            continue;
        }
		
        if ((msg.source_type == PERIPH_ID_TOUCH || msg.source_type == PERIPH_ID_BUTTON || msg.source_type == PERIPH_ID_ADC_BTN)
            && (msg.cmd == PERIPH_TOUCH_TAP || msg.cmd == PERIPH_BUTTON_PRESSED || msg.cmd == PERIPH_ADC_BUTTON_PRESSED)) {

            if ((int) msg.data == get_input_play_id()) {
                ESP_LOGI(TAG, "[ * ] [Play] touch tap event");
                periph_bluetooth_play(bt_periph);
            } else if ((int) msg.data == get_input_set_id()) {
                ESP_LOGI(TAG, "[ * ] [Set] touch tap event");
                periph_bluetooth_pause(bt_periph);
            } else if ((int) msg.data == get_input_volup_id()) {
                ESP_LOGI(TAG, "[ * ] [Vol+] touch tap event");
                periph_bluetooth_next(bt_periph);
            } else if ((int) msg.data == get_input_voldown_id()) {
                ESP_LOGI(TAG, "[ * ] [Vol-] touch tap event");
                periph_bluetooth_prev(bt_periph);
            }
        }

        /* Stop when the Bluetooth is disconnected or suspended */
        if (msg.source_type == PERIPH_ID_BLUETOOTH
            && msg.source == (void *)bt_periph) {
            if (msg.cmd == PERIPH_BLUETOOTH_DISCONNECTED) {
                ESP_LOGW(TAG, "[ * ] Bluetooth disconnected");
                break;
            }
        }
        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }

    ESP_LOGI(TAG, "[ 8 ] Stop audio_pipeline");
   
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, bt_stream_reader);
    audio_pipeline_unregister(pipeline, DspProcessor);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
   

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */


    audio_pipeline_deinit(pipeline);
    audio_element_deinit(bt_stream_reader);
    audio_element_deinit(DspProcessor);
    audio_element_deinit(i2s_stream_writer);
    
    esp_periph_set_destroy(set);
    bluetooth_service_destroy();
    esp_restart();


   
  //  esp_bluedroid_disable();
   // esp_bluedroid_deinit();
//    esp_bt_controller_disable();
 //   esp_bt_controller_deinit();
  //  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  
}
