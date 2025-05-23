/*

This sketch shows the Ethernet event usage with ESP32-POE2. Compile and download the code and open serial monitor to check if the boards connects to google.com.

Remember to select proper board in the board selector.
ESP32-POE2 has WROVER module with PSRAM. You should have PSRAM enabled! From Tools -> PSRAM -> Enabled.
Make sure proper COM port is selected.

*/

#include <ETH.h>

#define ETH_PHY_TYPE ETH_PHY_LAN8720
#define ETH_PHY_ADDR 0
#define ETH_PHY_MDC 23
#define ETH_PHY_MDIO 18
#define ETH_PHY_POWER 12
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_OUT

static bool eth_connected = false;

// WARNING: onEvent is called from a separate FreeRTOS task (thread)!
void onEvent(arduino_event_id_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("ETH Started");
    // The hostname must be set after the interface is started, but needs
    // to be set before DHCP, so set it from the event handler thread.
    ETH.setHostname("esp32-ethernet");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.println("ETH Got IP");
    Serial.println(ETH);
    eth_connected = true;
    break;
  case ARDUINO_EVENT_ETH_LOST_IP:
    Serial.println("ETH Lost IP");
    eth_connected = false;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    eth_connected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    eth_connected = false;
    break;
  default:
    break;
  }
}

void testClient(const char *host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  NetworkClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

void setup()
{
  Serial.begin(115200);
  Serial.printf("PSRAM size: %u bytes\n", ESP.getPsramSize());
  Network.onEvent(onEvent);
  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  Serial.printf(PSTR("ETH "
                     "Config: type=%d, addr=%d, mdc=%d, mdio=%d, power=%d, clk_mode=%d"),
                ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);
  // ETH Config: type=1, addr=0, mdc=23, mdio=18, power=12, clk_mode=1
}

void loop()
{
  if (eth_connected)
  {
    testClient("google.com", 80);
  }
  delay(1000);
}
