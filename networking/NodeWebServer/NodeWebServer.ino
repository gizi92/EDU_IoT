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

    const String GetHTML() const
    {
      String response = "<!DOCTYPE html><html lang=\"en\"><head><title>Bootstrap Example</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\"><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js\"></script><script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js\"></script></head><body><div class=\"jumbotron text-center\"><h1>Amazing Technology Device</h1><p>Changing the world one led at a time</p></div><div class=\"container\"><div class=\"row justify-content-md-center\"><div class=\"col col-md-6\"><h1>Sensor data</h1><table class=\"table\"><thead><tr><th scope=\"col\">Sensor</th><th scope=\"col\">Value</th></tr>";
      for(const auto& kvp : m_values)
      {
        response += "<tr><th scope=\"row\">";
        response += kvp.first.c_str();
        response += "</th><td>";
        response += std::to_string(kvp.second).c_str();
        response += "</td></tr>";
      }
      response += "</thead></table></div></div><div class=\"row justify-content-md-center\"><div class=\"col col-md-6\"><h1>Control</h1><table class=\"table\"><thead><tr><th scope=\"col\">Device</th><th scope=\"col\"></th></tr><tr><th scope=\"row\">LED1</th><td><a href=\"toggleled1\"  type=\"submit\" class=\"btn btn-success\">Toggle</button></td></tr></thead></table></div></div></div></body></html>";
    
      return response;
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

    void SendJson(int responseCode, const String& response)
    {
      m_webServerPtr->send(responseCode, "text/json", response);
    }

    void SendHTML(int responseCode, const String& response)
    {
      m_webServerPtr->send(responseCode, "text/html", response);
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
    webServer.SendHTML(200, webServer.GetHTML());
    Serial.println("[NODE] Sent values");
  });

  webServer.AddHandler("/toggleled1", [&webServer] {
    webServer.SendHTML(200, webServer.GetHTML());
    Serial.println("[NODE] Toggling LED");
  });

  webServer.AddNotFoundHandler([&webServer] {
    webServer.SendJson(404, "{\"response\":\"Not found\"}");
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