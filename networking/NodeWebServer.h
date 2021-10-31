// Example usage :

// NodeWebServer server("SSID", "PASSWORD", 80);
//   server.AddHandler("/", [&server]() 
//   {
//     server.Send(200, server.GetValuesJson());
//   });
//   server.AddHandler("/gassensor", [&server]() {
//   	server.Send(200, todo_some_json)
//   })

// void loop() 
// {
// 	server.SetValue(SENSOR_ONE_ID, 16);
// 	server.Loop();
// }

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
    NodeWebServer(const char* ssid, const char* pwd, uint16_t port)
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

    const String GetValuesJson() const
    {
      String response = "{\"values\":{";
      for(const auto& kvp : m_values)
      {
        response += "\"";
        response += kvp.first.c_str();
        response += "\":";
        response += std::to_string(kvp.second).c_str();
        response += "},";
      }
      response += "\"version\":1}";
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