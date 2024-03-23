#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <fvad.h>
#include <stdint.h>
#include <stdbool.h>

#include "testdata/this_is_test.h"

static const char *TAG = "libfvad";

#define TEST_SAMPLE_RATE	16000
#define TEST_FRAME_MS		20
#define TEST_FRAME_LEN		(TEST_SAMPLE_RATE / 1000 * 10)

static void process_test(Fvad *vad, bool show)
{
	int vadres;
	unsigned int start_time;
	unsigned int end_time;
	unsigned int count;
	int16_t *frame_s = (int16_t *)this_is_test_pcm;
	unsigned int pcm_len = this_is_test_pcm_len / 2;

	printf("audio len: %dms\n", pcm_len * 1000 / TEST_SAMPLE_RATE);
	printf("frame len: %dms\n", TEST_FRAME_MS);

	start_time = esp_log_timestamp();
	for (count = 0; count + TEST_FRAME_LEN < pcm_len; count += TEST_FRAME_LEN) {
		vadres = fvad_process(vad, frame_s + count, TEST_FRAME_LEN);
		if (vadres < 0) {
			ESP_LOGE(TAG, "VAD processing failed");
			continue;
		}

		vadres = !!vadres;
		if (show)
			printf("%c", vadres ? '#' : '-');
	}
	if (show)
		printf("\n");

	end_time = esp_log_timestamp();
	ESP_LOGI(TAG, "%s:%d:  +%ums", __func__, __LINE__, end_time - start_time);
}

void app_main(void)
{
	int size = esp_get_free_heap_size();
	Fvad *vad = NULL;
	bool show = true;

	ESP_LOGI(TAG, "Hello! free heap size: %dbyte", size);

	vad = fvad_new();
	if (!vad) {
		ESP_LOGE(TAG, "new fvad fail");
		return;
	}

	if (fvad_set_sample_rate(vad, TEST_SAMPLE_RATE) < 0) {
		ESP_LOGE(TAG, "invalid sample rate: %d Hz\n", TEST_SAMPLE_RATE);
		goto end;
	}

	while (1) {
		process_test(vad, show);
		vTaskDelay(pdMS_TO_TICKS(1000));
		show = !show;
	}

end:
	fvad_free(vad);
}
