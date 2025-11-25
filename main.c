#include <stdio.h>
#include<esp_log.h>
#include<driver/i2c.h>

static const char *TAG = "SLAVE";


#define I2C_SLAVE_SCL_IO 22
#define I2C_SLAVE_SDA_IO 21
#define I2C_SLAVE_NUM 0 
#define DATA_LENGTH 512
#define RW_TEST_LENGTH 128
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)
#define I2C_SLAVE_ADDR 0x04


static esp_err_t i2c_slave_init(void){

    int i2c_slave_port = I2C_SLAVE_NUM;

    i2c_config_t conf_slave = {
        .sda_io_num= I2C_SLAVE_SDA_IO,
        .sda_pullup_en =GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .scl_pullup_en =GPIO_PULLUP_ENABLE,
        .mode = I2C_MODE_SLAVE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = I2C_SLAVE_ADDR,
        .clk_flags = 0,
    };

    i2c_param_config(i2c_slave_port, &conf_slave);

    return i2c_driver_install(i2c_slave_port, conf_slave.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN,0);


}



void app_main(void)
{

    uint8_t data[DATA_LENGTH];
    uint8_t *ptr;
    uint8_t pos;

    ESP_ERROR_CHECK(i2c_slave_init());
    ESP_LOGI(TAG, "I2C initialized succesfully");

    for(uint16_t i = 0; i< DATA_LENGTH; i++){
        data[i] = i;
    }
    

    pos = 0;
    ptr = &data[pos];

    while(1){
        uint16_t d_size = i2c_slave_read_buffer(I2C_SLAVE_NUM, ptr, RW_TEST_LENGTH, 1000 / portTICK_PERIOD_MS);

        if( d_size == 0){
            ESP_LOGI(TAG, "i2c slave tx buffer full");
        }
        else{
            pos = (pos + d_size) % DATA_LENGTH;
        }

        ptr= &data[pos];
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }


    ESP_ERROR_CHECK(i2c_driver_delete(I2C_SLAVE_NUM));
    ESP_LOGI(TAG, "I2C unitialized successfully");

}