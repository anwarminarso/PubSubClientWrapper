# Arduino Client for MQTT (based on PubSubClient)
This library provides a client for doing simple publish/subscribe messaging with a server that supports MQTT.


## Overview
This library is for Arduino for easy use of MQTT


## Example Code

#### Publishing MQTT Topic every 10 seconds
```cpp
wrapper.setPublisher("/MyTopic", 10000, [&] {
  String message = "";
  DynamicJsonDocument doc(512);
  
  float temp = random(2500.0f, 3500.0f) / 100.0f;
  float hum = random(7500.0f, 9000.0f) / 100.0f;

  // create JSON document
  doc["temperature"] = temp;
  doc["humidity"] = hum;

  // Serialize to JSON
  // Excample Output messge => { "temperature": 25,"humidity":75 }
  serializeJson(doc, message);
  return message;
});
```


#### Simple Subscription
```cpp
wrapper.setSubscription("/MyTopic", [&](const char* message) {
  Serial.print("Message Received : ");
  Serial.print(message);
  Serial.println();
});

wrapper.setSubscription("/MyComplexTopic", [&](const char* message) {
  Serial.print("Message Received : ");
  Serial.print(message);
  Serial.println();

  //If our message is json format
  //for example mqtt message => { "temp": 28.3, "hum": 78.4 }
  DynamicJsonDocument doc(512);
  deserializeJson(doc, message); //deserialize JSON message to DynamicJsonDocument
  
  float temperature = doc["temp"];
  float humidity = doc["hum"];
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(", Humidity: ");
  Serial.println(humidity);
});

```

## Related Link
- [PubSubClient](https://github.com/knolleary/pubsubclient "PubSubClient")
- [ArduinoJSON](https://github.com/bblanchon/ArduinoJson "ArduinoJSON")

## License
[MIT License](https://github.com/anwarminarso/PubSubClientWrapper/blob/main/LICENSE.txt "MIT License")
