// Ambient module that posts to Web Server, and saves data to SD card.

//#include "SD.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "DHTesp.h"

DHTesp dht;

const char* ssid = "INFINITUM73B1";
const char* password = "0525958116";

// declare an object of ESP8266WebServer library
WiFiServer server(80);    // Create a webserver object that listens for HTTP request on port 80

// current temperature & humidity, updated in loop()
static char celsiusTemp[7];
static char humidityTemp[7];
//CS pin on SD card
//#define CS_PIN D8;

void setup(void)
{ 
  // Start COM port
  Serial.begin(9600);
  
  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected!");
  // Print the IP address
  Serial.print("IP, ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC, ");
  Serial.println(WiFi.macAddress());
  Serial.println("");

  server.begin();
  Serial.println("HTTP server started :)");
  
  //Setup DHT11 Sensor
  //Serial.println("Status\tHum (%)\tTemp(C)\tTemp(F)\tHI(C)\tHI(F)");
  dht.setup(0, DHTesp::DHT11); // Select pin & sensor model -- GPIO 0 = D3
}

// To handle the actual incoming HTTP requests, we need to call the handleClient() method on the server object. 
// We also change the state of LED as per the request.
void loop() {
  // Listenning for new clients
  WiFiClient client = server.available();
  
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
            float h = dht.getHumidity();
            // Read temperature as Celsius (the default)
            float t = dht.getTemperature();
            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t)) {
              Serial.println("Failed to read from DHT sensor!");
              strcpy(celsiusTemp,"Failed");
              strcpy(humidityTemp, "Failed");         
            }
            else{
              // Computes temperature values in Celsius and Humidity
              float hic = dht.computeHeatIndex(t, h, false);       
              dtostrf(hic, 6, 2, celsiusTemp);             
              dtostrf(h, 6, 2, humidityTemp);
              // You can delete the following Serial.print's, it's just for debugging purposes
              Serial.print("Humidity: ");
              Serial.print(h);
              Serial.print(" %\t Temperature: ");
              Serial.print(t);
              Serial.print(" *C ");
              Serial.print(" \t Heat index: ");
              Serial.print(hic);
              Serial.print(" *C ");
            }
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature and humidity
            client.println("<!DOCTYPE HTML><html>");
            client.println("<head>");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">");
            client.println("<style>");
            client.println("html {");
            client.println("font-family: Arial;");
            client.println("display: inline-block;");
            client.println("margin: 0px auto;");
            client.println("text-align: center;");
            client.println("}");
            client.println("h2 { font-size: 3.0rem; }");
            client.println("p { font-size: 3.0rem; }");
            client.println(".units { font-size: 1.2rem; }");
            client.println(".dht-labels{");
            client.println("font-size: 1.5rem;");
            client.println("vertical-align:middle;");
            client.println("padding-bottom: 15px;");
            client.println("}");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h2>Ambient Device Test Server</h2>");
            client.println("<p>");
            client.println("<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;\"></i> ");
            client.println("<span class=\"dht-labels\">Temperature</span> ");
            client.println("<span id=\"temperature\">");
            client.println(celsiusTemp);
            client.println("</span>");
            client.println("<sup class=\"units\">&deg;C</sup>");
            client.println("</p>");
            client.println("<p>");
            client.println("<i class=\"fas fa-tint\" style=\"color:#00add6;\"></i> ");
            client.println("<span class=\"dht-labels\">Humidity</span>");
            client.println("<span id=\"humidity\">");
            client.println(humidityTemp);
            client.println("</span>");
            client.println("<sup class=\"units\">%</sup>");
            client.println("</p>");
            client.println("</body>");
            client.println("</html>");
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(dht.getMinimumSamplingPeriod());
    client.stop();
    Serial.println("Client disconnected.");
  }
}
