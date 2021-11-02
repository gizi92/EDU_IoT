/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#ifndef NODEWEBSERVER_H_
#define NODEWEBSERVER_H_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <memory>
#include <unordered_map>
#include <string>

typedef std::unique_ptr<ESP8266WebServer> WebServerUPtr;

class NodeWebServer
{
  public:
    NodeWebServer(uint16_t port)
    {
      m_webServerPtr = std::make_unique<ESP8266WebServer>(port);
    }

    void SetValue(const String& valueType, uint32_t value)
    {
      m_values[valueType.c_str()] = value;
    }

    const uint32_t GetValue(const String& valueType) const
    {
      if(m_values.count(valueType.c_str()) > 0)
        return m_values.at(valueType.c_str());

      return 0;
    }

    const WebServerUPtr& Get() const
    {
      return m_webServerPtr;
    }

    template <typename Handler>
    void AddHandler(const char* route, Handler handler)
    {
      m_webServerPtr->on(route, handler);
    }

    template <typename Handler>
    void AddNotFoundHandler(Handler handler)
    {
      m_webServerPtr->onNotFound(handler);
    }

    const String GetValuesJson() const
    {
      String response = "{\"values\":{";
      
      for(const auto& kvp : m_values)
      {
        response += "\"";
        response += kvp.first.c_str();
        response += "\":";
        response += std::to_string(kvp.second).c_str();
        response += ",";
      }
      response += "\"version\":1}}";

      return response;
    }

    void Send(int responseCode, const String& response)
    {
      m_webServerPtr->send(responseCode, "text/json", response);
    }

    void Loop()
    {
      m_webServerPtr->handleClient();
    }

    void Start()
    {
      m_webServerPtr->begin();
    }

  private:
    WebServerUPtr m_webServerPtr;
    std::unordered_map<std::string, uint32_t> m_values;
};

#endif

const char* ssid     = "SimpsonsDidIt";
const char* password = "Ezegyhosszujelszo576";

NodeWebServer webServer(80);
ESP8266WebServer basicServer(80);

void setup() 
{
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.print("[NODE] Connecting to");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("[Node] Connected to WiFi with IP ");
  Serial.println(WiFi.localIP());

  //basicServer.on("/", handle_onConnect);
  //basicServer.begin();

  webServer.AddHandler("/", [&webServer] {
    webServer.Send(200, webServer.GetValuesJson());
    Serial.println("[NODE] Sent values");
  });

  webServer.AddHandler("/turnonled", [&webServer] {
    webServer.Send(200, "{\"response\":\"Led turned on\"}");
    Serial.println("[NODE] Turning on LED");
  });

  webServer.AddHandler("/turnoffled", [&webServer] {
    webServer.Send(200, "{\"response\":\"Led turned off\"}");
    Serial.println("[NODE] Turning off LED");
  });

  webServer.AddNotFoundHandler([&webServer] {
    webServer.Send(404, "{\"response\":\"Not found\"}");
    Serial.println("[NODE] 404 not found");
  });
  
  webServer.Start();
}

void loop() 
{
  webServer.SetValue("gas1", 15);
  webServer.SetValue("light", 485);
  webServer.Loop(); 

  delay(100);
}
