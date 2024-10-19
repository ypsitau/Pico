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

class TokenHandlerEx : public TokenHandler {
public:
	virtual void DoHandle(Type type) override;
};

void TokenHandlerEx::DoHandle(Type type)
{
	if (type == Type::Symbol) {
		::printf("\"%s\"\n", GetSymbol());
	} else if (type == Type::EndOfLine) {
		::printf("[EOL]\n");
	}
}

int main()
{
	TokenHandlerEx tokenHandler;
	stdio_init_all();

	// I2C Initialisation. Using it at 400Khz.
	i2c_init(I2C_PORT, 400*1000);
	
	gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C_SDA);
	gpio_pull_up(I2C_SCL);
	// For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

	std::unique_ptr<TCPServer> pTCPServer(new TCPServer(4242, tokenHandler));
	if (!TCPServer::ConnectWifi(WIFI_SSID, WIFI_PASSWORD, 30000)) {
		::printf("failed to connect to: %s\n", WIFI_SSID);
		return 1;
	}
	::printf("Starting server at %s on port %u\n",
			::ip4addr_ntoa(netif_ip4_addr(netif_list)), pTCPServer->GetPort());
	if (!pTCPServer->WaitForClient()) {
		pTCPServer->Close();
		return 1;
	}
	for (;;) {
		TCPServer::PollWifi(1000);
	}
	//TCPServer::DisconnectWifi();
	return 0;
}
