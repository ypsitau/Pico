#include <stdio.h>
#include <memory>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "TCPServer.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

int main()
{
	stdio_init_all();

	// I2C Initialisation. Using it at 400Khz.
	i2c_init(I2C_PORT, 400*1000);
	
	gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C_SDA);
	gpio_pull_up(I2C_SCL);
	// For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

	std::unique_ptr<TCPServer> pTCPServer(new TCPServer());
	if (!TCPServer::ConnectWifi(WIFI_SSID, WIFI_PASSWORD, 30000)) {
		::printf("failed to connect: %s\n", WIFI_SSID);
		return 1;
	}
	if (!pTCPServer->Wait(4242)) {
		pTCPServer->Complete(-1);
		return 1;
	}
	while (!pTCPServer->complete_) {
		TCPServer::PollWifi(1000);
	}
	TCPServer::DisconnectWifi();
	return 0;
}
