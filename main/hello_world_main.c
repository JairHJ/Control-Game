#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_system.h"   // <--- para esp_reset_reason()

static const char *TAG = "NVS_COUNTER";

void app_main() {
    esp_err_t err;

    // Inicializar NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) abriendo NVS handle!", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "NVS abierto");

        // Verificar si ya se inicializó (flag que persiste entre reinicios)
        int8_t initialized = 0;
        err = nvs_get_i8(my_handle, "init_flag", &initialized);

        if (err == ESP_ERR_NVS_NOT_FOUND) {
            // Es la primera vez que corre este firmware → resetear contador
            ESP_LOGI(TAG, "Primer arranque: reiniciando contador");

            // Borrar contador previo (por si existe)
            nvs_erase_key(my_handle, "boot_count");

            // Establecer flag de inicialización para no repetir en próximos arranques
            nvs_set_i8(my_handle, "init_flag", 1);
            nvs_commit(my_handle);
        }

        // Leer contador
        int32_t counter = 0;
        err = nvs_get_i32(my_handle, "boot_count", &counter);
        switch (err) {
            case ESP_OK:
                ESP_LOGI(TAG, "Boot count leído: %d", counter);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "Boot count no encontrado, empezando en 0");
                break;
            default:
                ESP_LOGE(TAG, "Error leyendo boot_count: %s", esp_err_to_name(err));
        }

        // Obtener la razón del reset
        esp_reset_reason_t reset_reason = esp_reset_reason();
        ESP_LOGI(TAG, "Razón del reset: %d", reset_reason);

        // Incrementar contador solo si el reset fue por power on (desconexión USB)
        if (reset_reason == ESP_RST_POWERON) {
            counter++;
            nvs_set_i32(my_handle, "boot_count", counter);
            nvs_commit(my_handle);
            ESP_LOGI(TAG, "Boot count actualizado (desconexión USB): %d", counter);
        } else {
            ESP_LOGI(TAG, "Reset no causado por desconexión USB, contador sin cambios.");
        }

        nvs_close(my_handle);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}