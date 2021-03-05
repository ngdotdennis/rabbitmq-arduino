# rabbitmq-arduino
Arduino sending sensor data over RabbitMQ's MQTT plugin.

The sketch uses a DHT11 humidity and temperature sensor and the onboard LED of the ESP32 board.
Furthermore I use a potentiometer(PIN 34) to pump up the amount of messages sent in order to analyze my server's performance.
My server runs on an Ubuntu VM with 4GB RAM, of which 40% is reserved and used by RabbitMQ (default threshold). 
