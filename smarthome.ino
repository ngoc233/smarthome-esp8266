/*==================================*\
|        Client nạp vào wemos        |
\*==================================*/
#include <SocketIOClient.h>
#include "DHT.h"

DHT dht;
// giá trị điện cho cảm biến chuyển động;
int valuePIR = 0;
// chân digital nhận giá trị của cảm biến chuyển động
int dPIR = 15;

//giá trị đèn 
int light1 = 13;//D7
int light2 = 12;//D6
//giá trị quạt
int fan1   = 4; //  D2
int fan2   = 0; // D3

//khai báo nhận điện thế trả về của cảm biến độ ẩm
int outpPutLand = 2; //D4

const char* ssid = "TP-LINK_D0859C";
const char* password = "65535219";
int i = 0;
int timeEmit = 0;
// Server Ip
String host = "192.168.0.102";
// Server port
int port = 8000;

// Khởi tạo socket
SocketIOClient socket;

// Kết nối wifi
void setupNetwork() {
    WiFi.begin(ssid, password);
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
    if (i == 21) {
        while (1) delay(500);
    }
    
    // Hàm này là hàm in log ra serial
    Serial.println("Wifi connected!");
}

// Thay đổi trạng thái đèn theo dữ liệu nhận được
void changeLedState(String data) {
   if (data == "[\"light\",\"on1\"]") {
      digitalWrite(light1,HIGH);
      socket.emit("onLight",String(1));
   }
   else if(data == "[\"light\",\"off1\"]"){
      digitalWrite(light1,LOW);
      socket.emit("offLight",String(1));
   }
   else if(data == "[\"light\",\"on2\"]"){
      digitalWrite(light2,HIGH);
      socket.emit("onLight",String(2));
   }
   else if(data == "[\"light\",\"off2\"]"){
      digitalWrite(light2,LOW);
      socket.emit("offLight",String(2));
   }
}
// change fan
void changeFan(String data)
{
  
  if (data == "[\"fan\",\"on1\"]") {
      digitalWrite(fan1,HIGH);
      socket.emit("onFan",String(1));
  }
  else if(data == "[\"fan\",\"off1\"]"){
      digitalWrite(fan1,LOW);
      socket.emit("offFan",String(1));
  }
  else if(data == "[\"fan\",\"on2\"]"){
      digitalWrite(fan2,HIGH);
      socket.emit("onFan",String(2));
  }
  else if(data == "[\"fan\",\"off2\"]"){
      digitalWrite(fan2,LOW);
      socket.emit("offFan",String(2));
  }
  
}

// auto
void startAuto(String data)
{
  i++;
}
void endAuto(String data)
{
  i=0;
  digitalWrite(fan1,LOW);
  digitalWrite(light2,LOW);
  digitalWrite(fan2,LOW);
  digitalWrite(outpPutLand,LOW);
}

void setup() {

   // nhận chân d8 vào điện của pir
   pinMode(dPIR,INPUT);

   //landHumidity
   pinMode(A0, OUTPUT);
   digitalWrite(A0,HIGH);
   
   //set up for dht11
    dht.setup(D1);
  
    // Cài đặt chân LED_BUILTIN là chân đầu ra tín hiệu
    pinMode(light1, OUTPUT);
    pinMode(light2, OUTPUT);

    // Cài đặt chân Fan là chân đầu ra tín hiệu
    pinMode(fan1, OUTPUT);
    pinMode(fan2, OUTPUT);
    // cài đặt cho output lan là đầu ra 
    pinMode(outpPutLand,OUTPUT);
    digitalWrite(outpPutLand,LOW);
    

    // Cài đặt giá trị mặc định là quạt tắt
    digitalWrite(fan1, LOW);
    digitalWrite(fan2, LOW);
    
    // Cài đặt giá trị mặc định là đèn tắt
    digitalWrite(light1, LOW);
    digitalWrite(light2, LOW);
    
    // Bắt đầu kết nối serial với tốc độ baud là 115200.
    // Khi bạn bật serial monitor lên để xem log thì phải set đúng tốc độ baud này.
    Serial.begin(115200);
    setupNetwork();
    
    // Lắng nghe sự kiện light thì sẽ thực hiện hàm changeLedState
    socket.on("light", changeLedState);
    // Lắng nghe sự kiện fan thì xẽ thực hiện hàm changeFan
    socket.on("fan",changeFan);
    // listen event auto
    socket.on("startAuto", startAuto);
    socket.on("endAuto", endAuto);
    
    // Kết nối đến server
    socket.connect(host, port);
    
}

void loop() {
     // Luôn luôn giữ kết nối với server.
    socket.monitor();
    //dht11
    delay(2000);/* Delay of amount equal to sampling period */
    float humidity = dht.getHumidity() - 3;/* Get humidity value */
    float temperature = dht.getTemperature();/* Get temperature value */
    Serial.print(dht.getStatusString());/* Print status of communication */
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.println(temperature, 1);

    // landHumidity
    int landHumidity = analogRead(A0);
    Serial.println(landHumidity);
    int percent = map(landHumidity, 0, 1023, 0, 100);
    Serial.println(percent);
    timeEmit+= 2;
     // send data for server
    if( timeEmit == 20)
    {
      socket.emit("temperature",String(temperature));
      socket.emit("homeHumidity",String(humidity));
      socket.emit("landHumidity",String(landHumidity));
      timeEmit = 0;
    }
    
    // has socket auto
    if(i>0)
    {
      // temperature
      if(temperature >= 20)
      {
        digitalWrite(fan1,HIGH);
        socket.emit("onFan",String(1));
        digitalWrite(fan2, HIGH);
        socket.emit("onFan",String(2));
      }
      else
      {
        digitalWrite(fan1,LOW);
        socket.emit("offFan",String(1));
        digitalWrite(fan2, LOW);
        socket.emit("offFan",String(2));
      }
      //landHumidity
      
      if(landHumidity <= 1000)
      {
        digitalWrite(outpPutLand,LOW);
        socket.emit("water","off");
      }
      else
      {
        digitalWrite(outpPutLand,HIGH);
        socket.emit("water","on");
      }

      // giá trị chuyển động`
      valuePIR = digitalRead(dPIR);
      if(valuePIR > 0){
        Serial.println("Cảnh báo có chộm !!");
        digitalWrite(light2,HIGH);
        socket.emit("onLight",String(2));
      }
      else
      {
        digitalWrite(light2,LOW);
        Serial.println("Trạng thái bình thường");
        socket.emit("offLight",String(2));
      }
      
    }
    
    Serial.println(valuePIR);
}
