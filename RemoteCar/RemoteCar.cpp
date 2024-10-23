#include <stdio.h>
#include <memory>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "jxglib/SSD1306.h"
#include "jxglib/Font-Shinonome16.h"
#include "TCPServer.h"
#include "StepMotor.h"

SSD1306 oled(i2c_default);

StepMotor stepMotorL(0, 6, 400);
StepMotor stepMotorR(1, 10, 400);

class EventHandlerEx : public EventHandler {
public:
	virtual void OnClientConnected(TCPServer& tcpServer, const ip_addr_t& addr) override;
	virtual void OnCharRecv(char ch) override;
	virtual void DoHandle(Type type) override;
};

void EventHandlerEx::OnClientConnected(TCPServer& tcpServer, const ip_addr_t& addr)
{
	oled.Clear();
	oled.DrawString(0, 0, "Client Ready");
	oled.DrawString(0, 32, ::ip4addr_ntoa(&addr));
	oled.Refresh();
	tcpServer.SendString("Ready\r\n");
	tcpServer.SendString("Left  Right\r\n");
	tcpServer.SendString("[Q]   [W]   Forward\r\n");
	tcpServer.SendString("[A]   [S]   Stop\r\n");
	tcpServer.SendString("[Z]   [X]   Back\r\n");
}

void EventHandlerEx::OnCharRecv(char ch)
{
	switch (ch) {
	case 'q': {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Motor-L Fwd");
			oled.Refresh();
		} while (0);
		stepMotorL.Stop();
		stepMotorL.StartFullB(-1);
		break;
	}
	case 'w': {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Motor-R Fwd");
			oled.Refresh();
		} while (0);
		stepMotorR.Stop();
		stepMotorR.StartFullB(-1);
		break;
	}
	case 'a': {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Motor-L Stop");
			oled.Refresh();
		} while (0);
		stepMotorL.Stop();
		break;
	}
	case 's': {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Motor-R Stop");
			oled.Refresh();
		} while (0);
		stepMotorR.Stop();
		break;
	}
	case 'z': {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Motor-L Back");
			oled.Refresh();
		} while (0);
		stepMotorL.Stop();
		stepMotorL.StartFullA(-1);
		break;
	}
	case 'x': {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Motor-R Back");
			oled.Refresh();
		} while (0);
		stepMotorR.Stop();
		stepMotorR.StartFullA(-1);
		break;
	}
	}
}

void EventHandlerEx::DoHandle(Type type)
{
	if (type == Type::Symbol) {
		::printf("\"%s\"\n", GetSymbol());
	} else if (type == Type::EndOfLine) {
		::printf("[EOL]\n");
	}
}

int main()
{
	EventHandlerEx EventHandler;
	std::unique_ptr<TCPServer> pTCPServer(new TCPServer(4242, EventHandler));
	StepMotor::Initialize(pio0);
	stepMotorL.Enable();
	stepMotorR.Enable();
	stdio_init_all();
	i2c_init(i2c_default, 400 * 1000);
	::gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	::gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	::gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	::gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
	oled.Initialize();
	oled.Clear();
	oled.Refresh();
	oled.SetFont(Font::Shinonome16::fontSet);
	oled.SetFontScale(1, 1);
	do {
		oled.Clear();
		oled.DrawString(0, 0, "Connecting:");
		oled.DrawString(0, 32, WIFI_SSID);
		oled.Refresh();
	} while (0);
	if (!TCPServer::ConnectWifi(WIFI_SSID, WIFI_PASSWORD, 30000)) {
		do {
			oled.Clear();
			oled.DrawString(0, 0, "Failed:");
			oled.DrawString(0, 32, WIFI_SSID);
			oled.Refresh();
		} while (0);
		return 1;
	}
	do {
		char str[128];
		oled.Clear();
		oled.DrawString(0, 0, "Connected:");
		oled.DrawString(0, 32, ::ip4addr_ntoa(netif_ip4_addr(netif_list)));
		::sprintf(str, "Port %d", pTCPServer->GetPort());
		oled.DrawString(0, 48, str);
		oled.Refresh();
	} while (0);
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
