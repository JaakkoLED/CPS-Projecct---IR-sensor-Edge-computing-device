#include <WiFiNINA.h>
#include "Ultrasonic.h"

#define SYSTEM_PIN 3  // Indication that the system is ON or OFF. The signal is coming from the Arduino Uno


char ssid[] = " ";
char pass[] = " ";

WiFiServer server(80);
const int ledPin = 0;

Ultrasonic ultrasonic(1);  


// Ultrasonic sensor measures every 200 ms (memory)
volatile float cachedDistance = 0.0;
volatile unsigned long lastUltrasonicRead = 0;
const unsigned long ULTRASONIC_INTERVAL = 200;  // ms


// List of allowed IP-addresses 
IPAddress sallitutIP[] = {
  IPAddress(xxx, xxx, xxx, xxx),  
  IPAddress(xxx, xxx, xxx, xxx),  
  IPAddress(xxx, xxx, xxx, xxx)   
};
const int sallittujenMaara = 3;  

void setup()
{
  Serial.begin(9600);
  delay(2000);  
  
  pinMode(ledPin, OUTPUT);
  pinMode(SYSTEM_PIN, INPUT);  
  digitalWrite(ledPin, LOW);
  
  Serial.println("WiFi yhdistäminen käynnissä...");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  
  Serial.println("\nYhdistetty!");
  Serial.print("Arduino IP: http://");
  Serial.println(WiFi.localIP());
  Serial.println("\n--- SALLITUT IP-OSOITTEET ---");

  for (int i = 0; i < sallittujenMaara; i++)
  {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(sallitutIP[i]);
  }

  Serial.println("-----------------------------\n");
  
  digitalWrite(ledPin, HIGH);
  server.begin();
}

void loop()
{
  paivitaUltrasonic();
  
  static unsigned long viimeinenTarkistus = 0;

  if (millis() - viimeinenTarkistus > 5000)
  {
    viimeinenTarkistus = millis();
    
    if (WiFi.status() != WL_CONNECTED)
    {
      // WiFi disconnected
      digitalWrite(ledPin, LOW);  // Turn the LED off
      Serial.println("⚠️ WiFi katkennut! Yritetään uudelleen...");
      WiFi.begin(ssid, pass);
      
      int yritys = 0;

      while (WiFi.status() != WL_CONNECTED && yritys < 20)
      {
        delay(500);
        Serial.print(".");
        yritys++;
      }
      
      if (WiFi.status() == WL_CONNECTED)
      {
        digitalWrite(ledPin, HIGH);  
        Serial.println("\n✅ Yhdistetty uudelleen!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
      }

      else
      {
        Serial.println("\n❌ Yhdistäminen epäonnistui");
      } 
    }


    else
    {
      digitalWrite(ledPin, HIGH);
    }
  }
  

  WiFiClient client = server.available();
  
  if (client)
  {
    IPAddress clientIP = client.remoteIP();
    
    Serial.print("Yhteys: ");
    Serial.print(clientIP);
    
    
    if (!onkoSallittu(clientIP))
    {
      Serial.println(" ❌ ESTETTY!");
      
      // Send 403 Forbidden
      client.println("HTTP/1.1 403 Forbidden");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE HTML> <html><head><title> 403 Forbidden </title></head>");
      client.println("<body style='font-family:Arial;text-align:center;padding:50px;background:#ff5555;color:#fff;'>");
      client.println("<h1> Access denied !</h1>");
      client.println("<p>IP-Address is not on the whitelist.</p>");
      client.println("<p>IP: " + clientIP.toString() + "</p>");
      client.println("</body></html>");
      
      client.stop();
      
      return;
    }
    
    Serial.println(" ✅ SALLITTU");
    
    String req = "";
    
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        req += c;
        
        // DoS-protection: restrict the size of the requests
        if (req.length() > 2048)
        {
          client.println("HTTP/1.1 413 Request Too Large");
          client.stop();

          return;
        }
        
        if (c == '\n' && req.endsWith("\r\n\r\n"))
        {
          
          if (req.indexOf("GET /data.json") >= 0)
          {
            lahetaJSON(client);
          }

          else
          {
            lahetaHTML(client);
          }
          break;
        }
      }
    }

    client.stop();
  }
}

// IpCheck
bool onkoSallittu(IPAddress ip)
{
  for (int i = 0; i < sallittujenMaara; i++)
  {
    if (ip == sallitutIP[i])
    {
      return true;
    }
  }

  return false;
}

// Check system status Arduino Uno:sta
// INDICATION pin: HIGH = system OFF, LOW = system ON
bool onkoSystemPaalla()
{
  return digitalRead(SYSTEM_PIN) == LOW;
}

void paivitaUltrasonic()
{
  unsigned long currentTime = millis();

  if (currentTime - lastUltrasonicRead > ULTRASONIC_INTERVAL)
  {
    lastUltrasonicRead = currentTime;
    cachedDistance = ultrasonic.MeasureInCentimeters();
  }

}

void lahetaJSON(WiFiClient &client)
{
  bool systemPaalla = onkoSystemPaalla();
  
  String json = "{\"system\":\"";
  json += systemPaalla ? "ON" : "OFF";
  json += "\"";
  
  if (systemPaalla)
  {
    json += ",\"etaisyys\":" + String(cachedDistance, 1);
  }

  else
  {
    json += ",\"etaisyys\":0.0";
  }
  
  json += "}";
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(json);
}

void lahetaHTML(WiFiClient &client)
{
  bool systemPaalla = onkoSystemPaalla();
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE HTML><html><head><meta charset='UTF-8'>");
  client.println("<title> Distance Meter </title>");
  client.println("<style>body{font-family:Arial;text-align:center;padding:50px;background:#667eea;}");
  client.println(".box{background:#fff;padding:30px;border-radius:20px;display:inline-block;box-shadow:0 10px 30px rgba(0,0,0,0.3);}");
  client.println(".val{font-size:80px;color:#667eea;font-weight:bold;margin:20px 0;}");
  client.println(".unit{font-size:30px;color:#666;}");
  client.println(".secure{background:#4caf50;color:#fff;padding:5px 15px;border-radius:20px;font-size:12px;margin:10px;}");
  client.println(".status{font-size:20px;font-weight:bold;padding:10px;border-radius:10px;margin:10px 0;}");
  client.println(".status-on{background:#4caf50;color:#fff;}");
  client.println(".status-off{background:#f44336;color:#fff;}");
  client.println(".off-message{color:#f44336;font-weight:bold;font-size:18px;}</style></head><body>");
  
  client.println("<div class='box'><h1>📏 Etäisyysmittari</h1>");
  client.println("<div class='secure'>🔒 IP-suojattu</div>");
  
  // System status näyttö
  String statusKlassi = systemPaalla ? "status-on" : "status-off";
  String statusTeksti = systemPaalla ? "🟢 SYSTEM ON" : "🔴 SYSTEM OFF";
  client.println("<div class='status " + statusKlassi + "'>" + statusTeksti + "</div>");
  
  
  if (systemPaalla)
  {
    client.println("<div class='val' id='dist'>--</div>");
    client.println("<div class='unit'> cm </div>");
  }

  else
  {
    client.println("<div class='off-message'> System on offline </div>");
  }
  
  client.println("</div>");
  client.println("<script>function u(){fetch('/data.json').then(r=>r.json()).then(d=>{");
  client.println("var status=document.querySelector('.status');");
  client.println("if(d.system==='ON'){");
  client.println("status.className='status status-on';");
  client.println("status.innerText='🟢 SYSTEM ON';");
  client.println("document.getElementById('dist').innerText=d.etaisyys.toFixed(1);");
  client.println("}else{");
  client.println("status.className='status status-off';");
  client.println("status.innerText='🔴 SYSTEM OFF';}");
  client.println("})}");
  client.println("u();setInterval(u,100);</script></body></html>");
}
